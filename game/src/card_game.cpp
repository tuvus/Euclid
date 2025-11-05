#include <raylib.h>

#include "application.h"
#include "card_game.h"
#include "game_scene.h"
#include "lobby_scene.h"
#include "menu_scene.h"
#include "test_scene.h"

void Card_Game::resize_update() {
    screen_width = GetScreenWidth();
    screen_height = GetScreenHeight();

    auto root = scene->Get_Root();
    if (root) {
        root->size = {static_cast<float>(screen_width), static_cast<float>(screen_height)};
    }

    root->ctx->Perform_Layout();
}

void Card_Game::set_ui_screen(SCREEN new_screen) {
    if (scene != nullptr)
        to_delete.emplace_back(scene);
    scene = scenes[new_screen]();
    // will panic if eui_ctx is null, shouldn't ever happen so let it crash
    // TODO: this should probably be in the engine
    eui_ctx->Set_Root(scene->Get_Root());
    eui_ctx->Perform_Layout();
}

void Card_Game::Start_Client() {
    Application::Start_Client();

    scenes.insert({MENU, [this]() -> Scene* { return new Menu_Scene(*this); }});
    scenes.insert({LOBBY, [this]() -> Scene* { return new Lobby_Scene(*this); }});
    scenes.insert({GAME, [this]() -> Scene* { return new Game_Scene(*this); }});
    scenes.insert({TEST, [this]() -> Scene* { return new Test_Scene(*this); }});

    set_ui_screen(MENU);

    resize_update();
}

void Card_Game::Update(chrono::milliseconds deltaTime) {
    scene->Update(deltaTime);
}

void Card_Game::Update_UI(chrono::milliseconds deltaTime) {
    if (IsKeyPressed(KEY_F)) {
        ToggleFullscreen();
        resize_update();
    }

    if (IsKeyPressed(KEY_B)) {
        ToggleBorderlessWindowed();
        resize_update();
    }

    if (screen_width != GetScreenWidth() || screen_height != GetScreenHeight()) {
        resize_update();
    }

    if (IsKeyPressed(KEY_Q) && IsKeyDown(KEY_LEFT_CONTROL)) {
        Close_Application();
        return;
    }

    if (IsKeyPressed(KEY_T)) {
        set_ui_screen(TEST);
    }

    if (IsKeyPressed(KEY_M)) {
        set_ui_screen(MENU);
    }

    eui_ctx->Begin_Frame();
    eui_ctx->Update_Input();
    eui_ctx->Handle_Input();
    eui_ctx->End_Frame();

    if (WindowShouldClose()) {
        Close_Application();
        return;
    }

    BeginDrawing();

    ClearBackground(RAYWHITE);
    scene->Update_UI(deltaTime);

    EndDrawing();

    eui_ctx->Render();

    for (auto scene_to_delete : to_delete) {
        delete scene_to_delete;
    }
    to_delete.clear();
}

Card_Game::~Card_Game() {
    delete scene;
    scene = nullptr;
    UnloadFont(eui_ctx->default_font);
}

void Card_Game::Close_Application() {
    delete scene;
    scene = nullptr;
    Application::Close_Application();
}
