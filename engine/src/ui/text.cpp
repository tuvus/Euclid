#include <algorithm>

#include "ui/eui.h"

EUI_Text::EUI_Text(const std::string& text) : text(text) {
    // default styles
    style.border_radius = 0;

    style.horizontal_alignment = Alignment::Start;
    style.vertical_alignment = Alignment::Start;
    style.text_horizontal_alignment = Alignment::Start;
    style.text_vertical_alignment = Alignment::Start;
}

void EUI_Text::Layout() {
    EUI_Style style = Get_Effective_Style();

    float font_size = style.font_size.value_or(15.0f);
    float text_width = MeasureText(text.c_str(), font_size);
    float text_height = font_size;

    // calculate preferred size
    float font_spacing = style.font_spacing.value_or(1.0f);
    float width = text_width + font_spacing * text.length() +
                  style.padding.left + style.padding.right;
    float height = text_height + style.padding.top + style.padding.bottom;
    preferred_size = {width, height};

    min_size = {text_width, text_height};
    // TODO: what should this be...
    max_size = {9999, 9999};
    
    // Calculate text position based on current element position and dimensions
    // This will be updated when the element is positioned by its parent
    Calculate_Text_Position();
}

void EUI_Text::Handle_Input() {
    // no op for now
}

void EUI_Text::Calculate_Text_Position() {
    const auto style = Get_Effective_Style();
    float font_size = style.font_size.value_or(15.0f);
    float text_width = MeasureText(text.c_str(), font_size);
    float text_height = font_size;

    switch (style.text_vertical_alignment) {
        case Alignment::Center:
            text_pos.y =
                pos.y + (dim.y - text_height + style.padding.top - style.padding.bottom) / 2.0f;
            break;
        case Alignment::End:
            text_pos.y = pos.y + dim.y - text_height - style.padding.bottom;
            break;
        case Alignment::Stretch:
        case Alignment::Start:
        default:
            text_pos.y = pos.y + style.padding.top;
            break;
    }
    switch (style.text_horizontal_alignment) {
        case Alignment::Center:
            text_pos.x =
                pos.x + (dim.x - text_width + style.padding.left - style.padding.right) / 2;
            break;
        case Alignment::End:
            text_pos.x = pos.x + dim.x - text_width - style.padding.right;
            break;
        case Alignment::Stretch:
        case Alignment::Start:
        default:
            text_pos.x = pos.x + style.padding.left;
            break;
    }
}

void EUI_Text::Render() {
    if (!is_visible)
        return;

    const auto style = Get_Effective_Style();

    // Background
    if (style.background_color.has_value())
        DrawRectangleRec({pos.x, pos.y, dim.x, dim.y}, style.background_color.value());

    // Border
    if (style.border_radius > 0)
        DrawRectangleLinesEx({pos.x, pos.y, dim.x, dim.y}, style.border_radius, style.border_color);

    // Update text position in case element was repositioned
    Calculate_Text_Position();

    // Text
    float font_size = style.font_size.value_or(15.0f);
    Color text_color = style.text_color.value_or(BLACK);
    DrawText(text.c_str(), text_pos.x, text_pos.y, font_size, text_color);
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
