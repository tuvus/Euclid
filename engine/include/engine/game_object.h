#pragma once

#include <raylib.h>

class Object_UI;
class Game_UI_Manager;
class Game_Manager;
typedef long Obj_ID;

class Game_Object {
  public:
    Obj_ID id;
    Vector2 pos;
    float rot;
    Game_Manager& game_manager;
    float scale;
    Color color;

    Game_Object(Game_Manager& game_manager, Vector2 pos, float rot, float scale, Color color);
    void Delete_Object();
    virtual ~Game_Object();
    virtual void Update() = 0;
    virtual Object_UI* Create_UI_Object(Game_UI_Manager& game_ui_manager) = 0;
    long Get_Id() const;
};