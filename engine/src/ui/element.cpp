#include "ui/eui.h"

#define RETURN_STYLE_PROP(prop, getter)                                                            \
    if (style.prop.has_value())                                                                    \
        return style.prop.value();                                                                 \
    if (parent)                                                                                    \
        return parent->getter();                                                                   \
    return ctx->default_style.prop.value();

/* EUI_Element Implementations */
void EUI_Element::Set_Context(EUI_Context& ctx) {
    this->ctx = &ctx;
}

Color EUI_Element::Get_Text_Color() const {
    RETURN_STYLE_PROP(text_color, Get_Text_Color);
}

Font EUI_Element::Get_Font() const {
    RETURN_STYLE_PROP(font, Get_Font);
}
float EUI_Element::Get_Font_Size() const {
    RETURN_STYLE_PROP(font_size, Get_Font_Size);
}
float EUI_Element::Get_Font_Spacing() const {
    RETURN_STYLE_PROP(font_spacing, Get_Font_Spacing);
}

EUI_Style EUI_Element::Get_Effective_Style() const {
    EUI_Style effective;

    effective.background_color = style.background_color;
    effective.text_color = Get_Text_Color();

    effective.padding = style.padding;
    effective.margin = style.margin;

    effective.position = style.position;
    effective.top = style.top;
    effective.right = style.right;
    effective.bottom = style.bottom;
    effective.left = style.left;

    effective.border_color = style.border_color;
    effective.border_radius = style.border_radius;

    effective.font = Get_Font();
    effective.font_size = Get_Font_Size();
    effective.font_spacing = Get_Font_Spacing();

    effective.horizontal_alignment = style.horizontal_alignment;
    effective.vertical_alignment = style.vertical_alignment;
    effective.text_horizontal_alignment = style.text_horizontal_alignment;
    effective.text_vertical_alignment = style.text_vertical_alignment;

    return effective;
}

float EUI_Element::Get_Effective_Width() const {
    switch (style.width_sizing) {
        case Sizing::Fixed:
            if (style.width.has_value()) {
                return style.width.value();
            }
            break;
        case Sizing::Full:
            // If preferred_size has been set by parent during layout, use that
            if (preferred_size.x > 0) {
                return preferred_size.x;
            }
            // Otherwise, calculate from parent
            if (parent) {
                float parent_width = parent->Get_Effective_Width() - parent->style.padding.left - parent->style.padding.right;
                return parent_width;
            }
            break;
        case Sizing::Auto:
        default:
            // For containers, this will be calculated based on children
            // For non-containers, use preferred size
            if (Is_Container()) {
                return preferred_size.x;
            }
            if (style.width.has_value()) {
                return style.width.value();
            }
            // For non-containers with Auto sizing, use preferred size
            return preferred_size.x;
    }
    
    // Fallback
    if (parent) {
        return parent->Get_Effective_Width();
    }
    return ctx->default_style.width.value_or(100.0f);
}

float EUI_Element::Get_Effective_Height() const {
    switch (style.height_sizing) {
        case Sizing::Fixed:
            if (style.height.has_value()) {
                return style.height.value();
            }
            break;
        case Sizing::Full:
            if (parent) {
                return parent->Get_Effective_Height() - parent->style.padding.top - parent->style.padding.bottom;
            }
            break;
        case Sizing::Auto:
        default:
            // For containers, this will be calculated based on children
            // For non-containers, use preferred size
            if (Is_Container()) {
                return preferred_size.y;
            }
            if (style.height.has_value()) {
                return style.height.value();
            }
            break;
    }
    
    // Fallback
    if (parent) {
        return parent->Get_Effective_Height();
    }
    return ctx->default_style.height.value_or(100.0f);
}

void EUI_Element::Set_Width_Full() {
    style.width_sizing = Sizing::Full;
}

void EUI_Element::Set_Height_Full() {
    style.height_sizing = Sizing::Full;
}

void EUI_Element::Set_Width_Fixed(float value) {
    style.width_sizing = Sizing::Fixed;
    style.width = value;
}

void EUI_Element::Set_Height_Fixed(float value) {
    style.height_sizing = Sizing::Fixed;
    style.height = value;
}

void EUI_Element::Set_Width_Auto() {
    style.width_sizing = Sizing::Auto;
}

void EUI_Element::Set_Height_Auto() {
    style.height_sizing = Sizing::Auto;
}
