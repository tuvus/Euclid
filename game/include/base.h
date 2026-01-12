#pragma once
#include "ecs.h"

class Game_Scene;
struct Card_Component;
class Path;
class Card_Player;
struct Base_Component {
    static Component_Type component_type;
    Game_Scene* game_scene;
    vector<Card_Player*> players;
    Entity_ID other_base_id;
    int team;
    std::vector<vector<Entity_ID>*> units_on_path;
    int base_income_speed;
    int time_until_income;
    int health;
    int max_health;
    vector<Path*> paths;
};

void Init_Base(Entity entity, Game_Scene* game_scene, vector<Card_Player*> players,
               Entity_ID other_base_id, int team, Vector2 pos, vector<Path*> paths,
               int base_income_speed, int max_health);

void Try_Placing_Tower(ECS* ecs, int card_index, Entity card_entity, Card_Component* card,
                       Card_Player* card_player, Base_Component* base);

void Base_Update(ECS* ecs, Entity entity);

Entity_Type* Get_Base_Entity_Type();
