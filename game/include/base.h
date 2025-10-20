#pragma once
#include "ecs.h"

class Path;
class Card_Player;
struct Base_Component {
    static Component_Type component_type;
    int base_income_speed;
    int time_until_income;
    int health;
    int max_health;
    Path* path;
    Card_Player& card_player;
};

void Init_Base(Entity entity, Card_Player& card_player, Vector2 pos, Path* path,
               int base_income_speed, int max_health);

void Update_Base(ECS* ecs, Entity entity);

Entity_Type* Get_Base_Entity_Type() {
    return new Entity_Type(
        vector{&Transform_Component::component_type, &Base_Component::component_type});
}
