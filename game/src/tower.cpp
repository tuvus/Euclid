#include "tower.h"

#include "game_manager.h"
#include "tower_ui.h"
#include "unit.h"

#include <climits>
#include <raymath.h>

void Init_Tower(Entity entity, Vector2 pos, float range, int team, Texture2D* texture, float scale,
                Color color) {
    auto* tower =
        get<1>(entity)->Get_Component<Tower_Component>(entity, &Tower_Component::component_type);
    auto* transform = get<1>(entity)->Get_Component<Transform_Component>(
        entity, &Transform_Component::component_type);
    auto* ui = get<1>(entity)->Get_Component<UI_Component>(entity, &UI_Component::component_type);
    transform->pos = pos;
    transform->rot = 0;
    transform->scale = .4;
    tower->range = range;
    tower->team = team;
    ui->texture = texture;
    ui->scale = scale;
    ui->color = color;
}

void Tower_Update(ECS* ecs, Entity entity) {
    auto* tower =
        get<1>(entity)->Get_Component<Tower_Component>(entity, &Tower_Component::component_type);
    if (tower->reload > 0)
        tower->reload--;
    if (tower->reload > 0)
        return;

    Entity_ID entity_id = Entity_Array::Get_Entity_Data(entity).id;
    auto* transform = get<1>(entity)->Get_Component<Transform_Component>(
        entity, &Transform_Component::component_type);
    Vector2 home = Vector2(tower->team == 0 ? ecs->application.screen_height : 0,
                           ecs->application.screen_width / 2);

    tuple<Entity, Transform_Component*, Unit_Component*, float> closest_unit =
        make_tuple(Entity{}, nullptr, nullptr, INT_MAX);
    for (auto entity : ecs->Get_Entities_Of_Type(Get_Unit_Entity_Type())) {
        Entity_ID other_id = get<1>(entity)->Get_Entity_Data(entity).id;
        if (other_id == entity_id)
            continue;
        Unit_Component* unit =
            get<1>(entity)->Get_Component<Unit_Component>(entity, &Unit_Component::component_type);
        if (unit->team == tower->team || !unit->spawned)
            continue;
        Transform_Component* other_transform = get<1>(entity)->Get_Component<Transform_Component>(
            entity, &Transform_Component::component_type);
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
    get<2>(closest_unit)->spawned = false;
    ecs->Delete_Entity(Entity_Array::Get_Entity_Data(get<0>(closest_unit)).id);
    tower->reload = 100;
    transform->rot = Get_Rotation_From_Positions(transform->pos, get<1>(closest_unit)->pos);
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
