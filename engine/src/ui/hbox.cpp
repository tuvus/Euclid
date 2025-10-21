#include <algorithm>

#include "ui/eui.h"

void EUI_HBox::Layout() {
    Alignment main_axis_alignment = horizontal_alignment;

    float cursor = pos.x + padding.left;
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
        if (child->position == Position::Absolute) {
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
            (size.x - padding.left - padding.right - total_leaf_width) / num_containers;

    // place containers
    for (EUI_Element* child : children) {
        if (child->position == Position::Absolute) {
            continue;
        }
        if (child->Is_Container()) {
            child->pos = {cursor, pos.y + padding.top};
            child->size = {default_spacing, size.y - padding.top - padding.bottom};
            child->preferred_size = child->size;
            child->Layout();
        }
        total_content_width += child->preferred_size.x;
        cursor += child->preferred_size.x;
    }

    // pick starting cursor location
    cursor = pos.x + padding.left;
    switch (main_axis_alignment) {
        case Alignment::Center:
            cursor =
                pos.x +
                (size.x - total_content_width - total_gap + padding.left - padding.right) / 2.0f;
            break;
        case Alignment::End:
            cursor = pos.x + size.x - total_content_width - total_gap - padding.right;
            break;
        case Alignment::Stretch:
            interval = (size.x - total_content_width - padding.left - padding.right) /
                       (children.size() - 1);
            break;
        case Alignment::Start:
            break;
    }

    // place leaves
    for (int i = 0; i < children.size(); i++) {
        EUI_Element* child = children[i];

        if (!child->Is_Container() && child->position != Position::Absolute) {
            // clamp to min/max
            float width = std::clamp(child->preferred_size.x, child->min_size.x, child->max_size.x);
            float height =
                std::clamp(child->preferred_size.y, child->min_size.y, child->max_size.y);

            float x = cursor;
            float y = pos.y + padding.top;

            // cross axis alignment
            switch (child->vertical_alignment) {
                case Alignment::Center:
                    y = pos.y + (size.y - height + padding.top - padding.bottom) / 2.0f;
                    break;
                case Alignment::End:
                    y = pos.y + (size.y - height - padding.bottom);
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
