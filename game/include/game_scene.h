#pragma once

#include "card_game.h"
#include "game_manager.h"
#include "game_ui_manager.h"
#include "path.h"
#include "scene.h"
#include "ui/eui.h"

struct Card_Data;
class Game_Scene : public Scene, Network_Events_Receiver {
  private:
    Card_Game& card_game;
    std::unique_ptr<Game_Manager> game_manager;
    std::unique_ptr<Game_UI_Manager> game_ui_manager;
    EUI_Text* money_text;
    int time_until_income;
    Texture2D unit_texture;
    Texture2D tower_texture;
    Texture2D card_texture;
    vector<Card_Data*> card_datas;

  public:
    ECS* ecs;
    Path* f_path;
    Path* r_path;

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

    Path* Get_Team_Path(int team) const;

    static Color Get_Team_Color(int team);
};
