
#include "unit.h"

#include "game_manager.h"
#include "unit_ui.h"

void Init_Unit(ECS* ecs, unsigned char* entity_data, Entity_Array& entity_array,
               Unit_Data& unit_data, Path* path, float speed, float start_offset, int team,
               float scale, Color color) {
    auto* unit =
        entity_array.Get_Component<Unit_Component>(entity_data, &Unit_Component::component_type);
    auto* transform = entity_array.Get_Component<Transform_Component>(
        entity_data, &Transform_Component::component_type);
    unit->path = path;
    unit->speed = speed;
    unit->section = 0;
    unit->team = team;
    unit->unit_data = &unit_data;
    Move_Unit(ecs, unit, transform, Entity_Array::Get_Entity_Data(entity_data).id, start_offset);
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

    auto components = vector<Component_Type*>();
    components.emplace_back(&Transform_Component::component_type);
    components.emplace_back(&Unit_Component::component_type);
    for (auto [entity, entity_array] : ecs->Get_Entities_Of_Type(new Entity_Type(components))) {
        Entity_ID other_id = entity_array->Get_Entity_Data(entity).id;
        if (other_id == entity_id)
            continue;
        Unit_Component* other =
            entity_array->Get_Component<Unit_Component>(entity, &Unit_Component::component_type);

        if (other->team == unit->team || !other->spawned)
            continue;

        Transform_Component* other_transform = entity_array->Get_Component<Transform_Component>(
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

void Unit_Update(ECS* ecs, Entity_Array* entity_array, unsigned char* entity_data) {
    auto* unit =
        entity_array->Get_Component<Unit_Component>(entity_data, &Unit_Component::component_type);
    auto* transform = entity_array->Get_Component<Transform_Component>(
        entity_data, &Transform_Component::component_type);
    Move_Unit(ecs, unit, transform, Entity_Array::Get_Entity_Data(entity_data).id, unit->speed);
}

Unit::Unit(ECS* ecs, Game_Manager& game_manager, Unit_Data& unit_data, Path* path, float speed,
           float start_offset, int team, float scale, Color color, Entity_ID tmp_ecs_unit)
    : Game_Object(game_manager, path->positions[0], path->Get_Rotation_On_Path(0), scale, color),
      ecs(ecs), path(path), section(0), lerp(0), speed(speed), team(team), unit_data(unit_data),
      tmp_ecs_unit(tmp_ecs_unit) {
    spawned = true;
}

void Unit::Update() {
    // if (!spawned)
    // return;
    // Move(speed);
    if (!ecs->entities_by_id.contains(tmp_ecs_unit)) {
        Delete_Object();
        return;
    }
    auto [entity, index, array] = ecs->entities_by_id[tmp_ecs_unit];
    if (Entity_Array::Get_Entity_Data(entity).id == 0) {
        Delete_Object();
        return;
    }
    auto transform =
        array->Get_Component<Transform_Component>(entity, &Transform_Component::component_type);
    pos = transform->pos;
    rot = transform->rot;
}

void Unit::Move(float dist_to_move) {
    if (section + 1 == path->positions.size()) {
        pos = path->positions[section];
        Delete_Object();
        return;
    }

    float dist = Vector2Distance(path->positions[section], path->positions[section + 1]);
    lerp += dist_to_move / dist;
    if (lerp > 1) {
        lerp--;
        section++;
    }
    if (section + 1 == path->positions.size()) {
        pos = path->positions[section];
        Delete_Object();
        return;
    }

    for (auto* object : game_manager.Get_All_Objects()) {
        if (Unit* other = dynamic_cast<Unit*>(object)) {
            if (other->team == team || !other->spawned)
                continue;

            if (Vector2Distance(pos, other->pos) > 30)
                continue;

            // Collide
            spawned = false;
            other->spawned = false;
            game_manager.Delete_Object(this);
            game_manager.Delete_Object(other);
            return;
        }
    }

    pos = Vector2Lerp(path->positions[section], path->positions[section + 1], lerp);
    rot = path->Get_Rotation_On_Path(section);
}

Object_UI* Unit::Create_UI_Object(Game_UI_Manager& game_ui_manager) {
    return new Unit_UI(this, game_ui_manager);
}

Component_Type Unit_Component::component_type = Component_Type{"Unit", sizeof(Unit_Component)};