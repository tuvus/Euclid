#pragma once
#include "game_object_ui.h"
#include "game_ui_manager.h"
#include "unit.h"

class Unit_UI : public Game_Object_UI<Unit> {
  public:
    Unit_UI(Unit* unit, Game_UI_Manager& game_ui_manager) : Game_Object_UI(unit, game_ui_manager) {}

    void Update_UI(EUI_Context* ctx) override {
        game_ui_manager.DrawImage(object->unit_data.texture, object->pos, object->rot,
                                  object->scale, object->color);
    }
};