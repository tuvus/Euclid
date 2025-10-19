#include <algorithm>

#include "ui/eui.h"

void EUI_HBox::Layout() {
    Alignment main_axis_alignment = style.horizontal_alignment;

    float cursor = pos.x + style.padding.left;
    float total_content_width = 0;
    float total_leaf_width = 0;

    // gap between elements in container
    float total_gap = 0;

    // gap for align-stretch
    float interval = 0;

    // for auto-sizing nested containers
    float default_spacing = 0;
    int num_containers = 0;

    int num_layout_children = 0;

    // calculate total non-container content height
    for (EUI_Element* child : children) {
        if (child->style.position == Position::Absolute) {
            continue;
        }
        if (child->Is_Container()) {
            num_containers++;
            continue;
        }
        child->Layout();
        total_leaf_width += child->preferred_size.x;
        num_layout_children++;
    }

    if (gap && num_layout_children)
        total_gap = gap * (num_layout_children - 1);

    if (children.size())
        default_spacing =
            (size.x - style.padding.left - style.padding.right - total_leaf_width) / num_containers;

    // place containers
    for (EUI_Element* child : children) {
        if (child->Get_Effective_Style().position == Position::Absolute) {
            continue;
        }
        if (child->Is_Container()) {
            child->pos = {cursor, pos.y + style.padding.top};
            child->size = {default_spacing, size.y - style.padding.top - style.padding.bottom};
            child->preferred_size = child->size;
            child->Layout();
        }
        total_content_width += child->preferred_size.x;
        cursor += child->preferred_size.x;
    }

    // pick starting cursor location
    cursor = pos.x + style.padding.left;
    switch (main_axis_alignment) {
        case Alignment::Center:
            cursor = pos.x + (size.x - total_content_width - total_gap + style.padding.left -
                              style.padding.right) /
                                 2.0f;
            break;
        case Alignment::End:
            cursor = pos.x + size.x - total_content_width - total_gap - style.padding.right;
            break;
        case Alignment::Stretch:
            interval = (size.x - total_content_width - style.padding.left - style.padding.right) /
                       (children.size() - 1);
            break;
        case Alignment::Start:
            break;
    }

    // place leaves
    for (int i = 0; i < children.size(); i++) {
        EUI_Element* child = children[i];

        if (!child->Is_Container() && child->Get_Effective_Style().position != Position::Absolute) {
            // clamp to min/max
            float width = std::clamp(child->preferred_size.x, child->min_size.x, child->max_size.x);
            float height =
                std::clamp(child->preferred_size.y, child->min_size.y, child->max_size.y);

            float x = cursor;
            float y = pos.y + style.padding.top;

            // cross axis alignment
            switch (child->style.vertical_alignment) {
                case Alignment::Center:
                    y = pos.y + (size.y - height + style.padding.top - style.padding.bottom) / 2.0f;
                    break;
                case Alignment::End:
                    y = pos.y + (size.y - height - style.padding.bottom);
                    break;
                case Alignment::Stretch:
                case Alignment::Start:
                default:
                    break;
            }

            // Final bounds
            child->pos = {x, y};
            child->size = {width, height};
        }

        // Advance along main axis
        if (gap && i != children.size() - 1)
            cursor += gap;
        if (interval)
            cursor += interval;
        cursor += child->size.x;
    }
}
