#pragma once

#include "card.h"
#include "card_game.h"
#include "game_manager.h"
#include "game_ui_manager.h"
#include "path.h"
#include "scene.h"
#include "tower.h"
#include "ui/eui.h"
#include "unit.h"

class Game_Scene : public Scene, Network_Events_Receiver {
  private:
    Card_Game& card_game;
    std::unique_ptr<Game_Manager> game_manager;
    std::unique_ptr<Game_UI_Manager> game_ui_manager;
    EUI_Text* money_text;
    int time_until_income;
    Tower_Data tower_data;
    Unit_Data unit_data;
    vector<Card_Data*> card_datas;

  public:
    Path* f_path;
    Path* r_path;
    Card* active_card;

    Game_Scene(Card_Game& card_game);

    ~Game_Scene() override;
    void Setup_Scene(vector<Player*> players, Player* local_player, long seed);

    void Update_UI(chrono::milliseconds) override;
    void Update(std::chrono::milliseconds) override;

    void On_Connected() override {}

    void On_Disconnected() override;

    void On_Server_Start() override {}

    void On_Server_Stop() override;

    void On_Client_Connected(Client_ID) override {}

    void On_Client_Disconnected(Client_ID) override {}

    void Activate_Card(Card* card);

    bool Can_Place_Tower(Vector2 pos, float min_dist) const;
};
