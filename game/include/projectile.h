#pragma once
#include "ecs.h"

struct Projectile_Component {
    static Component_Type component_type;
    int team;
    int damage;
    float speed;
    float range;
};

void Init_Projectile(ECS* ecs, Entity entity, Vector2 pos, float rot,
                     Projectile_Component projectile_component, Texture2D* texture, float scale,
                     Color color);

void Projectile_Update(ECS* ecs, Entity entity);

Entity_Type* Get_Projectile_Entity_Type();

Object_UI* Create_Projectile_UI(Entity entity, Game_UI_Manager& game_ui_manager);
