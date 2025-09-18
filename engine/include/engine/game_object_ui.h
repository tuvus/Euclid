#pragma once
#include "game_object.h"

class Object_UI {
  public:
    Game_Object* game_object;

    Object_UI(Game_Object* game_object) : game_object(game_object) {}
    virtual ~Object_UI() = default;

    virtual void Update_UI(EUI_Context* eui_ctx) = 0;
};

template <typename T>
class Game_Object_UI : public Object_UI {
  public:
    T* object;
    Game_UI_Manager& game_ui_manager;

    Game_Object_UI(T* object, Game_UI_Manager& game_ui_manager)
        : Object_UI(static_cast<Game_Object*>(object)), object(object),
          game_ui_manager(game_ui_manager) {}
    virtual ~Game_Object_UI() = default;
};