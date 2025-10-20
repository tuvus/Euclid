#include "game_ui_manager.h"
#include "game_object_ui.h"

Game_UI_Manager::Game_UI_Manager(Application& application, ECS& ecs)
    : application(application), ecs(ecs) {
    camera = {0};
    camera.target = {static_cast<float>(application.screen_width) / 2,
                     static_cast<float>(application.screen_height) / 2};
    camera.offset = {static_cast<float>(application.screen_width) / 2,
                     static_cast<float>(application.screen_height) / 2};
    camera.rotation = 0;
    camera.zoom = 1.0f;
    active_ui_objects = unordered_map<Entity_ID, Object_UI*>();
    to_create = unordered_set<Entity_ID>();
    to_delete = unordered_set<Entity_ID>();
    game_ui_manager_instance = this;
    ecs.on_add_entity = [](Entity_ID id) { game_ui_manager_instance->On_Create_Object(id); };
    ecs.on_add_entity = [](Entity_ID id) { game_ui_manager_instance->On_Delete_Object(id); };
}

void Game_UI_Manager::Update_UI(std::chrono::milliseconds delta_time, EUI_Context* eui_ctx) {
    for (auto id : to_create) {
        if (to_delete.contains(id))
            continue;
        Entity entity = get<0>(ecs.entities_by_id[id]);
        Object_UI* new_object_ui = get<1>(entity)->entity_type.ui_creation_function(entity, *this);
        if (new_object_ui == nullptr)
            continue;
        active_ui_objects.emplace(id, new_object_ui);
    }
    to_create.clear();

    for (auto id : to_delete) {
        active_ui_objects.erase(id);
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

void Game_UI_Manager::On_Create_Object(Entity_ID id) {
    to_create.emplace(id);
}

void Game_UI_Manager::On_Delete_Object(Entity_ID id) {
    if (to_create.contains(id))
        to_create.erase(id);
    else
        to_delete.emplace(id);
}

Game_UI_Manager* Game_UI_Manager::game_ui_manager_instance = nullptr;
Component_Type UI_Component::component_type = Component_Type{"UI", sizeof(UI_Component)};
