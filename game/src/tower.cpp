#include "tower.h"

#include "game_manager.h"
#include "projectile.h"
#include "tower_card.h"
#include "tower_ui.h"
#include "unit.h"

#include <climits>
#include <raymath.h>

void Init_Tower(Entity entity, Vector2 pos, int team, Tower_Card_Component& tower_component,
                Texture2D* texture, float scale, Color color) {
    auto* tower = get<1>(entity)->Get_Component<Tower_Component>(entity);
    auto* transform = get<1>(entity)->Get_Component<Transform_Component>(entity);
    auto* ui = get<1>(entity)->Get_Component<UI_Component>(entity);
    transform->pos = pos;
    transform->rot = 0;
    transform->scale = .4;
    tower->reload_speed = tower_component.reload;
    tower->range = tower_component.range;
    tower->team = team;
    tower->damage = tower_component.damage;
    tower->projectile_speed = tower_component.projectile_speed;
    tower->projectile_texture = tower_component.projectile_texture;
    ui->texture = texture;
    ui->scale = scale;
    ui->color = color;
}

void Tower_Update(ECS* ecs, Entity entity) {
    auto* tower = get<1>(entity)->Get_Component<Tower_Component>(entity);
    if (tower->reload_time > 0)
        tower->reload_time--;
    if (tower->reload_time > 0)
        return;

    Entity_ID entity_id = Entity_Array::Get_Entity_Data(entity).id;
    auto* transform = get<1>(entity)->Get_Component<Transform_Component>(entity);
    Vector2 home = Vector2(0, tower->team == 0 ? ecs->application.screen_height : 0);

    tuple<Entity, Transform_Component*, Unit_Component*, float> closest_unit =
        make_tuple(Entity{}, nullptr, nullptr, INT_MAX);
    for (auto entity : ecs->Get_Entities_Of_Type(Get_Unit_Entity_Type())) {
        Entity_ID other_id = get<1>(entity)->Get_Entity_Data(entity).id;
        if (other_id == entity_id)
            continue;
        auto* unit = get<1>(entity)->Get_Component<Unit_Component>(entity);
        if (unit->team == tower->team || !unit->spawned)
            continue;
        auto* other_transform = get<1>(entity)->Get_Component<Transform_Component>(entity);
        if (Vector2Distance(transform->pos, other_transform->pos) > tower->range)
            continue;
        float new_dist = Vector2Distance(other_transform->pos, home);
        if (new_dist >= get<3>(closest_unit))
            continue;

        closest_unit = make_tuple(entity, other_transform, unit, new_dist);
    }

    if (get<1>(closest_unit) == nullptr)
        return;

    // Fire
    transform->rot = Get_Rotation_From_Positions(transform->pos, get<1>(closest_unit)->pos);

    auto projectile = ecs->Create_Entity(Get_Projectile_Entity_Type());
    auto* ui = get<1>(entity)->Get_Component<UI_Component>(entity);
    Init_Projectile(ecs, projectile, transform->pos, transform->rot,
                    {tower->team, tower->damage, tower->projectile_speed, tower->range},
                    tower->projectile_texture, transform->scale / 2, ui->color);

    tower->reload_time = tower->reload_speed;
}

Object_UI* Create_Tower_UI(Entity entity, Game_UI_Manager& game_ui_manager) {
    return new Tower_UI(entity, game_ui_manager);
}

Entity_Type* Get_Tower_Entity_Type() {
    return new Entity_Type(vector{&UI_Component::component_type,
                                  &Transform_Component::component_type,
                                  &Tower_Component::component_type});
}

Component_Type Tower_Component::component_type = Component_Type{"Tower", sizeof(Tower_Component)};
