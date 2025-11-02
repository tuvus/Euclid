#pragma once
#include "ecs.h"

struct Tower_Card_Component;
struct Tower_Component {
    static Component_Type component_type;
    int team;
    bool spawned;
    int reload_speed;
    int reload_time;
    float range;
    int damage;
    float projectile_speed;
    Texture2D* projectile_texture;
};

void Init_Tower(Entity entity, Vector2 pos, int team, Tower_Card_Component& tower_component,
                Texture2D* texture, float scale, Color color);

void Tower_Update(ECS* ecs, Entity entity);

Entity_Type* Get_Tower_Entity_Type();

Object_UI* Create_Tower_UI(Entity entity, Game_UI_Manager& game_ui_manager);
