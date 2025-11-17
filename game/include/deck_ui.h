#pragma once
#include "card.h"
#include "card_ui.h"
#include "deck.h"
#include "game_object_ui.h"
#include "game_ui_manager.h"

class Deck_UI : public Object_UI {
    Card_Player* card_player;

  public:
    Deck_UI(Entity entity, Game_UI_Manager& game_ui_manager, Card_Player* card_player)
        : Object_UI(entity, game_ui_manager), card_player(card_player) {}

    void Update_UI(EUI_Context* ctx) override {
        if (static_cast<Card_Player*>(game_ui_manager.game_manager.local_player)->deck !=
            card_player->deck)
            return;

        float x_pos = 10;

        for (auto entity_id : card_player->Get_Deck()->hand) {
            Entity entity = get<0>(ecs.entities_by_id[entity_id]);
            auto* ui_component = get<1>(entity)->Get_Component<UI_Component>(entity);
            Card_UI* card_ui = static_cast<Card_UI*>(game_ui_manager.active_ui_objects[entity_id]);
            float selected_offset = card_player->active_card == entity ? -40 : 0;
            card_ui->Update_UI(
                ctx,
                Vector2(x_pos, game_ui_manager.game_manager.application.screen_height - 10 +
                                   selected_offset),
                ui_component->scale / 4);
            x_pos += ui_component->texture->width * ui_component->scale / 4 + 10;
        }
    }
};