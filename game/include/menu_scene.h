#pragma once

#include "card_game.h"
#include "scene.h"

class Menu_Scene : public Scene {
  public:
    Menu_Scene(Card_Game& card_game) : Scene(card_game) {
        EUI_Box* root = new EUI_Box(true);
        root_elem = root;
        root->pos = {0, 0};
        root->size = {SCREEN_WIDTH, SCREEN_HEIGHT};
        root->vertical_alignment = Alignment::Center;
        root->horizontal_alignment = Alignment::Center;
        root->gap = 20;

        auto* title = new EUI_Text("Game Title!");
        title->font_size = 40;
        root->Add_Child(title);

        auto* join_button = new EUI_Button("Join Game", [&card_game] {
            card_game.Connect_To_Server();
            card_game.set_ui_screen(LOBBY);
            card_game.Get_Network()->Start_Network();
        });
        join_button->padding = {10, 20, 10, 20};
        root->Add_Child(join_button);

        auto* host_button = new EUI_Button("Host Game", [&card_game] {
            card_game.Start_Server();
            card_game.set_ui_screen(LOBBY);
            card_game.Get_Network()->Start_Network();
        });
        host_button->padding = {10, 20, 10, 20};
        root->Add_Child(host_button);
    }
    void Update_UI(chrono::milliseconds) override { root_elem->Render(); }
    void Update(chrono::milliseconds) override {}
};
