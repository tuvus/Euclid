#pragma once
#include "game_object.h"

struct Tower_Data {
    Texture2D texture;
};

class Tower : public Game_Object {
  public:
    int team;
    bool spawned;
    int reload;
    float range;
    Tower_Data& tower_data;

    Tower(Game_Manager& game_manager, Tower_Data& tower_data, Vector2 pos, float range, int team,
          float scale, Color color);

    void Update() override;

    Object_UI* Create_UI_Object(Game_UI_Manager& game_ui_manager) override;
};