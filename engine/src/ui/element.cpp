#include "ui/eui.h"

#define RETURN_STYLE_PROP(prop, getter, default)                                                   \
    if (prop.has_value())                                                                          \
        return prop.value();                                                                       \
    if (parent)                                                                                    \
        return parent->getter();                                                                   \
    return ctx->default;

/* EUI_Element Implementations */
void EUI_Element::Set_Context(EUI_Context& ctx) {
    this->ctx = &ctx;
}

Color EUI_Element::Get_Text_Color() const {
    RETURN_STYLE_PROP(text_color, Get_Text_Color, default_text_color);
}
Font EUI_Element::Get_Font() const {
    RETURN_STYLE_PROP(font, Get_Font, default_font);
}
float EUI_Element::Get_Font_Size() const {
    RETURN_STYLE_PROP(font_size, Get_Font_Size, default_font_size);
}
float EUI_Element::Get_Font_Spacing() const {
    RETURN_STYLE_PROP(font_spacing, Get_Font_Spacing, default_font_spacing);
}
