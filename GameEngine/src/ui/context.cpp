#include "ui/eui.h"

/* EUI_Context Implementations */
EUI_Context::EUI_Context() {
    input = {};
}

void EUI_Context::Update_Input() {
    input = {0};
    input.mouse_position = GetMousePosition();
    input.left_mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    input.left_mouse_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input.right_mouse_down = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    input.right_mouse_pressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    input.key_pressed = GetKeyPressed();
}
void EUI_Context::Begin_Frame() {
    hovered = nullptr;
    active = nullptr;
}
void EUI_Context::End_Frame() {
}
void EUI_Context::Perform_Layout() {
    if (root)
        root->Layout(*this);
}
void EUI_Context::Handle_Input() {
    if (root)
        root->Handle_Input(*this);
}
void EUI_Context::Render() {
    if (root)
        root->Render(*this);
}
