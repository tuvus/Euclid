#include <algorithm>

#include "ui/eui.h"

EUI_Text::EUI_Text(const std::string& text) : text(text) {
    // default styles
    border_radius = 0;

    horizontal_alignment = Alignment::Start;
    vertical_alignment = Alignment::Start;
    text_horizontal_alignment = Alignment::Start;
    text_vertical_alignment = Alignment::Start;
}

void EUI_Text::Layout() {
    Vector2 text_size =
        MeasureTextEx(Get_Font(), text.c_str(), Get_Font_Size(), Get_Font_Spacing());

    // calculate preferred size
    float width = std::max(text_size.x + padding.left + padding.right, preferred_size.x);
    float height = std::max(text_size.y + padding.top + padding.bottom, preferred_size.y);
    preferred_size = {width, height};

    min_size = {text_size.x, text_size.y};
    // TODO: what should this be...
    max_size = {9999, 9999};
}

void EUI_Text::Handle_Input() {
    // no op for now
}

void EUI_Text::Render() {
    if (!is_visible)
        return;

    // Background
    if (background_color.has_value())
        DrawRectangleRec({pos.x, pos.y, size.x, size.y}, background_color.value());

    // Border
    if (border_radius > 0)
        DrawRectangleLinesEx({pos.x, pos.y, size.x, size.y}, border_radius, border_color);

    // Text
    Vector2 text_size =
        MeasureTextEx(Get_Font(), text.c_str(), Get_Font_Size(), Get_Font_Spacing());

    switch (text_vertical_alignment) {
        case Alignment::Center:
            text_pos.y = pos.y + (size.y - text_size.y + padding.top - padding.bottom) / 2.0f;
            break;
        case Alignment::End:
            text_pos.y = pos.y + size.y - text_size.y - padding.bottom;
            break;
        case Alignment::Stretch:
        case Alignment::Start:
            text_pos.y = pos.y;
    }
    switch (text_horizontal_alignment) {
        case Alignment::Center:
            text_pos.x = pos.x + (size.x - text_size.x + padding.left - padding.right) / 2;
            break;
        case Alignment::End:
            text_pos.x = pos.x + size.x - text_size.x - padding.right;
            break;
        case Alignment::Stretch:
        case Alignment::Start:
            text_pos.x = pos.x;
    }

    DrawTextEx(Get_Font(), text.c_str(), text_pos, Get_Font_Size(), Get_Font_Spacing(),
               Get_Text_Color());
}

std::string& EUI_Text::Get_Text() {
    return this->text;
}

void EUI_Text::Set_Text(const std::string& text) {
    this->text = text;

    // recalculate size
    if (ctx && parent)
        parent->Layout();
}
