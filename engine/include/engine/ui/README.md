# EUI (Euclid UI)

**EUI** is a UI system built for the Euclid game engine. It provides a flexbox-like layout system with support for nested containers, alignment controls, and flexible positioning.

---

## Table of Contents

1. [Core Concepts](#core-concepts)
2. [Quick Start](#quick-start)
3. [Layout System](#layout-system)
4. [Sizing](#sizing)
5. [Alignment](#alignment)
6. [Positioning](#positioning)
7. [Styling](#styling)
8. [API Reference](#api-reference)
9. [Examples](#examples)

---

## Core Concepts

### Elements

All UI elements inherit from `EUI_Element`. The main element types are:

- **`EUI_Box`** - Container for other elements (flexbox-like)
- **`EUI_Text`** - Text display element
- **`EUI_Button`** - Interactive button (extends `EUI_Text`)

### Context

`EUI_Context` manages the UI system, handling input, layout, and rendering:

```cpp
EUI_Context* ctx = new EUI_Context();
ctx->Set_Root(root_element);

// Per-frame
ctx->Begin_Frame();
ctx->Update_Input();
ctx->Perform_Layout();
ctx->Handle_Input();
ctx->Render();
ctx->End_Frame();
```

---

## Quick Start

### Creating a Simple UI

```cpp
// Create root container
EUI_Box* root = new EUI_Box(true); // true = vertical layout
root->size = {800, 600};
root->padding = {20, 20, 20, 20};
root->gap = 10;

// Add a text element
EUI_Text* title = new EUI_Text("Hello, World!");
title->text_color = WHITE;
title->font_size = 24;
root->Add_Child(title);

// Add a button
EUI_Button* btn = new EUI_Button("Click Me", []() {
    std::cout << "Button clicked!" << std::endl;
});
btn->background_color = BLUE;
root->Add_Child(btn);

// Set up context
ctx->Set_Root(root);
```

---

## Layout System

EUI uses a **3-phase layout algorithm**:

### 1. Size Phase (Bottom-Up)
Elements calculate their minimum required size based on content and children's sizes.

### 2. Grow Phase (Top-Down)
Elements with `Size::Grow()` expand to fill available space.

### 3. Place Phase (Top-Down)
Elements are positioned based on alignment rules and parent position.

### Layout Models

**`EUI_Box`** supports two layout models:

```cpp
// Horizontal (default) - children placed left-to-right
EUI_Box* hbox = new EUI_Box();
// or
EUI_Box* hbox = new EUI_Box(false);

// Vertical - children placed top-to-bottom
EUI_Box* vbox = new EUI_Box(true);
```

---

## Sizing

Elements support three sizing modes:

### Fixed Size
```cpp
element->size = {300, 200}; // 300px wide, 200px tall
```

### Fit Content
```cpp
element->size = {Size::Fit(), Size::Fit()}; // Size to content
```

### Grow to Fill
```cpp
element->size = {Size::Grow(), Size::Grow()}; // Fill available space
```

### Mixed Sizing
```cpp
element->size = {Size::Grow(), 100}; // Grow width, fixed height
```

---

## Alignment

Alignment is specified relative to the layout direction:

- **Main axis** = layout direction (X for HBox, Y for VBox)
- **Cross axis** = perpendicular to layout direction

### Main Axis Alignment

Controls how children are positioned along the layout direction:

```cpp
box->main_axis_alignment = Alignment::Start;   // Left/Top (default)
box->main_axis_alignment = Alignment::Center;  // Center
box->main_axis_alignment = Alignment::End;     // Right/Bottom
box->main_axis_alignment = Alignment::Stretch; // Evenly distributed
```

### Cross Axis Alignment

Controls how children are positioned perpendicular to the layout direction:

```cpp
box->cross_axis_alignment = Alignment::Start;   // Top/Left (default)
box->cross_axis_alignment = Alignment::Center;  // Center
box->cross_axis_alignment = Alignment::End;     // Bottom/Right
box->cross_axis_alignment = Alignment::Stretch; // Fill cross axis
```

### Text Alignment

Text elements use the same alignment properties to align the text within the element.

```cpp
text->main_axis_alignment = Alignment::Center; // Vertical alignment
text->cross_axis_alignment = Alignment::Center; // Horizontal alignment
```

---

## Positioning

EUI supports three positioning modes:

### Static (Default)

Element participates in normal layout flow.

```cpp
element->position = Position::Static; // Default
```

### Relative

Element is placed in normal flow, then **offset** by `left`/`right`/`top`/`bottom` values. The original space in the layout is **preserved**.

```cpp
element->Set_Relative_Offset(20, 10); // +20px right, +10px down

// Or manually:
element->position = Position::Relative;
element->left = 20;  // Moves right
element->top = 10;   // Moves down
```

**Offset semantics:**
- `left`: positive values move **RIGHT**
- `right`: positive values move **LEFT**
- `top`: positive values move **DOWN**
- `bottom`: positive values move **UP**

### Absolute

Element is **removed from normal flow** and positioned at exact pixel coordinates. Useful for overlays, tooltips, HUDs, and floating elements.

```cpp
element->Set_Absolute_Position(100, 50); // Pixel coordinates

// Or manually:
element->position = Position::Absolute;
element->pos.x = 100;
element->pos.y = 50;
```

**Key behaviors:**
- Does not affect parent sizing or sibling placement
- Renders **on top** of static/relative siblings (z-ordering)
- Still processes children normally (if it's a container)
- User manually sets `pos.x` and `pos.y`

### Position Query Helpers

```cpp
if (element->Is_Static())   { /* ... */ }
if (element->Is_Relative()) { /* ... */ }
if (element->Is_Absolute()) { /* ... */ }
if (element->Is_Positioned()) { /* Any positioned (not static) */ }
```

---

## Styling

### Colors

```cpp
element->background_color = BLUE;
element->text_color = WHITE;
element->border_color = BLACK;
```

### Padding

```cpp
// Sides: {top, right, bottom, left}
element->padding = {10, 20, 10, 20};

// Uniform padding
element->padding = {10, 10, 10, 10};
```

### Border

```cpp
element->border_width = 2;
element->border_color = BLACK;
```

### Font

```cpp
element->font_size = 18;
element->font_spacing = 1.5;
// element->font = custom_font; // Optional custom font
```

### Gap (Spacing Between Children)

```cpp
box->gap = 15; // 15px spacing between children
```

---

## API Reference

### EUI_Context

| Method | Description |
|--------|-------------|
| `Set_Root(element)` | Set the root UI element |
| `Begin_Frame()` | Start a new frame |
| `Update_Input()` | Poll input from raylib |
| `Perform_Layout()` | Run the 3-phase layout algorithm |
| `Handle_Input()` | Process input events |
| `Render()` | Draw the UI |
| `End_Frame()` | Finish the frame |

### EUI_Element

| Property | Type | Description |
|----------|------|-------------|
| `id` | `string` | Element identifier |
| `pos` | `Vector2` | Position in pixels |
| `size` | `Vector2` | Size in pixels or `Size::Fit()`/`Size::Grow()` |
| `is_visible` | `bool` | Visibility flag |
| `padding` | `Sides` | Internal padding |
| `position` | `Position` | Positioning mode (Static/Relative/Absolute) |
| `top, right, bottom, left` | `float` | Relative positioning offsets |
| `main_axis_alignment` | `Alignment` | Main axis alignment |
| `cross_axis_alignment` | `Alignment` | Cross axis alignment |

| Method | Description |
|--------|-------------|
| `Set_Relative_Offset(x, y)` | Set relative positioning offset |
| `Set_Absolute_Position(x, y)` | Set absolute positioning coordinates |
| `Is_Static()` | Check if position is static |
| `Is_Relative()` | Check if position is relative |
| `Is_Absolute()` | Check if position is absolute |
| `Is_Positioned()` | Check if position is not static |

### EUI_Box

| Property | Type | Description |
|----------|------|-------------|
| `layout_model` | `Layout_Model` | Horizontal or Vertical |
| `gap` | `float` | Spacing between children |

| Method | Description |
|--------|-------------|
| `Add_Child(element)` | Add a child element |
| `Get_Children()` | Get vector of children |

### EUI_Text

| Property | Type | Description |
|----------|------|-------------|
| `text` | `string` | Text content |

| Method | Description |
|--------|-------------|
| `Get_Text()` | Get text content |
| `Set_Text(text)` | Update text content |

### EUI_Button

| Property | Type | Description |
|----------|------|-------------|
| `is_enabled` | `bool` | Whether button is interactive |
| `on_click` | `function<void()>` | Click callback |

---

## Examples

### Example 1: Centered Button

```cpp
EUI_Box* root = new EUI_Box(true);
root->size = {800, 600};
root->main_axis_alignment = Alignment::Center;
root->cross_axis_alignment = Alignment::Center;

EUI_Button* btn = new EUI_Button("Play", []() {
    std::cout << "Starting game..." << std::endl;
});
btn->background_color = GREEN;
btn->padding = {20, 40, 20, 40};
btn->font_size = 24;

root->Add_Child(btn);
```

### Example 2: Header/Content/Footer Layout

```cpp
EUI_Box* root = new EUI_Box(true); // Vertical
root->size = {SCREEN_WIDTH, SCREEN_HEIGHT};

// Header (fixed height)
EUI_Box* header = new EUI_Box();
header->size = {Size::Grow(), 60};
header->background_color = DARKBLUE;
header->main_axis_alignment = Alignment::Center;
EUI_Text* title = new EUI_Text("My Game");
header->Add_Child(title);

// Content (grows to fill)
EUI_Box* content = new EUI_Box();
content->size = {Size::Grow(), Size::Grow()};
content->background_color = GRAY;

// Footer (fixed height)
EUI_Box* footer = new EUI_Box();
footer->size = {Size::Grow(), 40};
footer->background_color = DARKGRAY;
footer->main_axis_alignment = Alignment::End;

root->Add_Child(header);
root->Add_Child(content);
root->Add_Child(footer);
```

### Example 3: Sidebar Layout

```cpp
EUI_Box* root = new EUI_Box(); // Horizontal
root->size = {SCREEN_WIDTH, SCREEN_HEIGHT};

// Sidebar (fixed width)
EUI_Box* sidebar = new EUI_Box(true);
sidebar->size = {200, Size::Grow()};
sidebar->background_color = DARKPURPLE;
sidebar->padding = {10, 10, 10, 10};

// Main content (grows to fill)
EUI_Box* main = new EUI_Box(true);
main->size = {Size::Grow(), Size::Grow()};
main->background_color = LIGHTGRAY;
main->padding = {20, 20, 20, 20};

root->Add_Child(sidebar);
root->Add_Child(main);
```

### Example 4: Relative Positioning

```cpp
EUI_Box* container = new EUI_Box(true);
container->gap = 10;

// Normal box
EUI_Box* box1 = new EUI_Box();
box1->size = {300, 80};
box1->background_color = BLUE;

// Offset box (space preserved)
EUI_Box* box2 = new EUI_Box();
box2->size = {300, 80};
box2->background_color = RED;
box2->Set_Relative_Offset(30, 5); // Nudge right and down

// Following box (shows preserved space)
EUI_Box* box3 = new EUI_Box();
box3->size = {300, 80};
box3->background_color = GREEN;

container->Add_Child(box1);
container->Add_Child(box2);
container->Add_Child(box3);
```

### Example 5: Absolute Positioning (Overlays)

```cpp
EUI_Box* root = new EUI_Box(true);
root->size = {SCREEN_WIDTH, SCREEN_HEIGHT};

// Normal content
EUI_Text* content = new EUI_Text("Main Content");
content->size = {Size::Grow(), Size::Grow()};
content->background_color = LIGHTGRAY;
root->Add_Child(content);

// Floating overlay (top-right corner)
EUI_Box* overlay = new EUI_Box();
overlay->Set_Absolute_Position(SCREEN_WIDTH - 220, 20);
overlay->size = {200, 100};
overlay->background_color = Color{100, 0, 200, 200}; // Semi-transparent
overlay->padding = {10, 10, 10, 10};
EUI_Text* label = new EUI_Text("Overlay");
label->text_color = WHITE;
overlay->Add_Child(label);
root->Add_Child(overlay); // Renders on top

// Floating Action Button (bottom-right)
EUI_Button* fab = new EUI_Button("+", []() {
    std::cout << "FAB clicked" << std::endl;
});
fab->Set_Absolute_Position(SCREEN_WIDTH - 80, SCREEN_HEIGHT - 80);
fab->size = {60, 60};
fab->background_color = RED;
fab->border_width = 2;
fab->main_axis_alignment = Alignment::Center;
fab->cross_axis_alignment = Alignment::Center;
root->Add_Child(fab); // Renders on top
```

---

## Testing

Run the built-in test suite to see example layouts demonstrating main features:

```bash
make run
```

Press `1-6` to switch between test scenes:

1. **Grow Sizing** - Demonstrates `Size::Grow()` behavior
2. **Center Alignment** - Main and cross-axis centering
3. **End Alignment** - Right/bottom alignment
4. **Mixed Alignments** - Complex nested alignments
5. **Complex Nested** - Full app layout (header/sidebar/content/footer)
6. **Positioning** - Static, Relative, and Absolute positioning

---

