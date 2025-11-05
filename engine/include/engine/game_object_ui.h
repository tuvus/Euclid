#pragma once


#include "ui/eui.h"
#include "ecs.h"

class Object_UI {
  public:
    Entity_ID entity_id;
    Game_UI_Manager& game_ui_manager;
    ECS& ecs;

    Object_UI(Entity entity, Game_UI_Manager& game_ui_manager)
        : entity_id(Entity_Array::Get_Entity_Data(entity).id), game_ui_manager(game_ui_manager),
          ecs(std::get<1>(entity)->ecs) {}
    virtual ~Object_UI() = default;

    virtual void Update_UI(EUI_Context* eui_ctx) = 0;
};
