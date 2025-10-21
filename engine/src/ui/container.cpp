#include "ui/eui.h"

EUI_Container::~EUI_Container() {
    for (EUI_Element* child : children) {
        delete child;
    }
}

void EUI_Container::Add_Child(EUI_Element* child) {
    if (!child)
        return;

    child->parent = this;
    children.push_back(child);
    if (ctx) {
        child->Set_Context(*ctx);
    }
}

void EUI_Container::Set_Context(EUI_Context& ctx) {
    this->ctx = &ctx;
    for (auto* child : children) {
        child->Set_Context(ctx);
    }
}

void EUI_Container::Handle_Input() {
    if (!is_visible || is_deleted)
        return;

    for (EUI_Element* child : children) {
        if (child->is_visible) {
            child->Handle_Input();
            if (is_deleted)
                return;
        }
    }
}

void EUI_Container::Render() {
    if (!is_visible)
        return;

    if (background_color.has_value())
        DrawRectangleRec({pos.x, pos.y, size.x, size.y}, background_color.value());

    if (border_radius > 0)
        DrawRectangleLinesEx({pos.x, pos.y, size.x, size.y}, border_radius, border_color);

    // Render children
    for (EUI_Element* child : children) {
        if (child->is_visible)
            child->Render();
    }
}

void EUI_Container::Delete() {
    for (EUI_Element* child : children) {
        child->Delete();
    }
}
