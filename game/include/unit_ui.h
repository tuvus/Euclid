#pragma once
#include "game_object_ui.h"
#include "game_ui_manager.h"

class Unit_UI : public Object_UI {
  public:
    Unit_UI(Entity entity, Game_UI_Manager& game_ui_manager) : Object_UI(entity, game_ui_manager) {}

    void Update_UI(EUI_Context* ctx) override {
        Entity entity = std::get<0>(ecs.entities_by_id[entity_id]);
        auto* transform = std::get<1>(entity)->Get_Component<Transform_Component>(
            entity, &Transform_Component::component_type);
        auto* ui =
            std::get<1>(entity)->Get_Component<UI_Component>(entity, &UI_Component::component_type);
        game_ui_manager.DrawImage(*ui->texture, transform->pos, transform->rot, transform->scale,
                                  ui->color);
    }
};