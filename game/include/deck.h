#pragma once
#include "game_object.h"

#include <deque>
#include <vector>

class Card;
class Card_Player;

class Deck : public Game_Object {
  public:
    Card_Player* player;
    std::deque<Card*> deck;
    std::vector<Card*> hand;
    std::vector<Card*> discard;
    Deck(Game_Manager& game_manager, Card_Player* player);

    void Draw_Card(int cards = 1);

    void Shuffle_Discard_Into_Deck();

    void Shuffle_Deck();

    void Update() override;

    void Discard_Card(Card*);

    Object_UI* Create_UI_Object(Game_UI_Manager& game_ui_manager) override;
};