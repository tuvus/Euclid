#pragma once
#include "ecs.h"
#include "game_object.h"

struct Tower_Data {
    Texture2D texture;
};

struct Tower_Component {
    static Component_Type component_type;
    int team;
    bool spawned;
    int reload;
    float range;
    Tower_Data* tower_data;
};

void Init_Tower(ECS* ecs, Entity entity, Tower_Data* tower_data, Vector2 pos, float range, int team,
                float scale, Color color);

void Tower_Update(ECS* ecs, Entity entity);

class Tower : public Game_Object {
  public:
    int team;
    bool spawned;
    int reload;
    float range;
    Tower_Data& tower_data;
    Entity_ID tmp_ecs_tower;
    ECS* ecs;

    Tower(ECS* ecs, Game_Manager& game_manager, Tower_Data& tower_data, Vector2 pos, float range,
          int team, float scale, Color color, Entity_ID tmp_ecs_tower);

    void Update() override;

    Object_UI* Create_UI_Object(Game_UI_Manager& game_ui_manager) override;
};