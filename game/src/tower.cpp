#include "tower.h"

#include "game_manager.h"
#include "tower_ui.h"
#include "unit.h"

#include <climits>

void Init_Tower(ECS* ecs, unsigned char* entity, Entity_Array& entity_array, Tower_Data* tower_data,
                Vector2 pos, float range, int team, float scale, Color color) {
    auto* tower =
        entity_array.Get_Component<Tower_Component>(entity, &Tower_Component::component_type);
    auto* transform = entity_array.Get_Component<Transform_Component>(
        entity, &Transform_Component::component_type);
    transform->pos = pos;
    transform->rot = 0;
    tower->tower_data = tower_data;
    tower->range = range;
    tower->team = team;
}

void Tower_Update(ECS* ecs, Entity_Array* entity_array, unsigned char* entity) {
    auto* tower =
        entity_array->Get_Component<Tower_Component>(entity, &Tower_Component::component_type);
    if (tower->reload > 0)
        tower->reload--;
    if (tower->reload > 0)
        return;

    Entity_ID entity_id = Entity_Array::Get_Entity_Data(entity).id;
    auto* transform = entity_array->Get_Component<Transform_Component>(
        entity, &Transform_Component::component_type);
    Vector2 home = Vector2(tower->team == 0 ? ecs->application.screen_height : 0,
                           ecs->application.screen_width / 2);

    auto components = vector<Component_Type*>();
    components.emplace_back(&Transform_Component::component_type);
    components.emplace_back(&Unit_Component::component_type);

    tuple<unsigned char*, Transform_Component*, Unit_Component*, float> closest_unit =
        make_tuple(nullptr, nullptr, nullptr, INT_MAX);
    for (auto [entity, entity_array] : ecs->Get_Entities_Of_Type(new Entity_Type(components))) {
        Entity_ID other_id = entity_array->Get_Entity_Data(entity).id;
        if (other_id == entity_id)
            continue;
        Unit_Component* unit =
            entity_array->Get_Component<Unit_Component>(entity, &Unit_Component::component_type);
        if (unit->team == tower->team || !unit->spawned)
            continue;
        Transform_Component* other_transform = entity_array->Get_Component<Transform_Component>(
            entity, &Transform_Component::component_type);
        if (Vector2Distance(transform->pos, other_transform->pos) > tower->range)
            continue;
        float new_dist = Vector2Distance(other_transform->pos, home);
        if (new_dist >= get<3>(closest_unit))
            continue;

        closest_unit = make_tuple(entity, other_transform, unit, new_dist);
    }

    if (get<0>(closest_unit) == nullptr)
        return;

    // Fire
    get<2>(closest_unit)->spawned = false;
    ecs->Delete_Entity(Entity_Array::Get_Entity_Data(get<0>(closest_unit)).id);
    tower->reload = 100;
    transform->rot = Get_Rotation_From_Positions(transform->pos, get<1>(closest_unit)->pos);
}

Tower::Tower(ECS* ecs, Game_Manager& game_manager, Tower_Data& tower_data, Vector2 pos, float range,
             int team, float scale, Color color, Entity_ID tmp_ecs_tower)
    : Game_Object(game_manager, pos, team == 0 ? 0 : 180, scale, color), ecs(ecs),
      tower_data(tower_data), range(range), team(team), tmp_ecs_tower(tmp_ecs_tower) {
    spawned = true;
    reload = 100;
}

void Tower::Update() {
    if (!ecs->entities_by_id.contains(tmp_ecs_tower)) {
        Delete_Object();
        return;
    }
    auto [entity, index, array] = ecs->entities_by_id[tmp_ecs_tower];
    if (Entity_Array::Get_Entity_Data(entity).id == 0) {
        Delete_Object();
        return;
    }
    auto transform =
        array->Get_Component<Transform_Component>(entity, &Transform_Component::component_type);
    pos = transform->pos;
    rot = transform->rot;
}

Object_UI* Tower::Create_UI_Object(Game_UI_Manager& game_ui_manager) {
    return new Tower_UI(this, game_ui_manager);
}
Component_Type Tower_Component::component_type = Component_Type{"Tower", sizeof(Tower_Component)};
