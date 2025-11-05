#include "lobby_scene.h"
#include "card_player.h"
#include "game_scene.h"

Lobby_Scene::Lobby_Scene(Card_Game& card_game)
    : Scene(card_game), card_game(card_game), player_id_count(0), player_count(0) {
    auto* root = new EUI_Box(true);
    root_elem = root;
    root->pos = {0, 0};
    root->size = {static_cast<float>(card_game.screen_width),
                  static_cast<float>(card_game.screen_height)};
    root->main_axis_alignment = Alignment::Center;
    root->cross_axis_alignment = Alignment::Center;
    root->gap = 20;

    status_text = new EUI_Text("Connecting...");
    if (card_game.Get_Network()->Is_Server())
        status_text->Set_Text("Setting up server...");
    root->Add_Child(status_text);
    start_button = new EUI_Button("Start Game", [this, &card_game]() { Server_Start_Game(); });
    start_button->padding = {10, 20, 10, 20};
    root->Add_Child(start_button);
    start_button->is_visible = card_game.Get_Network()->Is_Server();
    ai_only_button =
        new EUI_Button("Start AI Only Game", [this, &card_game]() { Server_Start_AI_Only(); });
    ai_only_button->padding = {10, 20, 10, 20};
    root->Add_Child(ai_only_button);
    ai_only_button->is_visible = card_game.Get_Network()->Is_Server();
    auto* leave_button = new EUI_Button("Leave Room", [this, &card_game]() {
        card_game.Close_Network();
        card_game.set_ui_screen(MENU);
    });
    leave_button->padding = {10, 20, 10, 20};
    root->Add_Child(leave_button);
    card_game.Get_Network()->bind_rpc("setplayercount", [this](int new_player_count) {
        player_count = new_player_count;
        status_text->Set_Text("Players: " + to_string(player_count));
        return RPC_Manager::Rpc_Validator_Result::VALID_CALL_ON_CLIENTS;
    });
    card_game.Get_Network()->bind_rpc(
        "addplayer", [this](Client_ID client_id, Player_ID player_id) {
            players.emplace_back(new Card_Player(client_id, player_id, 0));
            return RPC_Manager::Rpc_Validator_Result::VALID_CALL_ON_CLIENTS;
        });
    card_game.Get_Network()->bind_rpc("addaiplayer", [this](Player_ID player_id) {
        players.emplace_back(new Card_Player(player_id, 0));
        return RPC_Manager::Rpc_Validator_Result::VALID_CALL_ON_CLIENTS;
    });
    card_game.Get_Network()->bind_rpc("setplayerid", [this](Player_ID player_id) {
        for (auto& player : players) {
            if (player->player_id != player_id)
                continue;
            player->local_player = true;
            local_player = player;
        }
        return RPC_Manager::Rpc_Validator_Result::VALID;
    });
    card_game.Get_Network()->bind_rpc("removeplayer", [this](Client_ID client_id) {
        if (!any_of(players.begin(), players.end(),
                    [client_id](Player* player) { return player->client_id == client_id; }))
            return RPC_Manager::INVALID;
        erase_if(players, [client_id](Player* p) { return p->client_id == client_id; });
        return RPC_Manager::Rpc_Validator_Result::VALID_CALL_ON_CLIENTS;
    });
    card_game.Get_Network()->bind_rpc("setplayerteam", [this](Player_ID player_id, int team) {
        static_cast<Card_Player*>(*std::ranges::find_if(players, [player_id](Player* player) {
            return player->player_id == player_id;
        }))->team = team;
        return RPC_Manager::VALID_CALL_ON_CLIENTS;
    });
    card_game.Get_Network()->bind_rpc("startgame", [this, &card_game](long seed) {
        if (card_game.Get_Network()->Get_Network_State() != Network::Server_Running &&
            card_game.Get_Network()->Get_Network_State() != Network::Client_Connected)
            return RPC_Manager::Rpc_Validator_Result::INVALID;
        Start_Game(seed);
        return RPC_Manager::VALID_CALL_ON_CLIENTS;
    });
    card_game.Get_Network()->connection_events->emplace(
        static_cast<Network_Events_Receiver*>(this));
    players = vector<Player*>();
}

Lobby_Scene::~Lobby_Scene() {
    if (card_game.Get_Network() != nullptr)
        card_game.Get_Network()->connection_events->erase(
            static_cast<Network_Events_Receiver*>(this));
}

void Lobby_Scene::Server_Start_Game() {
    int team = 0;
    for (auto* player : players) {
        card_game.Get_Network()->call_rpc(true, "setplayerteam", player->player_id, team);
        team = (team + 1) % 2;
    }
    // If the teams are unbalanced add an AI
    if (team == 1) {
        Player_ID ai_id = player_id_count++;
        card_game.Get_Network()->call_rpc(true, "addaiplayer", ai_id);
        card_game.Get_Network()->call_rpc(true, "setplayerteam", ai_id, team);
    }
    card_game.Get_Network()->call_rpc(
        true, "startgame",
        chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch())
            .count());
}

void Lobby_Scene::Server_Start_AI_Only() {
    for (auto* player : players) {
        card_game.Get_Network()->call_rpc(true, "setplayerteam", player->player_id, -1);
    }
    Player_ID ai_id = player_id_count++;
    card_game.Get_Network()->call_rpc(true, "addaiplayer", ai_id);
    card_game.Get_Network()->call_rpc(true, "setplayerteam", ai_id, 0);
    ai_id = player_id_count++;
    card_game.Get_Network()->call_rpc(true, "addaiplayer", ai_id);
    card_game.Get_Network()->call_rpc(true, "setplayerteam", ai_id, 1);

    card_game.Get_Network()->call_rpc(
        true, "startgame",
        chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch())
            .count());
}

void Lobby_Scene::Start_Game(long seed) {
    card_game.set_ui_screen(GAME);
    Game_Scene* game_scene = static_cast<Game_Scene*>(card_game.scene);
    game_scene->Setup_Scene(players, local_player, seed);
}

void Lobby_Scene::Update_UI(std::chrono::milliseconds) {
    root_elem->Render();
}

void Lobby_Scene::Update(std::chrono::milliseconds) {
}

void Lobby_Scene::On_Connected() {
    if (player_count == 0)
        status_text->Set_Text("Connected");
}

void Lobby_Scene::On_Disconnected() {
    card_game.set_ui_screen(MENU);
}

void Lobby_Scene::On_Server_Start() {
    player_count = 1;
    card_game.Get_Network()->call_rpc(true, "setplayercount", player_count);
    card_game.Get_Network()->call_rpc(true, "addplayer", 0, player_id_count++);
    local_player = players[0];
}

void Lobby_Scene::On_Server_Stop() {
    card_game.set_ui_screen(MENU);
}

void Lobby_Scene::On_Client_Connected(Client_ID client_id) {
    player_count++;
    Player_ID new_player_id = player_id_count++;
    card_game.Get_Network()->call_rpc(true, "setplayercount", player_count);
    card_game.Get_Network()->call_rpc(true, "addplayer", client_id, new_player_id);
    card_game.Get_Network()->call_rpc_on_client(client_id, true, "setplayerid", new_player_id);
    for (const auto& player : players) {
        if (player->client_id == client_id)
            continue;
        card_game.Get_Network()->call_rpc_on_client(client_id, true, "addplayer", player->client_id,
                                                    player->player_id);
    }
}

void Lobby_Scene::On_Client_Disconnected(Client_ID client_id) {
    player_count--;
    card_game.Get_Network()->call_rpc(true, "setplayercount", player_count);
    card_game.Get_Network()->call_rpc(true, "removeplayer", client_id);
}
