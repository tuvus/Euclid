#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

#include "ui/eui.h"

// Layout debugging helpers
thread_local int layout_depth = 0; // Shared across translation units
static std::string get_indent() {
    return std::string(layout_depth * 2, ' ');
}

EUI_Box::~EUI_Box() {
    for (EUI_Element* child : children) {
        delete child;
    }
}

void EUI_Box::Add_Child(EUI_Element* child) {
    if (!child)
        return;

    child->parent = this;
    children.push_back(child);
    if (ctx) {
        child->Set_Context(*ctx);
    }
}

void EUI_Box::Set_Context(EUI_Context& ctx) {
    this->ctx = &ctx;
    for (auto* child : children) {
        child->Set_Context(ctx);
    }
}

void EUI_Box::Handle_Input() {
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

void EUI_Box::Size() {
    std::cout << get_indent() << "[SIZE] Box ("
              << (layout_model == Layout_Model::Horizontal ? "HBox" : "VBox") << ") id='" << id
              << "'" << std::endl;
    layout_depth++;

    // FIT SIZING
    float children_main_size = 0;
    float max_child_cross_size = 0;
    // # of visible, non-absolutely positioned children
    int visible_count = 0;

    // Size children first (size bottom up)
    for (EUI_Element* child : children) {
        if (child->is_visible) {
            child->Size();

            // Skip absolutely positioned children
            if (child->Is_Absolute()) {
                std::cout << get_indent() << "Skipping absolute child in size calculation"
                          << std::endl;
                continue;
            }

            visible_count++;
            if (layout_model == Layout_Model::Horizontal) {
                children_main_size += child->size.x != Size::Grow() ? child->size.x : 0;
                max_child_cross_size = std::max(max_child_cross_size, child->size.y);
            } else {
                children_main_size += child->size.y != Size::Grow() ? child->size.y : 0;
                max_child_cross_size = std::max(max_child_cross_size, child->size.x);
            }
        }
    }

    float total_gap = visible_count > 1 ? gap * (visible_count - 1) : 0;

    // min size is fit size
    min_size.x = layout_model == Layout_Model::Horizontal
                     ? children_main_size + padding.left + padding.right + total_gap
                     : max_child_cross_size + padding.left + padding.right;
    min_size.y = layout_model == Layout_Model::Horizontal
                     ? max_child_cross_size + padding.top + padding.bottom
                     : children_main_size + padding.top + padding.bottom + total_gap;

    if (size.x != Size::Grow()) {
        size.x = std::max(min_size.x, size.x);
    }
    if (size.y != Size::Grow()) {
        size.y = std::max(min_size.y, size.y);
    };

    layout_depth--;
    std::cout << get_indent() << "  → size=(" << size.x << ", " << size.y << ") min=(" << min_size.x
              << ", " << min_size.y << ") padding=(" << padding.top << "," << padding.right << ","
              << padding.bottom << "," << padding.left << ") gap=" << gap << std::endl;
}

void EUI_Box::Grow() {
    std::cout << get_indent() << "[GROW] Box ("
              << (layout_model == Layout_Model::Horizontal ? "HBox" : "VBox") << ") id='" << id
              << "' before=(" << size.x << ", " << size.y << ")" << std::endl;
    layout_depth++;

    // Calculate remaining space Within this box to distribute to children
    float remaining_width = size.x - padding.left - padding.right;
    float remaining_height = size.y - padding.top - padding.bottom;
    std::vector<EUI_Element*> growable;
    int visible_count = 0;

    for (EUI_Element* child : children) {
        if (!child->is_visible) {
            continue;
        }

        // Skip absolute positioned elements
        if (child->Is_Absolute()) {
            std::cout << get_indent() << "Skipping absolute child in grow calculation" << std::endl;
            // Still need to recursively grow them since they might have growable children
            child->Grow();
            continue;
        }

        visible_count++;
        std::cout << get_indent() << "Child " << child->id << " size=(" << child->size.x << ", "
                  << child->size.y << ")" << std::endl;

        if (layout_model == Layout_Model::Horizontal) {
            if (child->size.x == Size::Grow()) {
                growable.push_back(child);
                child->size.x = min_size.x;
            }
            remaining_width -= child->size.x;
        } else {
            if (child->size.y == Size::Grow()) {
                growable.push_back(child);
                child->size.y = min_size.y;
            }
            remaining_height -= child->size.y;
        }
    }

    std::cout << get_indent() << "Growable children: " << growable.size() << " | Remaining space: ("
              << remaining_width << ", " << remaining_height << ")" << std::endl;

    if (growable.size() == 0) {
        std::cout << get_indent() << "No growable children, skipping distribution" << std::endl;
        for (EUI_Element* child : children) {
            // Skip absolute since we already handled them
            if (child->is_visible && !child->Is_Absolute()) {
                child->Grow();
            }
        }
        layout_depth--;
        return;
    }

    float total_gap = visible_count > 1 ? (visible_count - 1) * gap : 0;
    if (layout_model == Layout_Model::Horizontal) {
        remaining_width -= total_gap;
        std::cout << get_indent() << "After gap: remaining_width=" << remaining_width << std::endl;
    } else {
        remaining_height -= total_gap;
        std::cout << get_indent() << "After gap: remaining_height=" << remaining_height
                  << std::endl;
    }

    // Grow children
    if (layout_model == Layout_Model::Horizontal) {
        while (remaining_width > 0) {
            float smallest = growable[0]->size.x;
            float second_smallest = INFINITY;
            float width_to_add = remaining_width;

            for (EUI_Element* child : growable) {
                if (child->size.x < smallest) {
                    second_smallest = smallest;
                    smallest = child->size.x;
                }
                if (child->size.x > smallest) {
                    second_smallest = std::min(second_smallest, child->size.x);
                    width_to_add = second_smallest - smallest;
                }
            }

            width_to_add = std::min(width_to_add, remaining_width / growable.size());

            for (EUI_Element* child : growable) {
                if (child->size.x == smallest) {
                    child->size.x += width_to_add;
                    remaining_width -= width_to_add;
                }
            }
        }
    } else {
        while (remaining_height > 0) {
            float smallest = growable[0]->size.y;
            float second_smallest = INFINITY;
            float height_to_add = remaining_height;

            for (EUI_Element* child : growable) {
                if (child->size.y < smallest) {
                    second_smallest = smallest;
                    smallest = child->size.y;
                }
                if (child->size.y > smallest) {
                    second_smallest = std::min(second_smallest, child->size.y);
                    height_to_add = second_smallest - smallest;
                }
            }

            height_to_add = std::min(height_to_add, remaining_height / growable.size());

            for (EUI_Element* child : growable) {
                if (child->size.y == smallest) {
                    child->size.y += height_to_add;
                    remaining_height -= height_to_add;
                }
            }
        }
    }

    for (auto* child : children) {
        if (child->is_visible && !child->Is_Absolute()) {
            // cross-axis grow
            if (child->size.y == Size::Grow() && layout_model == Layout_Model::Horizontal) {
                child->size.y = remaining_height;
            }
            if (child->size.x == Size::Grow() && layout_model == Layout_Model::Vertical) {
                child->size.x = remaining_width;
            }
            // propagate grow to children
            child->Grow();
        }
    }

    layout_depth--;
    std::cout << get_indent() << "  → final size=(" << size.x << ", " << size.y << ")" << std::endl;
}

void EUI_Box::Place() {
    std::cout << get_indent() << "[PLACE] Box ("
              << (layout_model == Layout_Model::Horizontal ? "HBox" : "VBox") << ") id='" << id
              << "' at pos=(" << pos.x << ", " << pos.y << ") size=(" << size.x << ", " << size.y
              << ")" << std::endl;
    layout_depth++;

    // Calculate total size of children for main-axis alignment (only static/relative)
    float total_children_size = 0;
    int visible_count = 0;
    for (EUI_Element* child : children) {
        if (child->is_visible && !child->Is_Absolute()) {
            visible_count++;
            if (layout_model == Layout_Model::Horizontal) {
                total_children_size += child->size.x;
            } else {
                total_children_size += child->size.y;
            }
        }
    }

    float total_gap = visible_count > 1 ? gap * (visible_count - 1) : 0;
    total_children_size += total_gap;

    // Calculate initial offset based on main-axis alignment
    float offset;
    switch (main_axis_alignment) {
        case Alignment::Start:
        case Alignment::Stretch:
            offset = layout_model == Layout_Model::Horizontal ? padding.left : padding.top;
            break;
        case Alignment::Center:
            offset = layout_model == Layout_Model::Horizontal
                         ? (size.x - total_children_size) / 2.0f
                         : (size.y - total_children_size) / 2.0f;
            break;
        case Alignment::End:
            offset = layout_model == Layout_Model::Horizontal
                         ? size.x - total_children_size - padding.right
                         : size.y - total_children_size - padding.bottom;
            break;
    }

    std::cout << get_indent() << "Main-axis alignment offset: " << offset
              << " (total_children_size=" << total_children_size << ")" << std::endl;

    // Place non-absolute children with normal flow
    for (EUI_Element* child : children) {
        if (!child->is_visible || child->Is_Absolute()) {
            continue;
        }

        if (layout_model == Layout_Model::Horizontal) {
            // place child at offset cursor
            child->pos.x = pos.x + offset;

            // handle stretch alignment offset calculation
            if (main_axis_alignment == Alignment::Stretch && visible_count > 1) {
                float stretch_gap = (size.x - total_children_size - padding.left - padding.right) /
                                    (visible_count - 1);
                offset += stretch_gap;
            }

            offset += child->size.x + gap;

            // cross (Y) axis
            switch (cross_axis_alignment) {
                case Alignment::Start:
                case Alignment::Stretch:
                    child->pos.y = pos.y + padding.top;
                    break;
                case Alignment::Center:
                    child->pos.y = pos.y + (size.y - child->size.y) / 2.0f;
                    break;
                case Alignment::End:
                    child->pos.y = pos.y + size.y - child->size.y - padding.bottom;
                    break;
            }
        } else {
            // place child at offset cursor
            child->pos.y = pos.y + offset;

            // handle stretch alignment offset calculation
            if (main_axis_alignment == Alignment::Stretch && visible_count > 1) {
                float stretch_gap = (size.y - total_children_size - padding.top - padding.bottom) /
                                    (visible_count - 1);
                offset += stretch_gap;
            }

            offset += child->size.y + gap;

            // cross (X) axis
            switch (cross_axis_alignment) {
                case Alignment::Start:
                case Alignment::Stretch:
                    child->pos.x = pos.x + padding.left;
                    break;
                case Alignment::Center:
                    child->pos.x = pos.x + (size.x - child->size.x) / 2.0f;
                    break;
                case Alignment::End:
                    child->pos.x = pos.x + size.x - child->size.x - padding.right;
                    break;
            }
        }

        std::cout << get_indent() << "Placed child at (" << child->pos.x << ", " << child->pos.y
                  << ") with size (" << child->size.x << ", " << child->size.y << ")" << std::endl;

        // propagate placement to children after placement
        child->Place();
    }

    // Handle absolute positioned children
    for (EUI_Element* child : children) {
        if (child->is_visible && child->Is_Absolute()) {
            std::cout << get_indent() << "Placing absolute child at user-defined pos=("
                      << child->pos.x << ", " << child->pos.y << ")" << std::endl;
            // User has already set child->pos, just propagate to children
            child->Place();
        }
    }

    // Apply relative positioning offset after normal placement
    if (Is_Relative()) {
        pos.x += left - right;
        pos.y += top - bottom;
        std::cout << get_indent() << "Applied relative offset: left=" << left << " right=" << right
                  << " top=" << top << " bottom=" << bottom << " → final pos=(" << pos.x << ", "
                  << pos.y << ")" << std::endl;
    }

    layout_depth--;
}

void EUI_Box::Render() {
    if (!is_visible)
        return;

    if (background_color.has_value())
        DrawRectangleRec({pos.x, pos.y, size.x, size.y}, background_color.value());

    if (border_radius > 0)
        DrawRectangleLinesEx({pos.x, pos.y, size.x, size.y}, border_radius, border_color);

    // Render non-absolute children first
    for (EUI_Element* child : children) {
        if (child->is_visible && !child->Is_Absolute()) {
            child->Render();
        }
    }

    // Render absolute positioned children last (on top)
    // TODO: potentially add z-indexing
    for (EUI_Element* child : children) {
        if (child->is_visible && child->Is_Absolute()) {
            child->Render();
        }
    }
}

void EUI_Box::Delete() {
    for (EUI_Element* child : children) {
        child->Delete();
    }
}
