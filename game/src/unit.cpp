
#include "unit.h"

#include "game_manager.h"
#include "unit_ui.h"

Unit::Unit(Game_Manager& game_manager, Unit_Data& unit_data, Path* path, float speed,
           float start_offset, int team, float scale, Color color)
    : Game_Object(game_manager, path->positions[0], path->Get_Rotation_On_Path(0), scale, color),
      path(path), section(0), lerp(0), speed(speed), team(team), unit_data(unit_data) {
    spawned = true;
    Move(start_offset);
}

void Unit::Update() {
    if (!spawned)
        return;
    Move(speed);
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
