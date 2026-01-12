#include "projectile.h"

#include "projectile_ui.h"
#include "unit.h"

#include <raymath.h>

void Init_Projectile(ECS* ecs, Entity entity, Vector2 pos, float rot,
                     Projectile_Component projectile_component, Texture2D* texture, float scale,
                     Color color) {
    auto* transform = get<1>(entity)->Get_Component<Transform_Component>(entity);
    auto* projectile = get<1>(entity)->Get_Component<Projectile_Component>(entity);
    auto* ui = get<1>(entity)->Get_Component<UI_Component>(entity);
    transform->pos = pos;
    transform->rot = rot;
    transform->scale = scale;
    projectile->team = projectile_component.team;
    projectile->damage = projectile_component.damage;
    projectile->speed = projectile_component.speed;
    projectile->range = projectile_component.range;
    ui->scale = scale;
    ui->color = color;
    ui->texture = texture;
}

void Projectile_Update(ECS* ecs, Entity entity) {
    Entity_ID entity_id = Entity_Array::Get_Entity_ID(entity);
    auto* transform = get<1>(entity)->Get_Component<Transform_Component>(entity);
    auto* projectile = get<1>(entity)->Get_Component<Projectile_Component>(entity);

    for (auto other_entity : ecs->Get_Entities_Of_Type(Get_Unit_Entity_Type())) {
        Entity_ID other_id = Entity_Array::Get_Entity_ID(other_entity);
        if (other_id == entity_id)
            continue;
        Unit_Component* other = get<1>(other_entity)->Get_Component<Unit_Component>(other_entity);

        if (other->team == projectile->team || !other->spawned)
            continue;

        Transform_Component* other_transform =
            get<1>(other_entity)->Get_Component<Transform_Component>(other_entity);
        if (Vector2Distance(transform->pos, other_transform->pos) > 30)
            continue;

        // Collide
        other->health -= projectile->damage;
        if (other->health <= 0) {
            other->spawned = false;
            ecs->Delete_Entity(other_entity);
        } else {
            other->bump_back = projectile->damage * 5;
        }
        ecs->Delete_Entity(entity);
        return;
    }

    auto mov = Vector2(sin(transform->rot * DEG2RAD) * projectile->speed,
                       -cos(transform->rot * DEG2RAD) * projectile->speed);
    transform->pos += mov;
    projectile->range -= projectile->speed;
    if (projectile->range <= 0)
        ecs->Delete_Entity(entity);
}

Entity_Type* Get_Projectile_Entity_Type() {
    return new Entity_Type(vector{&UI_Component::component_type,
                                  &Transform_Component::component_type,
                                  &Projectile_Component::component_type});
}

Object_UI* Create_Projectile_UI(Entity entity, Game_UI_Manager& game_ui_manager) {
    return new Projectile_UI(entity, game_ui_manager);
}

Component_Type Projectile_Component::component_type =
    Component_Type{"Projectile", sizeof(Projectile_Component)};
