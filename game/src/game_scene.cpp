#include "game_scene.h"

#include "card_player.h"
#include "tower.h"
#include "unit.h"

Game_Scene::Game_Scene(Card_Game& card_game)
    : Scene(card_game), card_game(card_game), placing_tower(false), time_until_income(0) {
    EUI_HBox* root = new EUI_HBox();
    root_elem = root;

    root->pos = {0, 0};
    root->dim = {(float) card_game.screen_width, (float) card_game.screen_height};
    root->Set_Width_Fixed((float) card_game.screen_width);
    root->Set_Height_Fixed((float) card_game.screen_height);
    root->style.vertical_alignment = Alignment::Center;
    root->style.horizontal_alignment = Alignment::Center;

    money_text = new EUI_Text("Money: 0");
    root->Add_Child(money_text);
    auto* button = new EUI_Button("Menu", [&card_game] {
        card_game.Close_Network();
        card_game.set_ui_screen(MENU);
    });

    button->style.padding = {10, 20, 10, 20};
    root->Add_Child(button);
    spawn_unit_button = new EUI_Button("Spawn Unit (1)", [this] {
        this->card_game.Get_Network()->call_game_rpc("spawnunit",
                                                     this->game_manager->local_player->player_id);
    });
    root->Add_Child(spawn_unit_button);
    place_tower_button = new EUI_Button("Place Tower (10)", [this] { placing_tower = true; });
    root->Add_Child(place_tower_button);
    card_game.Get_Network()->connection_events->emplace(
        static_cast<Network_Events_Receiver*>(this));
}

Game_Scene::~Game_Scene() {
    if (card_game.Get_Network() != nullptr)
        card_game.Get_Network()->connection_events->erase(
            static_cast<Network_Events_Receiver*>(this));
}

void Game_Scene::Setup_Scene(vector<Player*> players, Player* local_player, long seed) {
    game_manager = std::make_unique<Game_Manager>(card_game, *card_game.Get_Network(), players,
                                                  local_player, seed);
    vector<Vector2> positions = vector<Vector2>();
    static uniform_int_distribution<int> start_dist(-200, 200);
    positions.emplace_back(start_dist(game_manager->random) + card_game.screen_width / 2,
                           card_game.screen_height - 80);

    while (positions[positions.size() - 1].y > 80) {
        Vector2 prev_pos = positions[positions.size() - 1];
        static uniform_int_distribution<int> move_dist(-40, 40);
        int new_x = max(min((int) prev_pos.x + move_dist(game_manager->random),
                            static_cast<int>(card_game.screen_width - 100)),
                        100);
        static uniform_int_distribution<int> forward_dist(20, 40);
        positions.emplace_back(new_x, prev_pos.y - forward_dist(game_manager->random));
    }

    f_path = new Path(positions);
    vector<Vector2> reversed = vector<Vector2>(positions);
    ranges::reverse(reversed);
    r_path = new Path(reversed);

    game_ui_manager = make_unique<Game_UI_Manager>(card_game, *game_manager);
    if (static_cast<Card_Player*>(game_manager->local_player)->team == 1) {
        game_ui_manager->camera.rotation = 180;
    }

    card_game.Get_Network()->bind_rpc("spawnunit", [this](Player_ID player_id) {
        Card_Player* player = static_cast<Card_Player*>(game_manager->Get_Player(player_id));
        if (player->money < 1)
            return RPC_Manager::INVALID;

        player->money -= 1;
        game_manager->Add_Object(new Unit(
            *game_manager, LoadTextureFromImage(LoadImage("resources/Arrow.png")),
            player->team == 0 ? f_path : r_path, 1, player->team, .4f, player->team ? RED : BLUE));
        return RPC_Manager::VALID_CALL_ON_CLIENTS;
    });
    card_game.Get_Network()->bind_rpc("spawntower", [this](Player_ID player_id, float x, float y) {
        if (!Can_Place_Tower(Vector2(x, y), 50))
            return RPC_Manager::INVALID;

        Card_Player* player = static_cast<Card_Player*>(game_manager->Get_Player(player_id));
        if (player->money < 10)
            return RPC_Manager::INVALID;

        player->money -= 10;
        game_manager->Add_Object(
            new Tower(*game_manager, LoadTextureFromImage(LoadImage("resources/Tower.png")),
                      Vector2(x, y), 150, player->team, .4f, player->team ? RED : BLUE));
        return RPC_Manager::VALID_CALL_ON_CLIENTS;
    });
}

void Game_Scene::Update_UI(chrono::milliseconds delta_time) {
    BeginMode2D(game_ui_manager->camera);
    // Visualize path
    Vector2 past_pos = Vector2One() * -1;
    for (const auto& pos : f_path->positions) {
        if (past_pos != Vector2One() * -1)
            DrawLineEx(past_pos, pos, 40, DARKGRAY);
        DrawCircle(pos.x, pos.y, 20, DARKGRAY);
        past_pos = pos;
    }
    EndMode2D();

    // Draw arrows
    game_ui_manager->Update_UI(delta_time);

    Card_Player* local_player = static_cast<Card_Player*>(game_manager->local_player);

    if (placing_tower) {
        auto mouse_pos = card_game.eui_ctx->input.mouse_position;
        auto world_pos = GetScreenToWorld2D(mouse_pos, game_ui_manager->camera);
        if (Can_Place_Tower(world_pos, 50))
            DrawCircle(mouse_pos.x, mouse_pos.y, 75, ColorAlpha(LIGHTGRAY, .3f));
        DrawCircle(mouse_pos.x, mouse_pos.y, 20, local_player->team ? RED : BLUE);

        if (card_game.eui_ctx->input.left_mouse_pressed) {
            this->card_game.Get_Network()->call_game_rpc("spawntower", local_player->player_id,
                                                         world_pos.x, world_pos.y);
            placing_tower = false;
        }
    }

    spawn_unit_button->is_enabled = local_player->money >= 1;
    place_tower_button->is_enabled = local_player->money >= 10;
    money_text->Set_Text("Money: " + to_string(local_player->money));

    root_elem->Render();
}

bool Game_Scene::Can_Place_Tower(Vector2 pos, float min_dist) {
    for (auto* object : game_manager->Get_All_Objects()) {
        if (Tower* other = dynamic_cast<Tower*>(object)) {
            if (Vector2Distance(pos, other->pos) <= min_dist)
                return false;
        }
    }
    for (auto path_pos : f_path->positions) {
        if (Vector2Distance(pos, path_pos) <= min_dist)
            return false;
    }
    return true;
}

void Game_Scene::Update(std::chrono::milliseconds) {
    game_manager->Update();
    if (time_until_income-- == 0) {
        for (auto* player : game_manager->players) {
            Card_Player* card_player = static_cast<Card_Player*>(player);
            card_player->money++;
        }
        time_until_income = 40;
    }
}

void Game_Scene::On_Disconnected() {
    card_game.set_ui_screen(MENU);
    card_game.Close_Network();
}

void Game_Scene::On_Server_Stop() {
    card_game.set_ui_screen(MENU);
    card_game.Close_Network();
}
