#pragma once
#include "card.h"
#include "tower.h"

class Tower_Card : public Card {
  public:
    Tower_Data& tower_data;

    Tower_Card(Game_Manager& game_manager, Game_Scene& game_scene, Card_Data& card_data,
               Tower_Data& tower_data);

    Card* Clone() override;

    bool Can_Play_Card(Card_Player* card_player, Vector2 pos) override;

    void Play_Card(Card_Player* player, Vector2 pos) override;

    void Update() override {}
};