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

    float text_width = MeasureText(text.c_str(), style.font_size.value());
    float text_height = style.font_size.value();

    // calculate preferred size
    float width = std::max(text_width + style.font_spacing.value() * text.length() +
                               style.padding.left + style.padding.right,
                           preferred_size.x);
    float height =
        std::max(text_height + style.padding.top + style.padding.bottom, preferred_size.y);
    preferred_size = {width, height};

    min_size = {text_width, text_height};
    // TODO: what should this be...
    max_size = {9999, 9999};
}

void EUI_Text::Handle_Input() {
    // no op for now
}

void EUI_Text::Render() {
    if (!is_visible)
        return;

    const auto style = Get_Effective_Style();

    // Background
    if (style.background_color.has_value())
        DrawRectangleRec({pos.x, pos.y, size.x, size.y}, style.background_color.value());

    // Border
    if (style.border_radius > 0)
        DrawRectangleLinesEx({pos.x, pos.y, size.x, size.y}, style.border_radius,
                             style.border_color);

    // Text
    float text_width = MeasureText(text.c_str(), style.font_size.value());
    float text_height = style.font_size.value();

    switch (style.text_vertical_alignment) {
        case Alignment::Center:
            text_pos.y =
                pos.y + (size.y - text_height + style.padding.top - style.padding.bottom) / 2.0f;
            break;
        case Alignment::End:
            text_pos.y = pos.y + size.y - text_height - style.padding.bottom;
            break;
        case Alignment::Stretch:
        case Alignment::Start:
            text_pos.y = pos.y;
    }
    switch (style.text_horizontal_alignment) {
        case Alignment::Center:
            text_pos.x =
                pos.x + (size.x - text_width + style.padding.left - style.padding.right) / 2;
            break;
        case Alignment::End:
            text_pos.x = pos.x + size.x - text_width - style.padding.right;
            break;
        case Alignment::Stretch:
        case Alignment::Start:
            text_pos.x = pos.x;
    }

    DrawText(text.c_str(), text_pos.x, text_pos.y, style.font_size.value(),
             style.text_color.value());
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
