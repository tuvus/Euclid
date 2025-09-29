#include "ui/eui.h"

EUI_Container::EUI_Container(Layout_Model lm) : layout_model(lm) {
    style.display = Display::Block;
}

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

    const auto style = Get_Effective_Style();

    if (style.background_color.has_value())
        DrawRectangleRec({pos.x, pos.y, dim.x, dim.y}, style.background_color.value());

    if (style.border_radius > 0)
        DrawRectangleLinesEx({pos.x, pos.y, dim.x, dim.y}, style.border_radius, style.border_color);

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

Vector2 EUI_Container::Calculate_Size_From_Children() {
    if (children.empty()) {
        return {style.padding.left + style.padding.right, style.padding.top + style.padding.bottom};
    }

    float total_width = 0;
    float total_height = 0;
    float max_width = 0;
    float max_height = 0;
    int visible_children = 0;

    for (EUI_Element* child : children) {
        if (child->style.position == Position::Absolute || !child->is_visible) {
            continue;
        }

        visible_children++;

        // Use the child's preferred size (children should already be laid out)
        if (layout_model == Layout_Model::Horizontal) {
            // For HBox, sum widths and take max height
            total_width += child->preferred_size.x;
            max_height = std::max(max_height, child->preferred_size.y);
        } else {
            // For VBox, sum heights and take max width
            total_height += child->preferred_size.y;
            // Skip children with Full width sizing when calculating Auto parent width
            if (child->style.width_sizing != Sizing::Full) {
                max_width = std::max(max_width, child->preferred_size.x);
            }
        }
    }

    // Add gaps
    if (gap > 0 && visible_children > 1) {
        if (layout_model == Layout_Model::Horizontal) {
            total_width += gap * (visible_children - 1);
        } else {
            total_height += gap * (visible_children - 1);
        }
    }

    // Add padding
    float final_width = (layout_model == Layout_Model::Horizontal)
                            ? total_width + style.padding.left + style.padding.right
                            : max_width + style.padding.left + style.padding.right;

    float final_height = (layout_model == Layout_Model::Horizontal)
                             ? max_height + style.padding.top + style.padding.bottom
                             : total_height + style.padding.top + style.padding.bottom;

    return {final_width, final_height};
}
