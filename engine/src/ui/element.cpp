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
