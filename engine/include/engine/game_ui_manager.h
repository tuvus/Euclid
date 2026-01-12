#pragma once
#include "ecs.h"
#include "game_manager.h"

#include <chrono>

class Object_UI;

struct UI_Component {
    static Component_Type component_type;
    Texture2D* texture;
    float scale;
    Color color;
};

class Game_UI_Manager {
  private:
    std::unordered_set<Entity_ID> to_create;
    std::unordered_set<Entity_ID> to_delete;

    void On_Create_Object(Entity_ID);
    void On_Delete_Object(Entity_ID);
    static Game_UI_Manager* game_ui_manager_instance;

  public:
    Application& application;
    ECS& ecs;
    Game_Manager& game_manager;
    std::unordered_map<Entity_ID, Object_UI*> active_ui_objects;

    Camera2D camera;

    Game_UI_Manager(Application& application, ECS& ecs, Game_Manager& game_manager);

    void Resize_UI(Vector2 screen_seize_change);
    void Update_UI(std::chrono::milliseconds delta_time, EUI_Context* eui_ctx);

    void DrawImage(Texture2D& texture, Vector2 pos, float rot, float scale, Color color);

    void DrawScreenImage(Texture2D& texture, Vector2 pos, float rot, float scale, Color color);
};
