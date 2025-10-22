#pragma once
#include "ecs.h"
#include "game_ui_manager.h"

struct Tower_Component {
    static Component_Type component_type;
    int team;
    bool spawned;
    int reload;
    float range;
};

void Init_Tower(Entity entity, Vector2 pos, float range, int team, Texture2D* texture, float scale,
                Color color);

void Tower_Update(ECS* ecs, Entity entity);

Entity_Type* Get_Tower_Entity_Type();

Object_UI* Create_Tower_UI(Entity entity, Game_UI_Manager& game_ui_manager);
