#pragma once

#include <chrono>

#include "networking/network.h"
#include "ui/eui.h"

using namespace std;

class ApplicationWindow;
class ApplicationFactory;

enum class ApplicationState {
    SettingUp,
    Running,
    Closing,
};

class Application {
  private:
    bool client;
    string application_name;
    ApplicationState application_state;
    shared_ptr<Network> network;

    void Application_Loop();

  protected:
    void Start_Headless();
    virtual void Start_Client();

  public:
    Application(std::string name, bool client, float screen_width, float screen_height);
    virtual ~Application() = default;

    float screen_width;
    float screen_height;

    EUI_Context* eui_ctx = new EUI_Context();

    /* Game update behavior like physics a game logic. Does not include any UI */
    virtual void Update(chrono::milliseconds) = 0;
    /* UI update where the buttons, text and game objects are rendered. */
    virtual void Update_UI(chrono::milliseconds) = 0;

    void Start_Application();
    void Start_Server();
    void Connect_To_Server();

    string Get_Name();
    shared_ptr<Network> Get_Network();

    virtual void Close_Application();
    void Close_Network();
};

void Create_Application(unique_ptr<ApplicationFactory> application_factory, bool client);
