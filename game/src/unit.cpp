
#include "unit.h"

#include "game_manager.h"
#include "unit_ui.h"

void Init_Unit(unsigned char* entity_data, Entity_Array& entity_array, Unit_Data& unit_data,
               Path* path, float speed, float start_offset, int team, float scale, Color color) {
    auto* unit =
        entity_array.Get_Component<Unit_Component>(entity_data, &Unit_Component::component_type);
    auto* transform = entity_array.Get_Component<Transform_Component>(
        entity_data, &Transform_Component::component_type);
    unit->path = path;
    unit->speed = speed;
    unit->section = 0;
    unit->team = team;
    unit->unit_data = &unit_data;
    Move_Unit(unit, transform, start_offset);
}

void Move_Unit(Unit_Component* unit, Transform_Component* transform, float dist_to_move) {
    if (unit->section + 1 == unit->path->positions.size()) {
        transform->pos = unit->path->positions[unit->section];
        // Delete_Object();
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
            // Delete_Object();
            return;
        }
    }

    // for (auto* object : game_manager.Get_All_Objects()) {
    //     if (Unit* other = dynamic_cast<Unit*>(object)) {
    //         if (other->team == unit->team || !other->spawned)
    //             continue;
    //
    //         if (Vector2Distance(transform->pos, other->pos) > 30)
    //             continue;
    //
    //         // Collide
    //         unit->spawned = false;
    //         other->spawned = false;
    //         // game_manager.Delete_Object(this);
    //         // game_manager.Delete_Object(other);
    //         return;
    //     }
    // }

    transform->pos = Vector2Lerp(unit->path->positions[unit->section],
                                 unit->path->positions[unit->section + 1], unit->lerp);
    transform->rot = unit->path->Get_Rotation_On_Path(unit->section);
}

void Unit_Update(ECS* ecs, Entity_Array* entity_array, unsigned char* entity_data) {
    auto* unit =
        entity_array->Get_Component<Unit_Component>(entity_data, &Unit_Component::component_type);
    auto* transform = entity_array->Get_Component<Transform_Component>(
        entity_data, &Transform_Component::component_type);
    Move_Unit(unit, transform, 10);
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
    auto [entity, array] = ecs->entities_by_id[tmp_ecs_unit];
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