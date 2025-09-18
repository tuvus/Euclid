#pragma once

#include "card_game.h"
#include "card_player.h"
#include "game_manager.h"
#include "networking/network.h"
#include "scene.h"

using namespace std;

class Lobby_Scene : public Scene, Network_Events_Receiver {
  private:
    EUI_Text* status_text;
    EUI_Button* start_button;
    EUI_Button* ai_only_button;
    Card_Game& card_game;
    int player_count;
    // Used on the server to generate player ids
    int player_id_count;
    vector<Player*> players;
    Player* local_player;
    void Server_Start_Game();
    void Server_Start_AI_Only();
    void Start_Game(long seed);

  public:
    Lobby_Scene(Card_Game& card_game);
    ~Lobby_Scene() override;
    void Update_UI(std::chrono::milliseconds) override;
    void Update(std::chrono::milliseconds) override;
    void On_Connected() override;
    void On_Disconnected() override;
    void On_Server_Start() override;
    void On_Server_Stop() override;
    void On_Client_Connected(Client_ID) override;
    void On_Client_Disconnected(Client_ID) override;
};
