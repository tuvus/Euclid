#include "game_ui_manager.h"

#include "game_object_ui.h"

Game_UI_Manager::Game_UI_Manager(Application& application, Game_Manager& game_manager)
    : application(application), game_manager(game_manager) {
    camera = {0};
    camera.target = {static_cast<float>(application.screen_width) / 2,
                     static_cast<float>(application.screen_height) / 2};
    camera.offset = {static_cast<float>(application.screen_width) / 2,
                     static_cast<float>(application.screen_height) / 2};
    camera.rotation = 0;
    camera.zoom = 1.0f;
    active_ui_objects = unordered_map<Game_Object*, Object_UI*>();
    to_create = unordered_set<Game_Object*>();
    to_delete = unordered_set<Game_Object*>();
    game_ui_manager_instance = this;
    game_manager.on_add_object = [](Game_Object* obj) {
        game_ui_manager_instance->On_Create_Object(obj);
    };
    game_manager.on_delete_object = [](Game_Object* obj) {
        game_ui_manager_instance->On_Delete_Object(obj);
    };
}

void Game_UI_Manager::Update_UI(std::chrono::milliseconds delta_time, EUI_Context* eui_ctx) {
    for (auto* new_obj : to_create) {
        if (to_delete.contains(new_obj))
            continue;
        Object_UI* new_object_ui = new_obj->Create_UI_Object(*this);
        if (new_object_ui == nullptr)
            continue;
        active_ui_objects.emplace(static_cast<Game_Object*>(new_obj), new_object_ui);
    }
    to_create.clear();

    for (auto* new_obj : to_delete) {
        active_ui_objects.erase(static_cast<Game_Object*>(new_obj));
    }
    to_delete.clear();

    for (auto [obj, obj_ui] : active_ui_objects) {
        obj_ui->Update_UI(eui_ctx);
    }
}

void Game_UI_Manager::DrawImage(Texture2D& texture, Vector2 pos, float rot, float scale,
                                Color color) {
    BeginMode2D(camera);
    DrawTexturePro(texture, {0, 0, (float) texture.width, (float) texture.height},
                   {pos.x, pos.y, texture.width * scale, texture.height * scale},
                   Vector2(texture.width * scale / 2, texture.height * scale / 2), rot, color);
    EndMode2D();
}

void Game_UI_Manager::DrawScreenImage(Texture2D& texture, Vector2 pos, float rot, float scale,
                                      Color color) {
    DrawTexturePro(texture, {0, 0, (float) texture.width, (float) texture.height},
                   {pos.x, pos.y, texture.width * scale, texture.height * scale},
                   Vector2(texture.width * scale / 2, texture.height * scale / 2), rot, color);
}

void Game_UI_Manager::On_Create_Object(Game_Object* obj) {
    to_create.emplace(obj);
}

void Game_UI_Manager::On_Delete_Object(Game_Object* obj) {
    if (to_create.contains(obj))
        to_create.erase(obj);
    else
        to_delete.emplace(obj);
}

Game_UI_Manager* Game_UI_Manager::game_ui_manager_instance = nullptr;