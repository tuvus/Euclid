
#include "unit.h"

#include "game_manager.h"
#include "unit_ui.h"

#include <raymath.h>

void Init_Unit(ECS* ecs, Entity entity, Path* path, float speed, float start_offset, int team,
               Texture2D* texture, float scale, Color color) {
    auto* unit =
        get<1>(entity)->Get_Component<Unit_Component>(entity, &Unit_Component::component_type);
    auto* transform = get<1>(entity)->Get_Component<Transform_Component>(
        entity, &Transform_Component::component_type);
    auto* ui = get<1>(entity)->Get_Component<UI_Component>(entity, &UI_Component::component_type);
    unit->path = path;
    unit->speed = speed;
    unit->section = 0;
    unit->team = team;
    unit->spawned = true;
    ui->texture = texture;
    ui->scale = scale;
    ui->color = color;
    Move_Unit(ecs, unit, transform, Entity_Array::Get_Entity_Data(entity).id, start_offset);
}

void Move_Unit(ECS* ecs, Unit_Component* unit, Transform_Component* transform, Entity_ID entity_id,
               float dist_to_move) {
    if (unit->section + 1 == unit->path->positions.size()) {
        transform->pos = unit->path->positions[unit->section];
        ecs->Delete_Entity(entity_id);
        return;
    }

    while (dist_to_move > 0) {
        float dist = Vector2Distance(unit->path->positions[unit->section],
                                     unit->path->positions[unit->section + 1]);
        float new_dist_to_move = dist_to_move - dist * (1 - unit->lerp);
        unit->lerp += dist_to_move / dist;
        dist_to_move = new_dist_to_move;
        if (unit->lerp > 1) {
            unit->lerp = 0;
            unit->section++;
        }
        if (unit->section + 1 == unit->path->positions.size()) {
            transform->pos = unit->path->positions[unit->section];
            ecs->Delete_Entity(entity_id);
            return;
        }
    }

    transform->pos = Vector2Lerp(unit->path->positions[unit->section],
                                 unit->path->positions[unit->section + 1], unit->lerp);
    transform->rot = unit->path->Get_Rotation_On_Path(unit->section);

    for (auto entity : ecs->Get_Entities_Of_Type(Get_Unit_Entity_Type())) {
        Entity_ID other_id = get<1>(entity)->Get_Entity_Data(entity).id;
        if (other_id == entity_id)
            continue;
        Unit_Component* other =
            get<1>(entity)->Get_Component<Unit_Component>(entity, &Unit_Component::component_type);

        if (other->team == unit->team || !other->spawned)
            continue;

        Transform_Component* other_transform = get<1>(entity)->Get_Component<Transform_Component>(
            entity, &Transform_Component::component_type);
        if (Vector2Distance(transform->pos, other_transform->pos) > 30)
            continue;

        // Collide
        unit->spawned = false;
        other->spawned = false;
        ecs->Delete_Entity(entity_id);
        ecs->Delete_Entity(other_id);
        return;
    }
}

void Unit_Update(ECS* ecs, Entity entity) {
    auto* unit =
        get<1>(entity)->Get_Component<Unit_Component>(entity, &Unit_Component::component_type);
    auto* transform = get<1>(entity)->Get_Component<Transform_Component>(
        entity, &Transform_Component::component_type);
    Move_Unit(ecs, unit, transform, Entity_Array::Get_Entity_Data(entity).id, unit->speed);
}

Object_UI* Create_Unit_UI(Entity entity, Game_UI_Manager& game_ui_manager) {
    return new Unit_UI(entity, game_ui_manager);
}

Entity_Type* Get_Unit_Entity_Type() {
    return new Entity_Type(vector{&UI_Component::component_type,
                                  &Transform_Component::component_type,
                                  &Unit_Component::component_type});
}

Component_Type Unit_Component::component_type = Component_Type{"Unit", sizeof(Unit_Component)};
