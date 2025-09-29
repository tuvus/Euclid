#include <algorithm>

#include "ui/eui.h"

void EUI_HBox::Layout() {
    Alignment main_axis_alignment = style.horizontal_alignment;

    float cursor = pos.x + style.padding.left;
    float total_content_width = 0;

    // gap between elements in container
    float total_gap = 0;

    // gap for align-stretch
    float interval = 0;

    int num_layout_children = 0;

    // first pass: layout children and calculate their preferred sizes
    float fixed_width = 0;
    int full_width_children = 0;

    for (EUI_Element* child : children) {
        if (child->style.position == Position::Absolute) {
            continue;
        }

        // layout child to get its preferred size
        child->Layout();

        // calculate child dimensions based on sizing mode
        float child_width, child_height;

        if (child->style.width_sizing == Sizing::Full) {
            // Don't calculate width for full-width children yet
            full_width_children++;
            child_width = 0; // Will be calculated later
        } else if (child->Is_Container()) {
            child_width = child->Get_Effective_Width();
            child_height = child->Get_Effective_Height();
        } else {
            child_width = child->preferred_size.x;
            child_height = child->preferred_size.y;
        }

        // set preferred size but don't position yet
        child->preferred_size = {child_width, child_height};

        if (child->style.width_sizing != Sizing::Full) {
            fixed_width += child->preferred_size.x;
        }
        num_layout_children++;
    }

    // Calculate gap first
    if (gap && num_layout_children)
        total_gap = gap * (num_layout_children - 1);

    // Calculate available width for full-width children
    float available_width = dim.x - style.padding.left - style.padding.right;
    float full_width_per_child = 0;
    if (full_width_children > 0) {
        full_width_per_child = (available_width - fixed_width - total_gap) / full_width_children;
    }

    // Update full-width children with their calculated width
    for (EUI_Element* child : children) {
        if (child->style.position == Position::Absolute) {
            continue;
        }

        if (child->style.width_sizing == Sizing::Full) {
            child->preferred_size.x = full_width_per_child;
        }
    }

    total_content_width = fixed_width + (full_width_per_child * full_width_children);

    // calculate container size if using auto sizing (after children are laid out)
    if (style.width_sizing == Sizing::Auto) {
        Vector2 child_size = Calculate_Size_From_Children();
        preferred_size = child_size;
        dim = child_size;
    }

    // calculate alignment and positioning
    cursor = pos.x + style.padding.left;
    switch (main_axis_alignment) {
        case Alignment::Center:
            cursor = pos.x + style.padding.left +
                     (available_width - total_content_width - total_gap) / 2.0f;
            break;
        case Alignment::End:
            cursor = pos.x + style.padding.left + available_width - total_content_width - total_gap;
            break;
        case Alignment::Stretch:
            interval = (available_width - total_content_width) / (children.size() - 1);
            break;
        case Alignment::Start:
            break;
    }

    // second pass: position all children
    for (int i = 0; i < children.size(); i++) {
        EUI_Element* child = children[i];

        if (child->Get_Effective_Style().position == Position::Absolute) {
            continue;
        }

        // position child
        float x = cursor;
        float y = pos.y + style.padding.top;

        // set child dimensions
        float width, height;
        if (child->Is_Container()) {
            width = child->Get_Effective_Width();
            height = child->Get_Effective_Height();
        } else {
            // for non-containers, apply min/max constraints
            width = std::clamp(child->preferred_size.x, child->min_size.x, child->max_size.x);
            height = std::clamp(child->preferred_size.y, child->min_size.y, child->max_size.y);
        }
        child->dim = {width, height};

        // cross axis alignment
        float available_height = dim.y - style.padding.top - style.padding.bottom;
        switch (child->style.vertical_alignment) {
            case Alignment::Center:
                y = pos.y + style.padding.top + (available_height - child->dim.y) / 2.0f;
                break;
            case Alignment::End:
                y = pos.y + style.padding.top + available_height - child->dim.y;
                break;
            case Alignment::Stretch:
                child->dim.y = available_height;
                y = pos.y + style.padding.top;
                break;
            case Alignment::Start:
            default:
                y = pos.y + style.padding.top;
                break;
        }

        // final bounds
        child->pos = {x, y};

        // Update text position for text elements
        if (EUI_Text* text_elem = dynamic_cast<EUI_Text*>(child)) {
            text_elem->Calculate_Text_Position();
        }

        // Advance along main axis
        if (gap && i != children.size() - 1)
            cursor += gap;
        if (interval)
            cursor += interval;
        cursor += child->dim.x;
    }
}
