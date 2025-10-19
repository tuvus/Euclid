#include "card_game.h"
#include "application.h"
#include "game_scene.h"
#include "lobby_scene.h"
#include "menu_scene.h"

void Card_Game::resize_update() {
    auto root = scene->Get_Root();
    if (root) {
        root->size = {(float) screen_width, (float) screen_height};
    }

    // TODO: should we panic if ctx is null?
    eui_ctx->Perform_Layout();
}

void Card_Game::set_ui_screen(SCREEN new_screen) {
    if (scene != nullptr)
        to_delete.emplace_back(scene);
    scene = sceens[new_screen]();
    // will panic if eui_ctx is null, shouldn't ever happen so let it crash
    // TODO: this should probably be in the engine
    eui_ctx->Set_Root(scene->Get_Root());
    eui_ctx->Perform_Layout();
}

void Card_Game::Start_Client() {
    Application::Start_Client();

    sceens.insert({MENU, [this]() -> Scene* { return new Menu_Scene(*this); }});
    sceens.insert({LOBBY, [this]() -> Scene* { return new Lobby_Scene(*this); }});
    sceens.insert({GAME, [this]() -> Scene* { return new Game_Scene(*this); }});

    set_ui_screen(MENU);

    resize_update();
}

void Card_Game::Update(chrono::milliseconds deltaTime) {
    scene->Update(deltaTime);
}

void Card_Game::Update_UI(chrono::milliseconds deltaTime) {
    if (IsKeyPressed(KEY_F)) {
        ToggleFullscreen();
        screen_width = GetScreenWidth();
        screen_height = GetScreenHeight();

        resize_update();
    }

    if (IsKeyPressed(KEY_B)) {
        ToggleBorderlessWindowed();
        screen_width = GetScreenWidth();
        screen_height = GetScreenHeight();

        resize_update();
    }

    if (IsKeyPressed(KEY_Q) && IsKeyDown(KEY_LEFT_CONTROL)) {
        Close_Application();
        return;
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
}

void Card_Game::Close_Application() {
    delete scene;
    scene = nullptr;
    Application::Close_Application();
}
