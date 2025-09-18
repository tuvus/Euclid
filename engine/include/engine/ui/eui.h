#pragma once

/*
 * EUI is a simple UI library for creating UI in a html/css-like way.
 */

#include <functional>
#include <optional>
#include <raylib.h>
#include <string>
#include <vector>

class Application;
// TOP, RIGHT, BOTTOM, LEFT
typedef struct Sides {
  public:
    float top, right, bottom, left;
} Sides;

enum class Layout_Model { Horizontal, Vertical };

enum class Alignment { Start, Center, End, Stretch };

enum class Position { Static, Relative, Absolute, Fixed, Sticky };

class EUI_Input;
class EUI_Style;
class EUI_Element;
class EUI_Context;

class EUI_Input {
  public:
    Vector2 mouse_position;
    bool left_mouse_pressed;
    bool right_mouse_pressed;
    bool left_mouse_down;
    bool right_mouse_down;
    int key_pressed;
};

class EUI_Style {
  public:
    // Optional styles are inherited if not explicitly set
    std::optional<Color> background_color;
    std::optional<Color> text_color;

    // Units are all px
    Sides margin, padding = {0};

    Position position = Position::Static;
    // For relative positioning, the top/right/bottom/left values are relative to the parent
    float top, right, bottom, left = 0;

    Color border_color = BLACK;
    float border_radius = 0;

    std::optional<Font> font;
    std::optional<float> font_size;
    std::optional<float> font_spacing;

    Alignment horizontal_alignment = Alignment::Start;
    Alignment vertical_alignment = Alignment::Start;
    Alignment text_horizontal_alignment = Alignment::Start;
    Alignment text_vertical_alignment = Alignment::Start;
};

class EUI_Context {
  private:
    EUI_Element* root = nullptr;

  public:
    EUI_Context() {};
    EUI_Context(EUI_Style default_style);
    ~EUI_Context();

    EUI_Input input = {};

    EUI_Element* hovered = nullptr;
    EUI_Element* active = nullptr;
    EUI_Element* focused = nullptr;

    EUI_Style default_style = {
        .text_color = BLACK,

        .margin = {0},
        .padding = {0},

        .border_color = BLACK,
        .border_radius = 0,

        .font = GetFontDefault(),
        .font_size = 15,
        .font_spacing = 1,

        .horizontal_alignment = Alignment::Start,
        .vertical_alignment = Alignment::Start,
        .text_horizontal_alignment = Alignment::Start,
        .text_vertical_alignment = Alignment::Start,
    };

    // TODO: do something with this
    float global_scale = 1.0f;
    float dpi_factor = 1.0f;

    EUI_Element* Get_Root() const { return root; }
    void Set_Root(EUI_Element* new_root);

    void Begin_Frame();
    void End_Frame();
    void Update_Input();
    void Perform_Layout();
    void Handle_Input();
    void Render();
};

class EUI_Element {
  public:
    virtual ~EUI_Element() {}

    std::string id;

    EUI_Context* ctx = nullptr;
    EUI_Element* parent = nullptr;

    Vector2 pos = {0}, dim = {0};
    Vector2 min_size, max_size, preferred_size;

    bool is_visible = true;
    bool is_hovered = false;
    bool is_focused = false;
    bool is_active = false;
    bool is_deleted = false;

    EUI_Style style;

    // Getters for inheritable properties (optionals)
    Color Get_Text_Color() const;
    Font Get_Font() const;
    float Get_Font_Size() const;
    float Get_Font_Spacing() const;
    EUI_Style Get_Effective_Style() const;

    virtual void Set_Context(EUI_Context& ctx);
    virtual void Delete() { is_deleted = true; }

    virtual void Layout() = 0;
    virtual void Handle_Input() = 0;
    virtual void Render() = 0;

    virtual bool Is_Container() const { return false; };
};

class EUI_Container : public EUI_Element {
  protected:
    EUI_Container(Layout_Model lm) : layout_model(lm) {}
    std::vector<EUI_Element*> children;

  public:
    ~EUI_Container() override;

    Layout_Model layout_model;

    float gap = 0;

    std::vector<EUI_Element*>& Get_Children();
    void Add_Child(EUI_Element* child);

    void Set_Context(EUI_Context& ctx) override;
    void Delete() override;

    virtual void Layout() override = 0;
    void Handle_Input() override;
    void Render() override;

    bool Is_Container() const override { return true; }
};

class EUI_VBox : public EUI_Container {
  public:
    EUI_VBox() : EUI_Container(Layout_Model::Vertical) {}

    void Layout() override;
};

class EUI_HBox : public EUI_Container {
  public:
    EUI_HBox() : EUI_Container(Layout_Model::Horizontal) {}

    void Layout() override;
};

class EUI_Text : public EUI_Element {
  protected:
    std::string text;

  public:
    EUI_Text(const std::string& text);

    Vector2 text_pos;

    void Layout() override;
    void Handle_Input() override;
    void Render() override;

    std::string& Get_Text();
    void Set_Text(const std::string& text);
};

class EUI_Button : public EUI_Text {
  public:
    bool is_enabled;

    EUI_Button(const std::string& text, std::function<void()> on_click);

    std::function<void()> on_click;

    void Handle_Input() override;
};
