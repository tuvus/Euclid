#include "test_layout_scene.h"
#include <algorithm>

Test_Layout_Scene::Test_Layout_Scene(Card_Game& card_game) : Scene(card_game) {
    // Create root container with fixed size
    EUI_HBox* root = new EUI_HBox();
    root_elem = root;
    root->pos = {0, 0};
    root->dim = {SCREEN_WIDTH, SCREEN_HEIGHT};
    root->Set_Width_Fixed(SCREEN_WIDTH);
    root->Set_Height_Fixed(SCREEN_HEIGHT);
    root->style.vertical_alignment = Alignment::Center;
    root->style.horizontal_alignment = Alignment::Center;
    root->style.padding = {20, 20, 20, 20};
    root->gap = 20;

    // Left column - demonstrates auto-sizing containers
    EUI_VBox* left_column = new EUI_VBox();
    left_column->style.background_color = ColorAlpha(RED, 0.3f);
    left_column->Set_Width_Auto();
    left_column->Set_Height_Auto();
    left_column->style.padding = {15, 15, 15, 15};
    left_column->gap = 10;
    root->Add_Child(left_column);

    // Title for left column
    EUI_Text* left_title = new EUI_Text("Auto-Sized Containers");
    left_title->style.font_size = 24;
    left_title->style.text_color = WHITE;
    left_column->Add_Child(left_title);

    // Auto-sized container 1
    EUI_VBox* auto_container1 = new EUI_VBox();
    auto_container1->style.background_color = ColorAlpha(ORANGE, 0.7f);
    auto_container1->Set_Width_Auto();
    auto_container1->Set_Height_Auto();
    auto_container1->style.padding = {10, 10, 10, 10};
    auto_container1->gap = 5;
    left_column->Add_Child(auto_container1);

    EUI_Text* text1 = new EUI_Text("Auto Container 1");
    text1->style.font_size = 16;
    text1->style.text_color = WHITE;
    auto_container1->Add_Child(text1);

    EUI_Text* text2 = new EUI_Text("Sizes to content");
    text2->style.font_size = 14;
    text2->style.text_color = WHITE;
    auto_container1->Add_Child(text2);

    // Auto-sized container 2
    EUI_VBox* auto_container2 = new EUI_VBox();
    auto_container2->style.background_color = ColorAlpha(YELLOW, 0.7f);
    auto_container2->Set_Width_Auto();
    auto_container2->Set_Height_Auto();
    auto_container2->style.padding = {10, 10, 10, 10};
    auto_container2->gap = 5;
    left_column->Add_Child(auto_container2);

    EUI_Text* text3 = new EUI_Text("Auto Container 2");
    text3->style.font_size = 16;
    text3->style.text_color = BLACK;
    auto_container2->Add_Child(text3);

    EUI_Text* text4 = new EUI_Text("Different content size");
    text4->style.font_size = 14;
    text4->style.text_color = BLACK;
    auto_container2->Add_Child(text4);

    // Middle column - demonstrates fixed-sized containers
    EUI_VBox* middle_column = new EUI_VBox();
    middle_column->style.background_color = ColorAlpha(GREEN, 0.3f);
    middle_column->Set_Width_Fixed(250.0f);
    middle_column->Set_Height_Auto();
    middle_column->style.padding = {15, 15, 15, 15};
    middle_column->gap = 10;
    root->Add_Child(middle_column);

    // Title for middle column
    EUI_Text* middle_title = new EUI_Text("Fixed-Sized Containers");
    middle_title->style.font_size = 24;
    middle_title->style.text_color = WHITE;
    middle_column->Add_Child(middle_title);

    // Fixed-sized container 1
    EUI_VBox* fixed_container1 = new EUI_VBox();
    fixed_container1->style.background_color = ColorAlpha(BLUE, 0.7f);
    fixed_container1->Set_Width_Fixed(200.0f);
    fixed_container1->Set_Height_Fixed(120.0f);
    fixed_container1->style.padding = {10, 10, 10, 10};
    fixed_container1->style.vertical_alignment = Alignment::Center;
    fixed_container1->style.horizontal_alignment = Alignment::Center;
    middle_column->Add_Child(fixed_container1);

    EUI_Text* text5 = new EUI_Text("Fixed 200x120");
    text5->style.font_size = 16;
    text5->style.text_color = WHITE;
    text5->style.text_horizontal_alignment = Alignment::Center;
    text5->style.text_vertical_alignment = Alignment::Center;
    fixed_container1->Add_Child(text5);

    // Fixed-sized container 2
    EUI_VBox* fixed_container2 = new EUI_VBox();
    fixed_container2->style.background_color = ColorAlpha(PURPLE, 0.7f);
    fixed_container2->Set_Width_Fixed(180.0f);
    fixed_container2->Set_Height_Fixed(100.0f);
    fixed_container2->style.padding = {10, 10, 10, 10};
    fixed_container2->style.vertical_alignment = Alignment::Center;
    fixed_container2->style.horizontal_alignment = Alignment::Center;
    middle_column->Add_Child(fixed_container2);

    EUI_Text* text6 = new EUI_Text("Fixed 180x100");
    text6->style.font_size = 16;
    text6->style.text_color = WHITE;
    text6->style.text_horizontal_alignment = Alignment::Center;
    text6->style.text_vertical_alignment = Alignment::Center;
    fixed_container2->Add_Child(text6);

    // Right column - demonstrates full-width containers
    EUI_VBox* right_column = new EUI_VBox();
    right_column->style.background_color = ColorAlpha(MAGENTA, 0.3f);
    right_column->Set_Width_Full();
    right_column->Set_Height_Auto();
    right_column->style.padding = {15, 15, 15, 15};
    right_column->gap = 10;
    root->Add_Child(right_column);

    // Title for right column
    EUI_Text* right_title = new EUI_Text("Full-Width Containers");
    right_title->style.font_size = 24;
    right_title->style.text_color = WHITE;
    right_column->Add_Child(right_title);

    // Full-width container 1
    EUI_VBox* full_container1 = new EUI_VBox();
    full_container1->style.background_color = ColorAlpha(SKYBLUE, 0.7f);
    full_container1->Set_Width_Full();
    full_container1->Set_Height_Fixed(80.0f);
    full_container1->style.padding = {10, 10, 10, 10};
    full_container1->style.vertical_alignment = Alignment::Center;
    full_container1->style.horizontal_alignment = Alignment::Center;
    right_column->Add_Child(full_container1);

    EUI_Text* text7 = new EUI_Text("Full Width Container");
    text7->style.font_size = 16;
    text7->style.text_color = BLACK;
    text7->style.text_horizontal_alignment = Alignment::Center;
    text7->style.text_vertical_alignment = Alignment::Center;
    full_container1->Add_Child(text7);

    // Full-width container 2
    EUI_VBox* full_container2 = new EUI_VBox();
    full_container2->style.background_color = ColorAlpha(GREEN, 0.7f);
    full_container2->Set_Width_Full();
    full_container2->Set_Height_Fixed(80.0f);
    full_container2->style.padding = {10, 10, 10, 10};
    full_container2->style.vertical_alignment = Alignment::Center;
    full_container2->style.horizontal_alignment = Alignment::Center;
    right_column->Add_Child(full_container2);

    EUI_Text* text8 = new EUI_Text("Another Full Width");
    text8->style.font_size = 16;
    text8->style.text_color = BLACK;
    text8->style.text_horizontal_alignment = Alignment::Center;
    text8->style.text_vertical_alignment = Alignment::Center;
    full_container2->Add_Child(text8);

    // Nested container demonstration
    EUI_VBox* nested_demo = new EUI_VBox();
    nested_demo->style.background_color = ColorAlpha(WHITE, 0.2f);
    nested_demo->Set_Width_Auto();
    nested_demo->Set_Height_Auto();
    nested_demo->style.padding = {10, 10, 10, 10};
    nested_demo->gap = 5;
    right_column->Add_Child(nested_demo);

    EUI_Text* nested_title = new EUI_Text("Nested Auto Container");
    nested_title->style.font_size = 14;
    nested_title->style.text_color = WHITE;
    nested_demo->Add_Child(nested_title);

    // Nested auto container
    EUI_HBox* nested_auto = new EUI_HBox();
    nested_auto->style.background_color = ColorAlpha(GRAY, 0.5f);
    nested_auto->Set_Width_Auto();
    nested_auto->Set_Height_Auto();
    nested_auto->style.padding = {5, 5, 5, 5};
    nested_auto->gap = 5;
    nested_demo->Add_Child(nested_auto);

    EUI_Text* nested_text1 = new EUI_Text("Nested");
    nested_text1->style.font_size = 12;
    nested_text1->style.text_color = WHITE;
    nested_auto->Add_Child(nested_text1);

    EUI_Text* nested_text2 = new EUI_Text("Auto");
    nested_text2->style.font_size = 12;
    nested_text2->style.text_color = WHITE;
    nested_auto->Add_Child(nested_text2);

    // Back to menu button
    EUI_Button* back_button = new EUI_Button("Back to Menu", [&card_game] {
        card_game.set_ui_screen(MENU);
    });
    back_button->style.padding = {10, 20, 10, 20};
    back_button->style.background_color = ColorAlpha(DARKGRAY, 0.8f);
    back_button->style.text_color = WHITE;
    right_column->Add_Child(back_button);
}

void Test_Layout_Scene::Update_UI(chrono::milliseconds) {
    // Render the UI elements
    root_elem->Render();
}

void Test_Layout_Scene::Update(chrono::milliseconds deltaTime) {
}
