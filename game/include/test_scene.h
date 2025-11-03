#pragma once

#include "card_game.h"
#include "scene.h"

/// Scene for testing layout algorithms. Contains elements and nested layout with different
/// alignments. Change ACTIVE_TEST to switch between different test layouts.

class Test_Scene : public Scene {
  private:
    // Current active test (can be changed at runtime with 1-5 keys)
    int active_test = 4;

    // Pre-created test layouts (kept in memory to avoid create/destroy cycles)
    EUI_Element* test_layouts[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};

    // Helper to create a colored box with text
    EUI_Box* Create_Box(const std::string& label, Color color, Vector2 size,
                        Layout_Model model = Layout_Model::Horizontal) {
        EUI_Box* box = new EUI_Box(model == Layout_Model::Vertical);
        box->background_color = color;
        box->size = size;
        box->padding = {10, 10, 10, 10};

        if (!label.empty()) {
            EUI_Text* text = new EUI_Text(label);
            text->text_horizontal_alignment = Alignment::Center;
            text->text_vertical_alignment = Alignment::Center;
            text->text_color = WHITE;
            text->font_size = 16;
            box->Add_Child(text);
        }

        return box;
    }

    // Test 1: Basic grow sizing with padding
    EUI_Element* Test_Grow_Sizing() {
        EUI_Box* root = new EUI_Box();
        root->horizontal_alignment = Alignment::Center;
        root->vertical_alignment = Alignment::Center;
        root->gap = 20;

        EUI_Box* box1 = Create_Box("Fixed Size (600x300)", BLUE, {600, 300});
        EUI_Box* box2 = Create_Box("", RED, {500, 500}, Layout_Model::Vertical);
        box2->gap = 10;

        EUI_Box* grow_box = Create_Box("Grow Box (480x480)", GREEN, {Size::Grow(), Size::Grow()});
        grow_box->padding = {15, 15, 15, 15};

        EUI_Text* grow_text = new EUI_Text("Grow Text");
        grow_text->size = {Size::Grow(), Size::Fit()};
        grow_text->text_horizontal_alignment = Alignment::Center;
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
        root->horizontal_alignment = Alignment::Center;
        root->vertical_alignment = Alignment::Center;
        root->gap = 20;

        // Horizontal centering test
        EUI_Box* hbox = Create_Box("", DARKGRAY, {800, 200});
        hbox->horizontal_alignment = Alignment::Center; // Center children horizontally
        hbox->vertical_alignment = Alignment::Center;   // Center children vertically
        hbox->gap = 10;

        hbox->Add_Child(Create_Box("Box 1", RED, {100, 100}));
        hbox->Add_Child(Create_Box("Box 2", GREEN, {100, 100}));
        hbox->Add_Child(Create_Box("Box 3", BLUE, {100, 100}));

        // Vertical centering test
        EUI_Box* vbox = Create_Box("", DARKGRAY, {200, 600}, Layout_Model::Vertical);
        vbox->horizontal_alignment = Alignment::Center; // Center children horizontally
        vbox->vertical_alignment = Alignment::Center;   // Center children vertically
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
        root->horizontal_alignment = Alignment::Center;
        root->vertical_alignment = Alignment::Center;
        root->gap = 30;

        // Horizontal end alignment
        EUI_Box* hbox = Create_Box("", DARKGRAY, {800, 150});
        hbox->horizontal_alignment = Alignment::End; // Align children to end (right)
        hbox->gap = 10;

        hbox->Add_Child(Create_Box("Right 1", RED, {120, 80}));
        hbox->Add_Child(Create_Box("Right 2", GREEN, {120, 80}));

        // Vertical end alignment
        EUI_Box* vbox = Create_Box("", DARKGRAY, {200, 500}, Layout_Model::Vertical);
        vbox->vertical_alignment = Alignment::End; // Align children to end (bottom)
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
        root->horizontal_alignment = Alignment::Center;
        root->gap = 15;

        // Top row: Start, Center, End
        EUI_Box* row1 = Create_Box("", DARKGRAY, {900, 150});
        row1->gap = 20;

        EUI_Box* col_start = Create_Box("", BLUE, {250, 130}, Layout_Model::Vertical);
        col_start->vertical_alignment = Alignment::Start;
        col_start->gap = 5;
        col_start->Add_Child(Create_Box("Start", WHITE, {80, 30}));
        col_start->Add_Child(Create_Box("Aligned", WHITE, {80, 30}));

        EUI_Box* col_center = Create_Box("", GREEN, {250, 130}, Layout_Model::Vertical);
        col_center->vertical_alignment = Alignment::Center;
        col_center->gap = 5;
        col_center->Add_Child(Create_Box("Center", WHITE, {80, 30}));
        col_center->Add_Child(Create_Box("Aligned", WHITE, {80, 30}));

        EUI_Box* col_end = Create_Box("", RED, {250, 130}, Layout_Model::Vertical);
        col_end->vertical_alignment = Alignment::End;
        col_end->gap = 5;
        col_end->Add_Child(Create_Box("End", WHITE, {80, 30}));
        col_end->Add_Child(Create_Box("Aligned", WHITE, {80, 30}));

        row1->Add_Child(col_start);
        row1->Add_Child(col_center);
        row1->Add_Child(col_end);

        // Bottom row: Cross-axis alignment
        EUI_Box* row2 = Create_Box("", DARKGRAY, {900, 200});
        row2->gap = 15;

        EUI_Box* tall = Create_Box("Tall\nBox", ORANGE, {150, 150});
        tall->vertical_alignment = Alignment::Start;

        EUI_Box* medium = Create_Box("Medium", PURPLE, {Size::Grow(), 100});
        medium->vertical_alignment = Alignment::Center;
        medium->horizontal_alignment = Alignment::Center;

        EUI_Box* small = Create_Box("Small", PINK, {150, 60});
        small->vertical_alignment = Alignment::End;

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
        root->gap = 10;
        root->padding = {10, 10, 10, 10};

        // Header
        EUI_Box* header = Create_Box("Header (Centered)", DARKBLUE, {Size::Grow(), 60});
        header->horizontal_alignment = Alignment::Center;

        // Content area with sidebar
        EUI_Box* content = Create_Box("", DARKGRAY, {Size::Grow(), Size::Grow()});
        content->gap = 10;

        EUI_Box* sidebar = Create_Box("Sidebar\n(Fixed)", PURPLE, {200, Size::Grow()});
        sidebar->vertical_alignment = Alignment::Start;

        EUI_Box* main_area =
            Create_Box("", GRAY, {Size::Grow(), Size::Grow()}, Layout_Model::Vertical);
        main_area->gap = 10;

        main_area->Add_Child(Create_Box("Content 1 (Grow)", GREEN, {Size::Grow(), Size::Grow()}));
        main_area->Add_Child(Create_Box("Content 2 (Grow)", BLUE, {Size::Grow(), Size::Grow()}));

        content->Add_Child(sidebar);
        content->Add_Child(main_area);

        // Footer
        EUI_Box* footer = Create_Box("Footer (End Aligned)", DARKGREEN, {Size::Grow(), 50});
        footer->horizontal_alignment = Alignment::End;

        root->Add_Child(header);
        root->Add_Child(content);
        root->Add_Child(footer);

        return root;
    }

    // Initialize all test layouts once
    void Initialize_All_Tests() {
        test_layouts[0] = Test_Grow_Sizing();
        test_layouts[1] = Test_Center_Alignment();
        test_layouts[2] = Test_End_Alignment();
        test_layouts[3] = Test_Mixed_Alignments();
        test_layouts[4] = Test_Complex_Nested();

        Card_Game& game = static_cast<Card_Game&>(application);

        // Attach all to context
        for (int i = 0; i < 5; i++) {
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
        if (index >= 0 && index < 5) {
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
        for (int i = 0; i < 5; i++) {
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
        }
    }
};
