#pragma once
#include "ecs.h"
#include "game_object.h"

#include <deque>
#include <vector>

class Card;
class Card_Player;

class Deck : public Game_Object {
  public:
    Card_Player* player;
    std::deque<Entity_ID> deck;
    std::vector<Entity_ID> hand;
    std::vector<Entity_ID> discard;
    Deck(Game_Manager& game_manager, Card_Player* player);

    void Draw_Card(int cards = 1);

    void Shuffle_Discard_Into_Deck();

    void Shuffle_Deck();

    void Update() override;

    void Discard_Card(Entity_ID);

    Object_UI* Create_UI_Object(Game_UI_Manager& game_ui_manager) override;
};