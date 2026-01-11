
#include "unit.h"

#include "base.h"
#include "game_manager.h"
#include "unit_ui.h"

#include <raymath.h>

void Init_Unit(ECS* ecs, Entity entity, Entity_ID base, Path* path, float speed, int health,
               int damage, float start_offset, int team, Texture2D* texture, float scale,
               Color color) {
    auto* unit = get<1>(entity)->Get_Component<Unit_Component>(entity);
    auto* transform = get<1>(entity)->Get_Component<Transform_Component>(entity);
    auto* ui = get<1>(entity)->Get_Component<UI_Component>(entity);
    unit->base_id = base;
    unit->path = path;
    unit->speed = speed;
    unit->health = health;
    unit->damage = damage;
    unit->section = 0;
    unit->lerp = 1;
    unit->bump_back = 0;
    unit->team = team;
    unit->spawned = true;
    transform->scale = 1;
    ui->texture = texture;
    ui->scale = scale;
    ui->color = color;
    Move_Unit(ecs, unit, transform, entity, start_offset);
}

void Move_Unit(ECS* ecs, Unit_Component* unit, Transform_Component* transform, Entity entity,
               float dist_to_move) {
    if (dist_to_move > 0) {
        if (unit->section + 1 == unit->path->positions.size()) {
            transform->pos = unit->path->positions[unit->section];
            ecs->Delete_Entity(entity);
            return;
        }
        while (dist_to_move > 0) {
            float dist = Vector2Distance(unit->path->positions[unit->section],
                                         unit->path->positions[unit->section + 1]);
            float new_dist_to_move = dist_to_move - dist * (1 - unit->lerp);
            unit->lerp += dist_to_move / dist;
            dist_to_move = new_dist_to_move;
            if (unit->lerp >= 1) {
                unit->lerp = 0;
                unit->section++;
            }
            if (unit->section + 1 == unit->path->positions.size()) {
                transform->pos = unit->path->positions[unit->section];
                ecs->Delete_Entity(entity);
                return;
            }
        }
    } else if (dist_to_move < 0) {
        if (unit->section == 0) {
            transform->pos = unit->path->positions[0];
            return;
        }
        while (dist_to_move < 0) {
            float dist = Vector2Distance(unit->path->positions[unit->section],
                                         unit->path->positions[unit->section - 1]);
            float new_dist_to_move = dist_to_move + dist * unit->lerp;
            unit->lerp += dist_to_move / dist;
            dist_to_move = new_dist_to_move;
            if (unit->lerp < 0) {
                unit->lerp = 1;
                unit->section--;
            }
            if (unit->section == 0) {
                transform->pos = unit->path->positions[0];
                return;
            }
        }
    }
    transform->pos = Vector2Lerp(unit->path->positions[unit->section],
                                 unit->path->positions[unit->section + 1], unit->lerp);
    transform->rot = unit->path->Get_Rotation_On_Path(unit->section);

    Entity_ID entity_id = Entity_Array::Get_Entity_ID(entity);
    auto base_entity = get<0>(ecs->entities_by_id[unit->base_id]);
    auto* base = get<1>(base_entity)->Get_Component<Base_Component>(base_entity);
    auto other_base_entity = get<0>(ecs->entities_by_id[base->other_base_id]);
    auto* other_base = get<1>(other_base_entity)->Get_Component<Base_Component>(other_base_entity);
    for (auto other_id : *other_base->units_on_path[unit->path->index]) {
        auto other_entity = get<0>(ecs->entities_by_id[other_id]);
        if (other_id == entity_id)
            continue;
        auto* other = get<1>(other_entity)->Get_Component<Unit_Component>(other_entity);

        if (other->team == unit->team || !other->spawned)
            continue;

        auto* other_transform =
            get<1>(other_entity)->Get_Component<Transform_Component>(other_entity);
        if (Vector2Distance(transform->pos, other_transform->pos) > 30)
            continue;

        // Collide
        unit->health -= other->damage;
        other->health -= unit->damage;
        if (unit->health <= 0) {
            unit->spawned = false;
            ecs->Delete_Entity(entity);
        } else {
            unit->bump_back = other->damage * 20;
        }
        if (other->health <= 0) {
            other->spawned = false;
            ecs->Delete_Entity(other_entity);
        } else {
            other->bump_back = unit->damage * 20;
        }
        return;
    }
}

void Setup_Unit(Entity entity) {
    auto* unit = get<1>(entity)->Get_Component<Unit_Component>(entity);
    auto base_entity = get<0>(get<1>(entity)->ecs.entities_by_id[unit->base_id]);
    auto* base = get<1>(base_entity)->Get_Component<Base_Component>(base_entity);
    base->units_on_path[unit->path->index]->emplace_back(Entity_Array::Get_Entity_ID(entity));
}

void Delete_Unit(Entity entity) {
    auto* unit = get<1>(entity)->Get_Component<Unit_Component>(entity);
    auto base_entity = get<0>(get<1>(entity)->ecs.entities_by_id[unit->base_id]);
    auto* base = get<1>(base_entity)->Get_Component<Base_Component>(base_entity);

    auto& units_on_path = *base->units_on_path[unit->path->index];
    auto found_entity = ranges::find(units_on_path, Entity_Array::Get_Entity_ID(entity));
    if (found_entity != units_on_path.end())
        units_on_path.erase(found_entity);
    else
        throw runtime_error(
            "Could not find the unit on the path to delete! Was it already deleted?");
}

void Unit_Update(ECS* ecs, Entity entity) {
    auto* unit = get<1>(entity)->Get_Component<Unit_Component>(entity);
    auto* transform = get<1>(entity)->Get_Component<Transform_Component>(entity);
    if (unit->bump_back > 0.000001) {
        float bump = min(unit->bump_back, 3.0f);
        Move_Unit(ecs, unit, transform, entity, -bump);
        unit->bump_back -= bump;
    } else {
        Move_Unit(ecs, unit, transform, entity, unit->speed);
    }
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
