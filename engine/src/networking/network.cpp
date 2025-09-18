#include <cassert>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

#include "networking/network.h"
#include "networking/rpc_manager.h"

using namespace std;
const uint16 DEFAULT_SERVER_PORT = 27020;

Network::Network(bool server, std::function<void()> close_network_function)
    : close_network_function(close_network_function), server(server) {
    pthread_mutex_init(&newly_connected_client_mutex, 0);
    network_instance = this;
    state = Setting_Up;
    connection_events = make_unique<std::unordered_set<Network_Events_Receiver*>>();
    rpc_manager = make_unique<RPC_Manager>();
    rpc_step_heap = priority_queue<Rpc_Message*, std::vector<Rpc_Message*>, Rpc_Step_Comparator>();
}

Network::~Network() {
    state = Closed;
    if (server) {
        for (auto client_id : connected_clients) {
            connection_api->CloseConnection(client_id.first, 0, "Shutting down server", false);
        }
        for (const Network_Events_Receiver* receiver : *connection_events) {
            const_cast<Network_Events_Receiver*>(receiver)->On_Server_Stop();
        }
        connected_clients.clear();
        connection_api->CloseListenSocket(listen_socket);
        listen_socket = k_HSteamListenSocket_Invalid;
        connection_api->DestroyPollGroup(poll_group);
        poll_group = k_HSteamNetPollGroup_Invalid;
    }
    GameNetworkingSockets_Kill();
}

void Network::Start_Network() {
    SteamDatagramErrMsg err_msg;
    if (!GameNetworkingSockets_Init(nullptr, err_msg)) {
        std::cerr << "GameNetworkingSockets initialization failed " << err_msg << std::endl;
        exit(-1);
    }
    SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Debug,
                                                   Debug_Output);

    connection_api = SteamNetworkingSockets();
    SteamNetworkingIPAddr addr_server;
    addr_server.Clear();
    addr_server.SetIPv6LocalHost(DEFAULT_SERVER_PORT);
    SteamNetworkingConfigValue_t config_options;

    config_options.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                          (void*) On_Connect_Changed_Adapter);

    if (server) {
        remote_host_connection = k_HSteamNetConnection_Invalid;
        listen_socket = connection_api->CreateListenSocketIP(addr_server, 1, &config_options);
        if (listen_socket == k_HSteamListenSocket_Invalid)
            cerr << "Failed to setup socket listener on port " << addr_server.m_port << endl;
        poll_group = connection_api->CreatePollGroup();
        if (poll_group == k_HSteamNetPollGroup_Invalid)
            cerr << "Failed to setup poll group listener on port " << addr_server.m_port << endl;
        cout << "Starting server, listening on port: " << addr_server.m_port << endl;
        state = Server_Running;
        for (const Network_Events_Receiver* receiver : *connection_events) {
            const_cast<Network_Events_Receiver*>(receiver)->On_Server_Start();
        }
    } else {
        cout << "Starting client" << endl;
        remote_host_connection =
            connection_api->ConnectByIPAddress(addr_server, 1, &config_options);
        if (remote_host_connection == k_HSteamNetConnection_Invalid)
            cerr << "Failed to create connection to the host" << endl;
        state = Client_Connecting;
    }
}

void Network::Network_Update() {
    Poll_Incoming_Messages();
    connection_api->RunCallbacks();
    if (state == Closing) {
        close_network_function();
    }
}

/**
 * Handles receiving messages sent from the clients.
 */
void Network::Poll_Incoming_Messages() {
    if (state == Closing || state == Closed)
        return;

    pthread_mutex_lock(&newly_connected_client_mutex);
    for (Client_ID client_id : newly_connected_clients) {
        // Call all rpcs that we have previously received but coulden't process
        for (int i = 0; i < pre_connected_messages.size(); i++) {
            if (get<0>(pre_connected_messages[i]) != client_id)
                continue;
            Rpc_Message* d = get<1>(pre_connected_messages[i]);
            Receive_Message(client_id, d);
            pre_connected_messages.erase(pre_connected_messages.begin() + i);
            delete d;
            i--;
        }
    }
    newly_connected_clients.clear();
    pthread_mutex_unlock(&newly_connected_client_mutex);

    while (true) {
        ISteamNetworkingMessage* incoming_message = nullptr;
        int num_messages = -2;

        if (server)
            num_messages =
                connection_api->ReceiveMessagesOnPollGroup(poll_group, &incoming_message, 1);
        else
            num_messages = connection_api->ReceiveMessagesOnConnection(remote_host_connection,
                                                                       &incoming_message, 1);

        if (num_messages == 0)
            break;
        if (num_messages < 0)
            cerr << "Error checking messages on server: " << server
                 << ". Number of messages is: " << num_messages << endl;
        assert(num_messages == 1 && incoming_message);
        // // It's easier to parse the string as a c-string instead of a c++ string at first
        // std::string cstring_message;
        // cstring_message.assign((const char*)incoming_message->m_pData,
        // incoming_message->m_cbSize); const char* message = cstring_message.c_str();

        if (server) {
            auto client = connected_clients.find(incoming_message->m_conn);
            assert(client != connected_clients.end());
        } else {
            // cout << cstring_message << endl;
        }
        Receive_Message(incoming_message->GetConnection(),
                        static_cast<char*>(incoming_message->m_pData), incoming_message->m_cbSize);
        incoming_message->Release();
    }
}

/**
 * This static method is needed to convert On_Connection_Status_Changed to a
 * (void*) anonymous function instead of a method. We go around this by using a
 * static instance to point to the correct method that we wanted to call.
 */
void Network::On_Connect_Changed_Adapter(SteamNetConnectionStatusChangedCallback_t* new_status) {
    network_instance->On_Connection_Status_Changed(new_status);
}

void Network::On_Connection_Status_Changed(SteamNetConnectionStatusChangedCallback_t* new_status) {
    assert(new_status->m_hConn == remote_host_connection ||
           remote_host_connection == k_HSteamNetConnection_Invalid);
    auto client_id = new_status->m_hConn;
    switch (new_status->m_info.m_eState) {
        case k_ESteamNetworkingConnectionState_None:
            break;
        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
            if (server) {
                // Something happened to our client
                // Close the connection to them specifically
                if (new_status->m_eOldState != k_ESteamNetworkingConnectionState_Connected) {
                    assert(connected_clients.contains(client_id));
                    assert(new_status->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
                    connection_api->CloseConnection(new_status->m_hConn,
                                                    new_status->m_info.m_eState, nullptr, false);
                    for (const Network_Events_Receiver* receiver : *connection_events) {
                        const_cast<Network_Events_Receiver*>(receiver)->On_Client_Disconnected(
                            client_id);
                    }
                    connected_clients.erase(client_id);
                    break;
                }

                assert(connected_clients.contains(client_id));
                string debug_message;
                if (new_status->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer) {
                    debug_message =
                        "Connection closed by client request with id: " + to_string(client_id);
                } else {
                    debug_message =
                        "Internal problem on the server with id: " + to_string(client_id);
                }
                cout << debug_message << endl;
                connection_api->CloseConnection(new_status->m_hConn, new_status->m_info.m_eState,
                                                debug_message.c_str(), false);
                for (const Network_Events_Receiver* receiver : *connection_events) {
                    const_cast<Network_Events_Receiver*>(receiver)->On_Client_Disconnected(
                        client_id);
                }
                connected_clients.erase(client_id);
            } else {
                if (new_status->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer)
                    cout << "Leaving server due to server request" << endl;
                else
                    cout << "Leaving server" << endl;
                connection_api->CloseConnection(new_status->m_hConn, new_status->m_info.m_eState,
                                                nullptr, false);
                vector<Network_Events_Receiver*> receivers;
                for (const Network_Events_Receiver* receiver : *connection_events) {
                    receivers.emplace_back(const_cast<Network_Events_Receiver*>(receiver));
                }
                for (Network_Events_Receiver* receiver : receivers) {
                    receiver->On_Disconnected();
                }
                state = Closing;
            }
        }
        case k_ESteamNetworkingConnectionState_Connecting: {
            if (!server)
                break;
            if (connected_clients.contains(client_id)) {
                cerr << "Trying to connect a client that has already been connected!" << endl;
            }
            cout << "Connecting client " << new_status->m_info.m_szConnectionDescription << endl;
            if (connection_api->AcceptConnection(new_status->m_hConn) != k_EResultOK) {
                connection_api->CloseConnection(new_status->m_hConn,
                                                k_ESteamNetworkingConnectionState_ClosedByPeer,
                                                "Likely disconnected before connected", false);
                cout << "Couldn't connect client, they might have disconnected already" << endl;
                break;
            }
            if (!connection_api->SetConnectionPollGroup(new_status->m_hConn, poll_group)) {
                connection_api->CloseConnection(new_status->m_hConn,
                                                k_ESteamNetworkingConnectionState_ClosedByPeer,
                                                "Couldn't add client to a poll group", false);
            }
            connected_clients.emplace(client_id, new Connection_State());
            connection_api->SetConnectionName(new_status->m_hConn, to_string(client_id).c_str());
            for (const Network_Events_Receiver* receiver : *connection_events) {
                const_cast<Network_Events_Receiver*>(receiver)->On_Client_Connected(client_id);
            }
        }
        case k_ESteamNetworkingConnectionState_Connected: {
            if (server)
                break;
            cout << "Client connected" << endl;
            state = Client_Connected;
            for (const Network_Events_Receiver* receiver : *connection_events) {
                const_cast<Network_Events_Receiver*>(receiver)->On_Connected();
            }
            connected_clients.emplace(client_id, new Connection_State());
            pthread_mutex_lock(&newly_connected_client_mutex);
            newly_connected_clients.emplace_back(client_id);
            pthread_mutex_unlock(&newly_connected_client_mutex);
        }
        default:
            break;
    }
}

Network::Network_State Network::Get_Network_State() const {
    return state;
}

int Network::Get_Num_Connected_Clients() const {
    return connected_clients.size();
}

bool Network::Is_Server() const {
    return server;
}

std::string Network::Get_Network_State_Str() const {
    switch (state) {
        case Setting_Up:
            if (server)
                return "Starting Server";
            return "Starting Client";
        case Server_Running:
            return "Server Running";
        case Client_Connecting:
            return "Connecting to server";
        case Client_Connected:
            return "Connected to server";
        case Closing:
            if (server)
                return "Shutting down server";
            return "Closing client";
        case Closed:
            return "Closed";
    }
    return "Error";
}

void Network::Send_Message_To_Client(const Client_ID connection, const Rpc_Message& rpc_message) {
    clmdep_msgpack::sbuffer message_data;
    clmdep_msgpack::packer packer(message_data);

    // Only ordered rpc calls will have an ID
    if (rpc_message.order_sensitive)
        packer.pack(Rpc_Message(rpc_message, connected_clients[connection]->next_rpc_id++));
    else
        packer.pack(rpc_message);

    connection_api->SendMessageToConnection(connection, message_data.data(),
                                            static_cast<uint32>(message_data.size()),
                                            k_nSteamNetworkingSend_Reliable, nullptr);
}

void Network::Send_Message_To_Clients(const Rpc_Message& rpc_message) {
    assert(rpc_message.associated_step != -1);
    for (const auto& client_id : connected_clients) {
        Send_Message_To_Client(client_id.first, rpc_message);
    }
}

void Network::Send_Message_To_Server(const Rpc_Message& rpc_message) {
    // Psych! We can actually reuse Send_Message_To_Client but pass the server connection instead!
    Send_Message_To_Client(remote_host_connection, rpc_message);
}

void Network::Receive_Message(Client_ID from, char* data, size_t size) {
    clmdep_msgpack::object_handle result;
    clmdep_msgpack::unpack(result, data, size);
    auto tmp = result.get().as<Rpc_Message>();
    auto* rpc = new Rpc_Message(result.get().as<Rpc_Message>());
    if (!connected_clients.contains(from)) {
        pre_connected_messages.emplace_back(make_tuple(from, rpc));
        return;
    }
    Receive_Message(from, rpc);
}

void Network::Receive_Message(Client_ID from, Rpc_Message* rpc) {
    if (!rpc->order_sensitive) {
        invoke_rpc(rpc->order_sensitive, rpc->associated_step, rpc->rpc_call.data(),
                   rpc->rpc_call.size());
        return;
    }
    if (rpc->rpc_id > connected_clients[from]->next_rpc_id) {
        // This rpc has arrived earlier than a previous rpc and should be called later once the
        // previous ones arrive Make sure to copy the rpc to memory so that it isn't accidentally
        // deleted
        connected_clients[from]->rpc_order_heap.push(rpc);
        return;
    }

    connected_clients[from]->next_rpc_id++;
    rpc->associated_step == -2 ? invoke_rpc(rpc->order_sensitive, rpc->associated_step,
                                            rpc->rpc_call.data(), rpc->rpc_call.size())
                               : rpc_step_heap.emplace(rpc);

    // Check if we have any other rpcs stored to call
    while (!connected_clients[from]->rpc_order_heap.empty() &&
           connected_clients[from]->next_rpc_id ==
               connected_clients[from]->rpc_order_heap.top()->rpc_id) {
        connected_clients[from]->next_rpc_id++;
        Rpc_Message* next_rpc_message = connected_clients[from]->rpc_order_heap.top();
        next_rpc_message->associated_step == -2
            ? invoke_rpc(next_rpc_message->order_sensitive, rpc->associated_step,
                         next_rpc_message->rpc_call.data(), next_rpc_message->rpc_call.size())
            : rpc_step_heap.emplace(rpc);

        connected_clients[from]->rpc_order_heap.pop();
    }
}

void Network::invoke_rpc(bool order_sensitive, long associated_step, char* data, size_t size) {
    if (server) {
        auto result = rpc_manager->call_data_rpc(data, size);
        if (result == RPC_Manager::INVALID) {
            cerr << "Dropping invalid rpc call!" << endl;
            return;
        }
        if (result == RPC_Manager::VALID_CALL_ON_CLIENTS) {
            auto rpc_call_data = Rpc_Message(order_sensitive, associated_step, data, size);
            Send_Message_To_Clients(rpc_call_data);
        }
    } else {
        auto result = rpc_manager->call_data_rpc(data, size);
        // The rpc call must be valid by this point
        if (result != RPC_Manager::VALID && result != RPC_Manager::VALID_CALL_ON_CLIENTS) {
            string rpc_name = data + 4;
            cerr << "Client received an invalid rpc call " << rpc_name
                 << "! This is probably due to a desync!" << endl;
        }
    }
}

void Network::Process_Step_Rpcs(long step) {
    if (!rpc_step_heap.empty() && !server)
        assert(rpc_step_heap.top()->associated_step >= step);

    last_step = step;

    while (!rpc_step_heap.empty() && (rpc_step_heap.top()->associated_step == step ||
                                      rpc_step_heap.top()->associated_step == -1)) {
        Rpc_Message* rpc = rpc_step_heap.top();
        assert(rpc->associated_step == step || (server && rpc->associated_step == -1));
        invoke_rpc(rpc->order_sensitive, step, rpc->rpc_call.data(), rpc->rpc_call.size());

        rpc_step_heap.pop();
    }
}

void Debug_Output(ESteamNetworkingSocketsDebugOutputType error_type, const char* pszMsg) {
    if (error_type == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
        cerr << pszMsg << endl;
    } else {
        // cout << pszMsg << endl;
    }
}

Network* Network::network_instance = nullptr;
