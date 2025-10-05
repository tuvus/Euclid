#pragma once
#include "ecs.h"
#include "game_object.h"
#include "path.h"

struct Unit_Data {
    Texture2D texture;
};

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

void Unit_Update(ECS* ecs, Entity_Array* entity_array, unsigned char* entity_data);

class Unit : public Game_Object {
  public:
    Path* path;
    int section;
    // Linear interpolation between the previous point and the current point
    // 0 means the start position and 1 means the end position of the segment on the path
    float lerp;
    float speed;
    int team;
    bool spawned;
    Unit_Data& unit_data;
    int tmp_ecs_unit;
    ECS* ecs;

    Unit(ECS* ecs, Game_Manager& game_manager, Unit_Data& unit_data, Path* path, float speed,
         float start_offset, int team, float scale, Color color, int tmp_ecs_unit);

    void Update() override;

    void Move(float dist);

    Object_UI* Create_UI_Object(Game_UI_Manager& game_ui_manager) override;
};