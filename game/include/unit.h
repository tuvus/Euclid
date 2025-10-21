#pragma once
#include "ecs.h"
#include "game_ui_manager.h"
#include "path.h"

class Unit_UI;
struct Unit_Component {
    static Component_Type component_type;
    Path* path;
    int section;
    // Linear interpolation between the previous point and the current point
    // 0 means the start position and 1 means the end position of the segment on the path
    float lerp;
    float speed;
    int team;
    bool spawned;
};

void Init_Unit(ECS* ecs, Entity entity, Path* path, float speed, float start_offset, int team,
               Texture2D texture, float scale, Color color);

void Move_Unit(ECS* ecs, Unit_Component* unit, Transform_Component* transform, Entity_ID entity_id,
               float dist_to_move);

void Unit_Update(ECS* ecs, Entity entity);

Entity_Type* Get_Unit_Entity_Type();

Object_UI* Create_Unit_UI(Entity entity, Game_UI_Manager& game_ui_manager);
