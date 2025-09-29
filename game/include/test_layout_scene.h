#pragma once

#include "card_game.h"
#include "scene.h"

class Test_Layout_Scene : public Scene {
  private:
    Camera2D camera;
    float camera_speed = 300.0f; // pixels per second
    
  public:
    Test_Layout_Scene(Card_Game& card_game);
    void Update_UI(chrono::milliseconds) override;
    void Update(chrono::milliseconds) override;
};
