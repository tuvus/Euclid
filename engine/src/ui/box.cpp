#include <algorithm>
#include <cmath>

#include "ui/eui.h"

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
                children_main_size = std::max(children_main_size, child->size.x);
                max_child_cross_size += child->size.y;
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
}

void EUI_Box::Grow() {
    if (parent == nullptr) {
        for (EUI_Element* child : children) {
            if (child->is_visible) {
                child->Grow();
            }
        }
        return;
    }

    // CALCULATE REMAINING SPACE
    float remaining_width = parent->size.x - parent->padding.left - parent->padding.right;
    float remaining_height = parent->size.y - parent->padding.top - parent->padding.bottom;
    std::vector<EUI_Element*> growable;
    for (EUI_Element* child : children) {
        if (!child->is_visible) {
            return;
        }

        if (layout_model == Layout_Model::Horizontal) {
            if (child->size.x == Size::Grow()) {
                growable.push_back(child);
            }
            remaining_width -= child->size.x;
        } else {
            if (child->size.y == Size::Grow()) {
                growable.push_back(child);
            }
            remaining_height -= child->size.y;
        }
    }

    if (growable.size() == 0) {
        return;
    }

    float gap = children.size() < 2 ? 0 : (children.size() - 1) * gap;
    if (layout_model == Layout_Model::Horizontal) {
        remaining_width -= gap;
    } else {
        remaining_height -= gap;
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

        if (size.y == Size::Grow()) {
            size.y = parent->size.y - parent->padding.top - parent->padding.bottom;
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

        if (size.x == Size::Grow()) {
            size.x = parent->size.x - parent->padding.left - parent->padding.right;
        }
    }

    for (auto* child : children) {
        if (child->is_visible) {
            child->Grow();
        }
    }
}

void EUI_Box::Place() {
    // acts as a "cursor" while laying out children
    float offset = layout_model == Layout_Model::Horizontal ? padding.left : padding.top;

    // place each children with correct gap and padding
    for (EUI_Element* child : children) {
        if (child->is_visible) {
            child->Place();
            if (layout_model == Layout_Model::Horizontal) {
                child->pos.x = pos.x + offset;
                child->pos.y = pos.y;
                offset += child->size.x + gap;
            } else {
                child->pos.x = pos.x;
                child->pos.y = pos.y + offset;
                offset += child->size.y + gap;
            }
        }
    }
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
