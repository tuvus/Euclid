#pragma once
#include "card.h"
#include "game_object_ui.h"
#include "game_scene.h"
#include "game_ui_manager.h"

#include <raymath.h>

class Card_UI : public Object_UI {
  public:
    bool is_hovered = false;
    Card_UI(Entity entity, Game_UI_Manager& game_ui_manager) : Object_UI(entity, game_ui_manager) {}

    void Update_UI(EUI_Context*) override {}

    void Update_UI(EUI_Context* ctx, Vector2 pos, float scale) {
        Entity entity = std::get<0>(ecs.entities_by_id[entity_id]);
        auto* ui =
            std::get<1>(entity)->Get_Component<UI_Component>(entity, &UI_Component::component_type);
        auto* card = std::get<1>(entity)->Get_Component<Card_Component>(
            entity, &Card_Component::component_type);
        const float width = ui->texture->width * scale;
        const float height = ui->texture->height * scale;

        Card_Player* local_player =
            static_cast<Card_Player*>(game_ui_manager.game_manager.local_player);
        // Check for pointer events
        is_hovered = CheckCollisionPointRec(ctx->input.mouse_position,
                                            {pos.x, pos.y - height, width, height}) &&
                     Can_Play_Card(local_player, entity, Vector2Zero());
        if (is_hovered && ctx->input.left_mouse_pressed) {
            local_player->active_card = entity;
        }

        const Color color = (is_hovered && get<0>(local_player->active_card) == nullptr) ||
                                    (get<0>(local_player->active_card) != nullptr &&
                                     Entity_Array::Get_Entity_ID(local_player->active_card) ==
                                         Entity_Array::Get_Entity_ID(entity))
                                ? GRAY
                                : BLACK;

        // Draw the card
        game_ui_manager.DrawScreenImage(
            *ui->texture, Vector2(pos.x + width / 2, pos.y - height / 2), 0, scale, ui->color);
        DrawText(card->card_data->name.c_str(), pos.x + 5, pos.y - height + 10, 20, color);
        DrawText(to_string(card->card_data->cost).c_str(), pos.x + width - 25, pos.y - height + 45,
                 20, color);
        DrawText(card->card_data->desc.c_str(), pos.x + 5, pos.y - height / 2, 10, color);
    }
};