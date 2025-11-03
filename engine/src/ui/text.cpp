#include <algorithm>
#include <iostream>

#include "ui/eui.h"

// Layout debugging helpers (shared with box.cpp via extern)
extern thread_local int layout_depth;
static std::string get_indent() {
    return std::string(layout_depth * 2, ' ');
}

EUI_Text::EUI_Text(const std::string& text) : text(text) {
    // default styles
    border_radius = 0;

    horizontal_alignment = Alignment::Start;
    vertical_alignment = Alignment::Start;
    text_horizontal_alignment = Alignment::Start;
    text_vertical_alignment = Alignment::Start;
}

void EUI_Text::Size() {
    std::cout << get_indent() << "[SIZE] Text '" << text << "'" << std::endl;
    layout_depth++;

    Vector2 text_size =
        MeasureTextEx(Get_Font(), text.c_str(), Get_Font_Size(), Get_Font_Spacing());

    // min size is text and padding
    min_size = {text_size.x + padding.left + padding.right,
                text_size.y + padding.top + padding.bottom};

    // set size to min size or fixed set size only if its larger
    if (size.x != Size::Grow()) {
        size.x = std::max(min_size.x, size.x);
    }
    if (size.y != Size::Grow()) {
        size.y = std::max(min_size.y, size.y);
    };

    // TODO: what should this be...
    max_size = {9999, 9999};

    layout_depth--;
    std::cout << get_indent() << "  → text_size=(" << text_size.x << ", " << text_size.y
              << ") size=(" << size.x << ", " << size.y << ") min=(" << min_size.x << ", " << min_size.y << ")" << std::endl;
}

void EUI_Text::Grow() {
    std::cout << get_indent() << "[GROW] Text '" << text << "' before=(" << size.x << ", " << size.y << ")" << std::endl;
    layout_depth++;

    if (!parent) {
        std::cout << get_indent() << "No parent, skipping grow" << std::endl;
        layout_depth--;
        return;
    }

    // Grow to fill parent's available space (minus padding)
    // If main-axis was marked as Grow(), parent's distribution logic already set the size
    // If cross-axis is marked as Grow(), we handle it here
    if (size.x == Size::Grow()) {
        size.x = parent->size.x - parent->padding.left - parent->padding.right;
        std::cout << get_indent() << "Growing X to " << size.x << std::endl;
    }
    if (size.y == Size::Grow()) {
        size.y = parent->size.y - parent->padding.top - parent->padding.bottom;
        std::cout << get_indent() << "Growing Y to " << size.y << std::endl;
    }

    layout_depth--;
    std::cout << get_indent() << "  → final size=(" << size.x << ", " << size.y << ")" << std::endl;
}

void EUI_Text::Place() {
    std::cout << get_indent() << "[PLACE] Text '" << text << "' at pos=(" << pos.x << ", " << pos.y
              << ") size=(" << size.x << ", " << size.y << ")" << std::endl;
    layout_depth++;

    // Calculate text position based on alignment within element bounds
    Vector2 text_size =
        MeasureTextEx(Get_Font(), text.c_str(), Get_Font_Size(), Get_Font_Spacing());

    // Vertical alignment
    switch (text_vertical_alignment) {
        case Alignment::Center:
            text_pos.y = pos.y + (size.y - text_size.y + padding.top - padding.bottom) / 2.0f;
            break;
        case Alignment::End:
            text_pos.y = pos.y + size.y - text_size.y - padding.bottom;
            break;
        case Alignment::Stretch:
        case Alignment::Start:
            text_pos.y = pos.y + padding.top;
            break;
    }

    // Horizontal alignment
    switch (text_horizontal_alignment) {
        case Alignment::Center:
            text_pos.x = pos.x + (size.x - text_size.x + padding.left - padding.right) / 2.0f;
            break;
        case Alignment::End:
            text_pos.x = pos.x + size.x - text_size.x - padding.right;
            break;
        case Alignment::Stretch:
        case Alignment::Start:
            text_pos.x = pos.x + padding.left;
            break;
    }

    layout_depth--;
    std::cout << get_indent() << "  → text_pos=(" << text_pos.x << ", " << text_pos.y << ")" << std::endl;
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

    // Text (position already calculated in Place())
    DrawTextEx(Get_Font(), text.c_str(), text_pos, Get_Font_Size(), Get_Font_Spacing(),
               Get_Text_Color());
}

std::string& EUI_Text::Get_Text() {
    return this->text;
}

void EUI_Text::Set_Text(const std::string& text) {
    this->text = text;

    // recalculate size
    if (ctx && parent) {
        parent->Size();
        parent->Grow();
        parent->Place();
    }
}
