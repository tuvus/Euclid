#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

#include "ui/eui.h"

// Layout debugging helpers
thread_local int layout_depth = 0;  // Shared across translation units
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
              << (layout_model == Layout_Model::Horizontal ? "HBox" : "VBox")
              << ") id='" << id << "'" << std::endl;
    layout_depth++;

    // FIT SIZING
    float children_main_size = 0;
    float max_child_cross_size = 0;
    float total_gap = children.size() > 1 ? gap * (children.size() - 1) : 0;

    // Size children first (size bottom up)
    for (EUI_Element* child : children) {
        if (child->is_visible) {
            child->Size();
            if (layout_model == Layout_Model::Horizontal) {
                children_main_size += child->size.x;
                max_child_cross_size = std::max(max_child_cross_size, child->size.y);
            } else {
                children_main_size += child->size.y;
                max_child_cross_size = std::max(max_child_cross_size, child->size.x);
            }
        }
    }

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
    std::cout << get_indent() << "  → size=(" << size.x << ", " << size.y
              << ") min=(" << min_size.x << ", " << min_size.y
              << ") padding=(" << padding.top << "," << padding.right
              << "," << padding.bottom << "," << padding.left << ") gap=" << gap << std::endl;
}

void EUI_Box::Grow() {
    std::cout << get_indent() << "[GROW] Box ("
              << (layout_model == Layout_Model::Horizontal ? "HBox" : "VBox")
              << ") id='" << id << "' before=(" << size.x << ", " << size.y << ")" << std::endl;
    layout_depth++;

    if (parent == nullptr) {
        std::cout << get_indent() << "Root element, no grow calculation" << std::endl;
        for (EUI_Element* child : children) {
            if (child->is_visible) {
                child->Grow();
            }
        }
        layout_depth--;
        return;
    }

    // FIRST: Resolve this box's own Grow() sentinels (so we have concrete size to work with)
    if (size.x == Size::Grow()) {
        size.x = parent->size.x - parent->padding.left - parent->padding.right;
        std::cout << get_indent() << "Resolved own X growth to " << size.x << std::endl;
    }
    if (size.y == Size::Grow()) {
        size.y = parent->size.y - parent->padding.top - parent->padding.bottom;
        std::cout << get_indent() << "Resolved own Y growth to " << size.y << std::endl;
    }

    // CALCULATE REMAINING SPACE within THIS box to distribute to children
    float remaining_width = size.x - padding.left - padding.right;
    float remaining_height = size.y - padding.top - padding.bottom;
    std::vector<EUI_Element*> growable;
    for (EUI_Element* child : children) {
        if (!child->is_visible) {
            continue;
        }

        if (layout_model == Layout_Model::Horizontal) {
            if (child->size.x == Size::Grow()) {
                growable.push_back(child);
            } else {
                // Only subtract concrete sizes, not Grow() sentinel
                remaining_width -= child->size.x;
            }
        } else {
            if (child->size.y == Size::Grow()) {
                growable.push_back(child);
            } else {
                // Only subtract concrete sizes, not Grow() sentinel
                remaining_height -= child->size.y;
            }
        }
    }

    std::cout << get_indent() << "Growable children: " << growable.size()
              << " | Remaining space: (" << remaining_width << ", " << remaining_height << ")" << std::endl;

    if (growable.size() == 0) {
        std::cout << get_indent() << "No growable children, skipping distribution" << std::endl;
        layout_depth--;
        return;
    }

    // Initialize growable children to their min_size (replacing Grow() sentinel)
    for (EUI_Element* child : growable) {
        if (layout_model == Layout_Model::Horizontal) {
            if (child->size.x == Size::Grow()) {
                child->size.x = child->min_size.x;
                remaining_width -= child->min_size.x;
            }
        } else {
            if (child->size.y == Size::Grow()) {
                child->size.y = child->min_size.y;
                remaining_height -= child->min_size.y;
            }
        }
    }

    float total_gap = children.size() < 2 ? 0 : (children.size() - 1) * gap;
    if (layout_model == Layout_Model::Horizontal) {
        remaining_width -= total_gap;
        std::cout << get_indent() << "After gap: remaining_width=" << remaining_width << std::endl;
    } else {
        remaining_height -= total_gap;
        std::cout << get_indent() << "After gap: remaining_height=" << remaining_height << std::endl;
    }

    // GROW CHILDREN
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
        if (child->is_visible) {
            child->Grow();
        }
    }

    layout_depth--;
    std::cout << get_indent() << "  → final size=(" << size.x << ", " << size.y << ")" << std::endl;
}

void EUI_Box::Place() {
    std::cout << get_indent() << "[PLACE] Box ("
              << (layout_model == Layout_Model::Horizontal ? "HBox" : "VBox")
              << ") id='" << id << "' at pos=(" << pos.x << ", " << pos.y
              << ") size=(" << size.x << ", " << size.y << ")" << std::endl;
    layout_depth++;

    // Calculate total size of children for main-axis alignment
    float total_children_size = 0;
    int visible_count = 0;
    for (EUI_Element* child : children) {
        if (child->is_visible) {
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
    if (layout_model == Layout_Model::Horizontal) {
        switch (horizontal_alignment) {
            case Alignment::Start:
            case Alignment::Stretch:
                offset = padding.left;
                break;
            case Alignment::Center:
                offset = (size.x - total_children_size) / 2.0f;
                break;
            case Alignment::End:
                offset = size.x - total_children_size - padding.right;
                break;
        }
    } else {
        switch (vertical_alignment) {
            case Alignment::Start:
            case Alignment::Stretch:
                offset = padding.top;
                break;
            case Alignment::Center:
                offset = (size.y - total_children_size) / 2.0f;
                break;
            case Alignment::End:
                offset = size.y - total_children_size - padding.bottom;
                break;
        }
    }

    std::cout << get_indent() << "Main-axis alignment offset: " << offset
              << " (total_children_size=" << total_children_size << ")" << std::endl;

    // place each children with correct gap and padding
    for (EUI_Element* child : children) {
        if (child->is_visible) {
            // Set child position FIRST (so nested children can use correct parent pos)
            if (layout_model == Layout_Model::Horizontal) {
                // Main axis (X): use offset cursor
                child->pos.x = pos.x + offset;
                offset += child->size.x + gap;

                // Cross axis (Y): apply parent's vertical alignment to position children
                switch (vertical_alignment) {
                    case Alignment::Start:
                        child->pos.y = pos.y + padding.top;
                        break;
                    case Alignment::Center:
                        child->pos.y = pos.y + (size.y - child->size.y) / 2.0f;
                        break;
                    case Alignment::End:
                        child->pos.y = pos.y + size.y - child->size.y - padding.bottom;
                        break;
                    case Alignment::Stretch:
                        // Position at start; stretching should be done in Grow()
                        child->pos.y = pos.y + padding.top;
                        break;
                }
            } else {
                // Main axis (Y): use offset cursor
                child->pos.y = pos.y + offset;
                offset += child->size.y + gap;

                // Cross axis (X): apply parent's horizontal alignment to position children
                switch (horizontal_alignment) {
                    case Alignment::Start:
                        child->pos.x = pos.x + padding.left;
                        break;
                    case Alignment::Center:
                        child->pos.x = pos.x + (size.x - child->size.x) / 2.0f;
                        break;
                    case Alignment::End:
                        child->pos.x = pos.x + size.x - child->size.x - padding.right;
                        break;
                    case Alignment::Stretch:
                        // Position at start; stretching should be done in Grow()
                        child->pos.x = pos.x + padding.left;
                        break;
                }
            }

            std::cout << get_indent() << "Placed child at (" << child->pos.x << ", " << child->pos.y
                      << ") with size (" << child->size.x << ", " << child->size.y << ")" << std::endl;

            // THEN propagate placement to children
            child->Place();
        }
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

    // Render children
    for (EUI_Element* child : children) {
        if (child->is_visible)
            child->Render();
    }
}

void EUI_Box::Delete() {
    for (EUI_Element* child : children) {
        child->Delete();
    }
}
