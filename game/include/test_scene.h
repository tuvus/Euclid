#pragma once

#include "card_game.h"
#include "scene.h"

/// Scene for testing layout algorithms. Contains elements and nested layout with different
/// alignments. Change ACTIVE_TEST to switch between different test layouts.

class Test_Scene : public Scene {
  private:
    // Current active test (can be changed at runtime with 1-6 keys)
    int active_test = 6;

    // Pre-created test layouts (kept in memory to avoid create/destroy cycles)
    EUI_Element* test_layouts[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    // Helper to create a colored box with text
    EUI_Box* Create_Box(const std::string& label, Color color, Vector2 size,
                        Layout_Model model = Layout_Model::Horizontal) {
        EUI_Box* box = new EUI_Box(model == Layout_Model::Vertical);
        box->background_color = color;
        box->size = size;
        box->padding = {10, 10, 10, 10};

        if (!label.empty()) {
            EUI_Text* text = new EUI_Text(label);
            text->main_axis_alignment = Alignment::Center;
            text->cross_axis_alignment = Alignment::Center;
            text->text_color = WHITE;
            text->font_size = 16;
            box->Add_Child(text);
        }

        return box;
    }

    // Test 1: Basic grow sizing with padding
    EUI_Element* Test_Grow_Sizing() {
        EUI_Box* root = new EUI_Box();
        root->main_axis_alignment = Alignment::Center;
        root->cross_axis_alignment = Alignment::Center;
        root->gap = 20;

        EUI_Box* box1 = Create_Box("Fixed Size (600x300)", BLUE, {600, 300});
        EUI_Box* box2 = Create_Box("", RED, {500, 500}, Layout_Model::Vertical);
        box2->gap = 10;

        EUI_Box* grow_box = Create_Box("Grow Box (480x480)", GREEN, {Size::Grow(), Size::Grow()});
        grow_box->padding = {15, 15, 15, 15};

        EUI_Text* grow_text = new EUI_Text("Grow Text");
        grow_text->size = {Size::Grow(), Size::Grow()};
        grow_text->cross_axis_alignment = Alignment::Center;
        grow_text->text_color = YELLOW;
        grow_text->background_color = Color{0, 0, 0, 100};
        grow_box->Add_Child(grow_text);

        box2->Add_Child(grow_box);
        root->Add_Child(box1);
        root->Add_Child(box2);

        return root;
    }

    // Test 2: Center alignment on both axes
    EUI_Element* Test_Center_Alignment() {
        EUI_Box* root = new EUI_Box();
        root->main_axis_alignment = Alignment::Stretch;
        root->gap = 20;

        // Horizontal centering test
        EUI_Box* hbox = Create_Box("", DARKGRAY, {800, 200});
        hbox->main_axis_alignment = Alignment::Stretch; // Center children horizontally
        hbox->cross_axis_alignment = Alignment::Center; // Center children vertically
        hbox->gap = 10;

        hbox->Add_Child(Create_Box("Box 1", RED, {100, 100}));
        hbox->Add_Child(Create_Box("Box 2", GREEN, {100, 100}));
        hbox->Add_Child(Create_Box("Box 3", BLUE, {100, 100}));

        // Vertical centering test
        EUI_Box* vbox = Create_Box("", DARKGRAY, {200, 600}, Layout_Model::Vertical);
        vbox->cross_axis_alignment = Alignment::Center; // Center children horizontally
        vbox->main_axis_alignment = Alignment::Center;  // Center children vertically
        vbox->gap = 10;

        vbox->Add_Child(Create_Box("Box A", ORANGE, {100, 100}));
        vbox->Add_Child(Create_Box("Box B", PURPLE, {100, 100}));
        vbox->Add_Child(Create_Box("Box C", PINK, {100, 100}));

        root->Add_Child(hbox);
        root->Add_Child(vbox);

        return root;
    }

    // Test 3: End alignment
    EUI_Element* Test_End_Alignment() {
        EUI_Box* root = new EUI_Box(true); // Vertical
        root->cross_axis_alignment = Alignment::Center;
        root->main_axis_alignment = Alignment::Center;
        root->gap = 30;

        // Horizontal end alignment
        EUI_Box* hbox = Create_Box("", DARKGRAY, {800, 150});
        hbox->main_axis_alignment = Alignment::End; // Align children to end (right)
        hbox->gap = 10;

        hbox->Add_Child(Create_Box("Right 1", RED, {120, 80}));
        hbox->Add_Child(Create_Box("Right 2", GREEN, {120, 80}));

        // Vertical end alignment
        EUI_Box* vbox = Create_Box("", DARKGRAY, {200, 500}, Layout_Model::Vertical);
        vbox->main_axis_alignment = Alignment::End; // Align children to end (bottom)
        vbox->gap = 10;

        vbox->Add_Child(Create_Box("Bottom 1", ORANGE, {150, 80}));
        vbox->Add_Child(Create_Box("Bottom 2", PURPLE, {150, 80}));

        root->Add_Child(hbox);
        root->Add_Child(vbox);

        return root;
    }

    // Test 4: Mixed alignments with nesting
    EUI_Element* Test_Mixed_Alignments() {
        EUI_Box* root = new EUI_Box(true);
        root->main_axis_alignment = Alignment::Center;
        root->cross_axis_alignment = Alignment::Center;
        root->gap = 15;

        // Top row: Start, Center, End
        EUI_Box* row1 = Create_Box("", DARKGRAY, {900, 150});
        row1->gap = 20;

        EUI_Box* col_start =
            Create_Box("", BLUE, {Size::Grow(), Size::Fit()}, Layout_Model::Vertical);
        col_start->main_axis_alignment = Alignment::Start;
        col_start->gap = 5;
        col_start->Add_Child(Create_Box("Start", BLACK, {80, 30}));
        col_start->Add_Child(Create_Box("Aligned", BLACK, {80, 30}));

        EUI_Box* col_center = Create_Box("", GREEN, {150, 130}, Layout_Model::Vertical);
        col_center->main_axis_alignment = Alignment::Center;
        col_center->gap = 5;
        col_center->Add_Child(Create_Box("Center", BLACK, {80, 30}));
        col_center->Add_Child(Create_Box("Aligned", BLACK, {80, 30}));

        EUI_Box* col_end = Create_Box("", RED, {Size::Grow(), 130}, Layout_Model::Vertical);
        col_end->main_axis_alignment = Alignment::End;
        col_end->gap = 5;
        col_end->Add_Child(Create_Box("End", BLACK, {80, 30}));
        col_end->Add_Child(Create_Box("Aligned", BLACK, {80, 30}));

        row1->Add_Child(col_start);
        row1->Add_Child(col_center);
        row1->Add_Child(col_end);

        // Bottom row: Cross-axis alignment
        EUI_Box* row2 = Create_Box("", DARKGRAY, {900, 200});
        row2->size.y = Size::Grow();
        row2->gap = 15;

        EUI_Box* tall = Create_Box("Tall\nBox", ORANGE, {150, 150});
        tall->cross_axis_alignment = Alignment::Start;

        EUI_Box* medium = Create_Box("grow!", PURPLE, {Size::Grow(), Size::Grow()});
        medium->cross_axis_alignment = Alignment::Center;
        medium->main_axis_alignment = Alignment::Center;

        EUI_Box* small = Create_Box("Small", PINK, {150, 60});
        small->cross_axis_alignment = Alignment::End;

        row2->Add_Child(tall);
        row2->Add_Child(medium);
        row2->Add_Child(small);

        root->Add_Child(row1);
        root->Add_Child(row2);

        return root;
    }

    // Test 5: Complex nested layout with grow
    EUI_Element* Test_Complex_Nested() {
        EUI_Box* root = new EUI_Box(true);
        root->id = "root";
        root->size = {application.screen_width, application.screen_height};
        root->gap = 10;
        root->padding = {10, 10, 10, 10};

        // Header
        EUI_Box* header = Create_Box("Header (Centered)", DARKBLUE, {Size::Grow(), 60});
        header->id = "header";
        header->main_axis_alignment = Alignment::Center;

        // Content area with sidebar
        EUI_Box* content = Create_Box("", DARKGRAY, {Size::Grow(), Size::Grow()});
        content->id = "content";
        content->gap = 10;

        EUI_Box* sidebar = Create_Box("Sidebar\n(Fixed)", PURPLE, {200, Size::Grow()});
        sidebar->id = "sidebar";
        sidebar->cross_axis_alignment = Alignment::Start;

        EUI_Box* main_area =
            Create_Box("", GRAY, {Size::Grow(), Size::Grow()}, Layout_Model::Vertical);
        main_area->id = "main_area";
        main_area->gap = 10;

        main_area->Add_Child(Create_Box("Content 1 (Grow)", GREEN, {Size::Grow(), Size::Grow()}));
        main_area->Add_Child(Create_Box("Content 2 (Grow)", BLUE, {Size::Grow(), Size::Grow()}));

        content->Add_Child(sidebar);
        content->Add_Child(main_area);

        // Footer
        EUI_Box* footer = Create_Box("Footer (End Aligned)", DARKGREEN, {Size::Grow(), 50});
        footer->id = "footer";
        footer->main_axis_alignment = Alignment::End;

        root->Add_Child(header);
        root->Add_Child(content);
        root->Add_Child(footer);

        return root;
    }

    // Test 6: Positioning (Static, Relative, Absolute)
    EUI_Element* Test_Positioning() {
        EUI_Box* root = new EUI_Box(true);
        root->id = "root";
        root->size = {application.screen_width, application.screen_height};
        root->padding = {20, 20, 20, 20};
        root->gap = 20;
        root->main_axis_alignment = Alignment::Start;

        // Title
        EUI_Text* title = new EUI_Text("POSITIONING TEST (Static/Relative/Absolute)");
        title->text_color = WHITE;
        title->font_size = 20;
        title->background_color = DARKPURPLE;
        title->padding = {10, 10, 10, 10};
        root->Add_Child(title);

        // Container for examples
        EUI_Box* examples = new EUI_Box();
        examples->id = "examples";
        examples->size = {Size::Grow(), Size::Grow()};
        examples->gap = 30;
        examples->main_axis_alignment = Alignment::End;

        // === SECTION 1: Static positioning (normal flow) ===
        EUI_Box* static_section =
            Create_Box("", Color{40, 40, 40, 255}, {350, Size::Fit()}, Layout_Model::Vertical);
        static_section->id = "static_section";
        static_section->gap = 10;

        EUI_Text* static_label = new EUI_Text("Static (Normal Flow)");
        static_label->text_color = YELLOW;
        static_label->font_size = 18;
        static_section->Add_Child(static_label);

        EUI_Box* box1 = Create_Box("Box 1", BLUE, {300, 80});
        box1->position = Position::Static;
        EUI_Box* box2 = Create_Box("Box 2", GREEN, {300, 80});
        box2->position = Position::Static;
        EUI_Box* box3 = Create_Box("Box 3", RED, {300, 80});
        box3->position = Position::Static;

        static_section->Add_Child(box1);
        static_section->Add_Child(box2);
        static_section->Add_Child(box3);

        // === SECTION 2: Relative positioning ===
        EUI_Box* relative_section =
            Create_Box("", Color{40, 40, 40, 255}, {400, Size::Fit()}, Layout_Model::Vertical);
        relative_section->id = "relative_section";
        relative_section->gap = 10;

        EUI_Text* relative_label = new EUI_Text("Relative (Offset from normal)");
        relative_label->text_color = YELLOW;
        relative_label->font_size = 18;
        relative_section->Add_Child(relative_label);

        // Box A: Normal static
        EUI_Box* boxA = Create_Box("Static", DARKGRAY, {350, 60});
        relative_section->Add_Child(boxA);

        // Box B: Relative with right/down offset
        EUI_Box* boxB = Create_Box("Relative (+20 right, +10 down)", ORANGE, {350, 60});
        boxB->Set_Relative_Offset(20, 10); // Move 20px right, 10px down
        relative_section->Add_Child(boxB);

        // Box C: Shows space is preserved
        EUI_Box* boxC = Create_Box("Static (note space above)", DARKGRAY, {350, 60});
        relative_section->Add_Child(boxC);

        examples->Add_Child(static_section);
        examples->Add_Child(relative_section);

        root->Add_Child(examples);

        // === SECTION 3: Absolute positioning (overlays) ===
        // These are NOT added to the normal flow, so we add them to root directly

        // Overlay 1: Top-left corner
        EUI_Box* overlay1 = Create_Box("Absolute\nTop-Left", PURPLE, {150, 100});
        overlay1->Set_Absolute_Position(20, 80);
        overlay1->main_axis_alignment = Alignment::Center;
        overlay1->cross_axis_alignment = Alignment::Center;
        root->Add_Child(overlay1);

        // Overlay 2: Top-right corner
        EUI_Box* overlay2 = Create_Box("Absolute\nTop-Right", Color{200, 0, 200, 255}, {150, 100});
        overlay2->Set_Absolute_Position(application.screen_width - 170, 80);
        overlay2->main_axis_alignment = Alignment::Center;
        overlay2->cross_axis_alignment = Alignment::Center;
        root->Add_Child(overlay2);

        // Overlay 3: Bottom-center (floating action button style)
        EUI_Box* fab = Create_Box("FAB", Color{255, 100, 100, 255}, {100, 100});
        fab->Set_Absolute_Position((application.screen_width - 100) / 2,
                                   application.screen_height - 120);
        fab->border_width = 2;
        fab->main_axis_alignment = Alignment::Center;
        fab->cross_axis_alignment = Alignment::Center;
        root->Add_Child(fab);

        // Overlay 4: Nested absolute (absolute inside absolute)
        EUI_Box* overlay_parent = Create_Box("", Color{0, 100, 200, 200}, {300, 250});
        overlay_parent->Set_Absolute_Position(application.screen_width - 320,
                                              application.screen_height - 270);
        overlay_parent->padding = {20, 20, 20, 20};

        EUI_Text* parent_label = new EUI_Text("Parent: Absolute");
        parent_label->text_color = WHITE;
        parent_label->font_size = 14;
        overlay_parent->Add_Child(parent_label);

        // Child inside absolute parent - also absolute
        EUI_Box* overlay_child = Create_Box("Child\nAbsolute", YELLOW, {120, 80});
        overlay_child->Set_Absolute_Position(160, 150); // Pixel coordinates
        overlay_child->main_axis_alignment = Alignment::Center;
        overlay_child->cross_axis_alignment = Alignment::Center;
        overlay_child->text_color = BLACK;
        overlay_parent->Add_Child(overlay_child);

        root->Add_Child(overlay_parent);

        // Info text showing absolute elements render on top
        EUI_Text* info = new EUI_Text("Note: Purple boxes are absolute - they overlay everything!");
        info->text_color = Color{255, 255, 100, 255};
        info->font_size = 14;
        info->background_color = Color{0, 0, 0, 150};
        info->padding = {8, 8, 8, 8};
        info->Set_Absolute_Position(20, application.screen_height - 40);
        root->Add_Child(info);

        return root;
    }

    // Initialize all test layouts once
    void Initialize_All_Tests() {
        test_layouts[0] = Test_Grow_Sizing();
        test_layouts[1] = Test_Center_Alignment();
        test_layouts[2] = Test_End_Alignment();
        test_layouts[3] = Test_Mixed_Alignments();
        test_layouts[4] = Test_Complex_Nested();
        test_layouts[5] = Test_Positioning();

        Card_Game& game = static_cast<Card_Game&>(application);

        // Attach all to context
        for (int i = 0; i < 6; i++) {
            if (test_layouts[i] && game.eui_ctx) {
                test_layouts[i]->Set_Context(*game.eui_ctx);
            }
        }
    }

    // Switch to the test scene based on active_test
    void Load_Test() {
        Card_Game& game = static_cast<Card_Game&>(application);

        // Just switch which layout is active (1-indexed to 0-indexed)
        int index = active_test - 1;
        if (index >= 0 && index < 6) {
            root_elem = test_layouts[index];

            // Update size and context root, then perform layout
            if (root_elem) {
                root_elem->size = {(float) game.screen_width, (float) game.screen_height};
                game.eui_ctx->Set_Root(root_elem); // Tell context about the new root
                game.eui_ctx->Perform_Layout();
            }
        }
    }

  public:
    Test_Scene(Card_Game& card_game) : Scene(card_game) {
        root_elem = nullptr;
        Initialize_All_Tests(); // Pre-create all layouts
        Load_Test();            // Switch to initial test
    }

    ~Test_Scene() override {
        // Clear context's root to prevent it from trying to delete our layouts
        Card_Game& game = static_cast<Card_Game&>(application);
        game.eui_ctx->Set_Root(nullptr);

        // Set root_elem to nullptr so Scene destructor doesn't double-delete
        root_elem = nullptr;

        // Clean up all pre-created layouts
        for (int i = 0; i < 6; i++) {
            if (test_layouts[i]) {
                delete test_layouts[i];
                test_layouts[i] = nullptr;
            }
        }
    }

    void Update_UI(chrono::milliseconds) override { root_elem->Render(); }

    void Update(chrono::milliseconds) override {
        // Check for key presses 1-5 to switch test scenes
        if (IsKeyPressed(KEY_ONE)) {
            active_test = 1;
            Load_Test();
            std::cout << "\n===== SWITCHED TO TEST 1: Grow Sizing =====\n" << std::endl;
        } else if (IsKeyPressed(KEY_TWO)) {
            active_test = 2;
            Load_Test();
            std::cout << "\n===== SWITCHED TO TEST 2: Center Alignment =====\n" << std::endl;
        } else if (IsKeyPressed(KEY_THREE)) {
            active_test = 3;
            Load_Test();
            std::cout << "\n===== SWITCHED TO TEST 3: End Alignment =====\n" << std::endl;
        } else if (IsKeyPressed(KEY_FOUR)) {
            active_test = 4;
            Load_Test();
            std::cout << "\n===== SWITCHED TO TEST 4: Mixed Alignments =====\n" << std::endl;
        } else if (IsKeyPressed(KEY_FIVE)) {
            active_test = 5;
            Load_Test();
            std::cout << "\n===== SWITCHED TO TEST 5: Complex Nested Layout =====\n" << std::endl;
        } else if (IsKeyPressed(KEY_SIX)) {
            active_test = 6;
            Load_Test();
            std::cout
                << "\n===== SWITCHED TO TEST 6: Positioning (Static/Relative/Absolute) =====\n"
                << std::endl;
        }
    }
};
