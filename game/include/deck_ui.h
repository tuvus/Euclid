#pragma once
#include "card.h"
#include "card_ui.h"
#include "deck.h"
#include "game_object_ui.h"
#include "game_ui_manager.h"

class Deck_UI : public Game_Object_UI<Deck> {
  public:
    Deck_UI(Deck* deck, Game_UI_Manager& game_ui_manager) : Game_Object_UI(deck, game_ui_manager) {}

    void Update_UI(EUI_Context* ctx) override {
        if (static_cast<Card_Player*>(game_ui_manager.game_manager.local_player)->deck != object)
            return;

        float x_pos = 10;

        for (auto card : object->hand) {
            Card_UI* card_ui = static_cast<Card_UI*>(game_ui_manager.active_ui_objects[card]);
            float selected_offset =
                card_ui->object->game_scene.active_card == card_ui->object ? -40 : 0;
            card_ui->Update_UI(ctx,
                               Vector2(x_pos, game_object->game_manager.application.screen_height -
                                                  10 + selected_offset),
                               card->scale / 4);
            x_pos += card->card_data.texture.width * card->scale / 4 + 10;
        }
    }
};