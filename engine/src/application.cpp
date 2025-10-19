#include <iostream>
#include <raylib.h>
#include <thread>

#include "application.h"
#include "application_factory.h"

void Create_Application(unique_ptr<ApplicationFactory> application_factory, bool client) {
    const auto application = application_factory->Create_Application(client);
    cout << "Creating application: " << application->Get_Name() << endl << endl;
    application->Start_Application();
}

Application::Application(std::string name, bool client, uint16_t screen_width,
                         uint16_t screen_height)
    : application_name(name), client(client), screen_width(screen_width),
      screen_height(screen_height), application_state(ApplicationState::SettingUp),
      network(nullptr) {
}

void Application::Start_Application() {
    cout << "Starting application: " + Get_Name() << endl;
    client ? Start_Client() : Start_Headless();
    Application_Loop();
}

void Application::Start_Headless() {
    // Headless will always be servers
    Start_Server();
}

void Application::Start_Client() {
    // Clients can be both host and player or just a player
    InitWindow(screen_width, screen_height, application_name.c_str());
    SetTargetFPS(60);
    SetWindowFocused();
}

void Application::Start_Server() {
    network = make_shared<Network>(true, [this] { this->Close_Network(); });
}

void Application::Connect_To_Server() {
    network = make_shared<Network>(false, [this] { this->Close_Network(); });
}

void Application::Close_Network() {
    network.reset();
}

string Application::Get_Name() {
    return application_name;
}

shared_ptr<Network> Application::Get_Network() {
    return network;
}

void Application::Application_Loop() {
    application_state = ApplicationState::Running;
    chrono::time_point<chrono::system_clock> frame_end_time = chrono::system_clock::now();
    while (application_state == ApplicationState::Running) {
        chrono::time_point<chrono::system_clock> frame_start_time = chrono::system_clock::now();
        auto delta_time =
            /*chrono::duration_cast<std::chrono::milliseconds>(frame_start_time - frame_end_time);*/
            16ms; // TODO: ???

        if (network)
            network->Network_Update();

        for (int i = 0; i < 1; i++) {
            Update(delta_time);
        }

        if (client) {
            Update_UI(delta_time);
        }

        this_thread::sleep_until(frame_start_time + 16ms);
        frame_end_time = chrono::system_clock::now();
    }
    Close_Application();
}

void Application::Close_Application() {
    application_state = ApplicationState::Closing;
}
