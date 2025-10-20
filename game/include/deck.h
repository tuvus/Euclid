#pragma once
#include "ecs.h"

#include <deque>
#include <vector>

class Deck_UI;
class Card;
class Card_Player;

struct Deck_Component {
    static Component_Type component_type;
    Card_Player* player;
    std::deque<Entity_ID> deck;
    std::vector<Entity_ID> hand;
    std::vector<Entity_ID> discard;
};

void Init_Deck(Entity entity, Card_Player* player) {
    auto deck =
        std::get<1>(entity)->Get_Component<Deck_Component>(entity, &Deck_Component::component_type);
    deck->player = player;
    deck->deck = std::deque<Entity_ID>();
    deck->hand = std::vector<Entity_ID>();
    deck->discard = std::vector<Entity_ID>();
}

void Draw_Card(Entity entity, int cards = 1);
void Shuffle_Discard_Into_Deck(Entity entity);
void Shuffle_Deck(Entity entity);
void Discard_Card(Entity entity, Entity_ID);

Object_UI* Create_Deck_UI(Entity entity, Game_UI_Manager& game_ui_manager);
