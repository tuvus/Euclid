#pragma once
#include "card.h"
#include "game_object_ui.h"
#include "game_scene.h"
#include "game_ui_manager.h"

#include <raymath.h>

class Card_UI : public Game_Object_UI<Card> {
  public:
    bool is_hovered = false;
    Card_UI(Card* card, Game_UI_Manager& game_ui_manager) : Game_Object_UI(card, game_ui_manager) {}

    void Update_UI(EUI_Context*) override {}

    void Update_UI(EUI_Context* ctx, Vector2 pos, float scale) {
        const float width = object->card_data.texture.width * scale;
        const float height = object->card_data.texture.height * scale;

        // Check for pointer events
        is_hovered = CheckCollisionPointRec(ctx->input.mouse_position,
                                            {pos.x, pos.y - height, width, height}) &&
                     object->Can_Play_Card(
                         static_cast<Card_Player*>(game_ui_manager.game_manager.local_player),
                         Vector2Zero());
        if (is_hovered && ctx->input.left_mouse_pressed) {
            object->game_scene.Activate_Card(object);
        }

        const Color color = (is_hovered && object->game_scene.active_card == nullptr) ||
                                    object->game_scene.active_card == object
                                ? GRAY
                                : BLACK;

        // Draw the card
        game_ui_manager.DrawScreenImage(object->card_data.texture,
                                        Vector2(pos.x + width / 2, pos.y - height / 2), object->rot,
                                        scale, object->color);
        DrawText(object->card_data.name.c_str(), pos.x + 5, pos.y - height + 10, 20, color);
        DrawText(to_string(object->card_data.cost).c_str(), pos.x + width - 25, pos.y - height + 45,
                 20, color);
        DrawText(object->card_data.desc.c_str(), pos.x + 5, pos.y - height / 2, 10, color);
    }
};