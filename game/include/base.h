#pragma once
#include "ecs.h"

struct Card_Component;
class Path;
class Card_Player;
struct Base_Component {
    static Component_Type component_type;
    int base_income_speed;
    int time_until_income;
    int health;
    int max_health;
    vector<Path*> paths;
    Card_Player* card_player;
};

void Init_Base(Entity entity, Card_Player* card_player, Vector2 pos, vector<Path*> paths,
               int base_income_speed, int max_health);

void Try_Placing_Tower(ECS* ecs, int card_index, Entity card_entity, Card_Component* card,
                       Base_Component* base);

void Base_Update(ECS* ecs, Entity entity);

Entity_Type* Get_Base_Entity_Type();
