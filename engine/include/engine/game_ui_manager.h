#pragma once
#include "game_manager.h"

#include <chrono>

class Object_UI;
class Game_UI_Manager {
  private:
    std::unordered_set<Game_Object*> to_create;
    std::unordered_set<Game_Object*> to_delete;

    void On_Create_Object(Game_Object*);
    void On_Delete_Object(Game_Object*);
    static Game_UI_Manager* game_ui_manager_instance;

  public:
    Application& application;
    Game_Manager& game_manager;
    std::unordered_map<Game_Object*, Object_UI*> active_ui_objects;

    Camera2D camera;

    Game_UI_Manager(Application& application, Game_Manager& game_manager);

    void Update_UI(std::chrono::milliseconds delta_time, EUI_Context* eui_ctx);

    void DrawImage(Texture2D& texture, Vector2 pos, float rot, float scale, Color color);

    void DrawScreenImage(Texture2D& texture, Vector2 pos, float rot, float scale, Color color);
};
