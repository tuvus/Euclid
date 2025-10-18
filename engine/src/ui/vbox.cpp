#include "ui/eui.h"
#include <algorithm>

void EUI_VBox::Layout() {
    Alignment main_axis_alignment = style.vertical_alignment;

    float cursor = pos.y + style.padding.top;
    float total_content_height = 0;

    // gap between elements in container
    float total_gap = 0;

    // gap for align-stretch
    float interval = 0;

    int num_layout_children = 0;

    // first pass: layout children and calculate their preferred sizes
    for (EUI_Element* child : children) {
        if (child->style.position == Position::Absolute) {
            continue;
        }

        // layout child to get its preferred size
        child->Layout();

        // calculate child dimensions based on sizing mode
        float child_width, child_height;

        if (child->Is_Container()) {
            // For Full-width children, don't call Get_Effective_Width yet
            if (child->style.width_sizing == Sizing::Full) {
                child_width = 0; // Will be calculated later
            } else {
                child_width = child->preferred_size.x;
            }
            child_height = child->Get_Effective_Height();
        } else {
            child_width = child->preferred_size.x;
            child_height = child->preferred_size.y;
        }

        // set preferred size but don't position yet
        child->preferred_size = {child_width, child_height};

        total_content_height += child->preferred_size.y;
        num_layout_children++;
    }

    // Handle full-width children (similar to HBox logic)
    float fixed_width = 0;
    int full_width_children = 0;
    for (EUI_Element* child : children) {
        if (child->style.width_sizing == Sizing::Full) {
            full_width_children++;
        } else {
            // Only count non-Full children in fixed width calculation
            fixed_width += child->preferred_size.x;
        }
    }

    // Calculate available width for full-width children
    float available_width =
        dim.x - style.padding.left - style.padding.right; // Available content width
    float full_width_per_child = 0;
    if (full_width_children > 0) {
        full_width_per_child =
            available_width; // Each full-width child gets the full available width
    }
    for (EUI_Element* child : children) {
        if (child->style.width_sizing == Sizing::Full) {
            child->preferred_size.x = full_width_per_child;
        }
    }

    // calculate container size if using auto sizing (after children are laid out)
    if (style.height_sizing == Sizing::Auto) {
        Vector2 child_size = Calculate_Size_From_Children();
        preferred_size = child_size;
        dim = child_size;
    }

    // Handle width sizing
    if (style.width_sizing == Sizing::Full) {
        // width should be set by parent during layout, don't override it
    } else if (style.width_sizing == Sizing::Fixed) {
        // width should already be set, but ensure it's correct
        if (style.width.has_value()) {
            dim.x = style.width.value();
        }
    } else if (style.width_sizing == Sizing::Auto) {
        // Width should be calculated from children
        Vector2 child_size = Calculate_Size_From_Children();
        dim.x = child_size.x;
    }

    if (gap && num_layout_children)
        total_gap = gap * (num_layout_children - 1);

    // calculate alignment and positioning
    float available_height = dim.y - style.padding.top - style.padding.bottom;
    cursor = pos.y + style.padding.top;
    switch (main_axis_alignment) {
        case Alignment::Center:
            cursor = pos.y + style.padding.top +
                     (available_height - total_content_height - total_gap) / 2.0f;
            break;
        case Alignment::End:
            cursor =
                pos.y + style.padding.top + available_height - total_content_height - total_gap;
            break;
        case Alignment::Stretch:
            if (num_layout_children >= 2) {
                interval = (available_height - total_content_height) / (num_layout_children - 1);
            } else {
                interval = 0;
            }
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
        float x = pos.x + style.padding.left;
        float y = cursor;

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
        float available_width = dim.x - style.padding.left - style.padding.right;
        switch (style.horizontal_alignment) {
            case Alignment::Center:
                x = pos.x + style.padding.left + (available_width - child->dim.x) / 2.0f;
                break;
            case Alignment::End:
                x = pos.x + style.padding.left + available_width - child->dim.x;
                break;
            case Alignment::Stretch:
                child->dim.x = available_width;
                x = pos.x + style.padding.left;
                break;
            case Alignment::Start:
            default:
                x = pos.x + style.padding.left;
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
        cursor += child->dim.y;
    }
}
