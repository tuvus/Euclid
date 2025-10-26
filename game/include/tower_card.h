#pragma once
#include "card.h"
#include "path.h"

struct Tower_Card_Component {
    static Component_Type component_type;
    Texture2D* tower_texture;
    int team;
    bool spawned;
    int reload;
    float range;
};

Entity_ID Init_Tower_Card(Entity entity, Card_Data* card_data,
                          Tower_Card_Component tower_card_component, Texture2D* texture,
                          float scale, Color color);

bool Can_Play_Tower_Card(Card_Player* card_player, Entity entity, Vector2 pos);

bool Can_Place_Tower(Entity tower_card, vector<Path*> path, Vector2 pos, float min_dist);

void Play_Tower_Card(Card_Player* card_player, Entity entity, Vector2 pos);

Entity_Type* Get_Tower_Card_Entity_Type();
