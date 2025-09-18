#pragma once
#include "game_object.h"

class Path;
class Card_Player;
class Base : public Game_Object {
  public:
    int time_until_income;
    int health;
    Path* path;
    Card_Player& card_player;

    Base(Game_Manager& game_manager, Card_Player& card_player, Vector2 pos, Path* path,
         int base_income_speed, int max_health);

    void Update() override;

    Object_UI* Create_UI_Object(Game_UI_Manager& game_ui_manager) override { return nullptr; }
};