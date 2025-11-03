#pragma once

#include "card_game.h"
#include "scene.h"

/// Scene for testing layout algorithms. Contains elements and nested layout with differenet
/// alignments.

class Test_Scene : public Scene {
  public:
    Test_Scene(Card_Game& card_game) : Scene(card_game) {
        // root is the main (horizontal) box
        EUI_Box* root = new EUI_Box();
        root_elem = root;
        root->vertical_alignment = Alignment::Center;
        root->horizontal_alignment = Alignment::Center;

        // box1 is left hbox
        EUI_Box* box1 = new EUI_Box();
        box1->vertical_alignment = Alignment::Center;
        box1->horizontal_alignment = Alignment::Center;
        box1->background_color = BLUE;
        box1->size = {600, 300};

        // box2 is right vbox
        EUI_Box* box2 = new EUI_Box(true);
        box2->vertical_alignment = Alignment::Center;
        box2->horizontal_alignment = Alignment::Center;
        box2->background_color = RED;
        box2->size = {500, 500};
        box2->padding = {10, 10, 10, 10};
        EUI_Box* box2_1 = new EUI_Box();
        box2_1->vertical_alignment = Alignment::Center;
        box2_1->horizontal_alignment = Alignment::Center;
        box2_1->gap = 20;
        box2_1->background_color = GREEN;
        box2_1->size = {Size::Grow(), Size::Grow()};
        box2->Add_Child(box2_1);

        root->Add_Child(box1);
        root->Add_Child(box2);
    }

    void Update_UI(chrono::milliseconds) override { root_elem->Render(); }
    void Update(chrono::milliseconds) override {}
};
