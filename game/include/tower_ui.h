#pragma once
#include "game_object_ui.h"
#include "game_ui_manager.h"
#include "tower.h"

class Tower_UI : public Game_Object_UI<Tower> {
  public:
    Tower_UI(Tower* tower, Game_UI_Manager& game_ui_manager)
        : Game_Object_UI(tower, game_ui_manager) {}

    void Update_UI(EUI_Context* ctx) override {
        game_ui_manager.DrawImage(object->tower_data.texture, object->pos, object->rot,
                                  object->scale, object->color);
    }
};