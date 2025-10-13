#pragma once

#include <chrono>
#include <iostream>
#include <raylib.h>

#include "application.h"
#include "scene.h"
#include "ui/eui.h"

using namespace std;

#define GRID_W 16
#define GRID_H 9
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define TITLE_FONT_SIZE 24

enum SCREEN { MENU, LOBBY, GAME };

typedef struct Tile {
    Color color;
} Tile;

class Card_Game : public Application {
  private:
    vector<Scene*> to_delete;
    void resize_update();
    Font custom_font; // Store the loaded font

  protected:
    void Start_Client() override;

  public:
    Card_Game(bool client) : Application("CARD GAME", client, SCREEN_WIDTH, SCREEN_HEIGHT) {
        scene = nullptr;
        // Font will be loaded in Start_Client() after OpenGL context is initialized
    }

    void Update(chrono::milliseconds s) override;
    void Update_UI(chrono::milliseconds s) override;
    void set_ui_screen(SCREEN screen);

    ~Card_Game() override;

    void Close_Application() override;

    Scene* scene;
    std::unordered_map<SCREEN, std::function<Scene*()>> sceens;
};
