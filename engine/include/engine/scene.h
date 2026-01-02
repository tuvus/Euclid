#pragma once

#include <chrono>

#include "ui/eui.h"

class Scene {
  protected:
    Application& application;
    EUI_Element* root_elem;

  public:
    Scene(Application& app) : application(app) {}
    virtual ~Scene() { delete root_elem; }

    virtual void Resize_UI(Vector2 screen_seize_change) {};
    virtual void Update_UI(std::chrono::milliseconds) = 0;
    virtual void Update(std::chrono::milliseconds) = 0;

    EUI_Element* Get_Root() { return root_elem; }
};
