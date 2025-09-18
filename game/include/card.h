#pragma once
#include "card_player.h"
#include "game_object.h"

#include <string>

class Game_Scene;
struct Card_Data {
    Texture2D texture;
    std::string name;
    std::string desc;
    int cost;
};

class Card : public Game_Object {
  public:
    Card_Data& card_data;
    Game_Scene& game_scene;

    Card(Game_Manager& game_manager, Game_Scene& game_scene, Card_Data& card_data);

    virtual Card* Clone() = 0;

    virtual bool Can_Play_Card(Card_Player* card_player, Vector2 pos);

    virtual void Play_Card(Card_Player* card_player, Vector2 pos);

    Object_UI* Create_UI_Object(Game_UI_Manager& game_ui_manager) override;
};