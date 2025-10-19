#include "game_scene.h"

#include "base.h"
#include "card_player.h"
#include "card_ui.h"
#include "tower.h"
#include "tower_card.h"
#include "unit.h"
#include "unit_card.h"

Game_Scene::Game_Scene(Card_Game& card_game)
    : Scene(card_game), card_game(card_game), active_card(nullptr), time_until_income(0) {
    EUI_HBox* root = new EUI_HBox();
    root_elem = root;

    root->pos = {0, 0};
    root->dim = {(float) card_game.screen_width, (float) card_game.screen_height};
    root->style.vertical_alignment = Alignment::Center;
    root->style.horizontal_alignment = Alignment::Center;

    money_text = new EUI_Text("Money: 0");
    money_text->style.font_size = 24;
    root->Add_Child(money_text);
    auto* button = new EUI_Button("Menu", [&card_game] {
        card_game.Close_Network();
        card_game.set_ui_screen(MENU);
    });

    button->style.padding = {10, 20, 10, 20};
    root->Add_Child(button);

    unit_texture = LoadTextureFromImage(LoadImage("resources/Arrow.png"));
    tower_texture = LoadTextureFromImage(LoadImage("resources/Tower.png"));
}

Game_Scene::~Game_Scene() {
    if (card_game.Get_Network() != nullptr)
        card_game.Get_Network()->connection_events->erase(
            static_cast<Network_Events_Receiver*>(this));
}

void Game_Scene::Setup_Scene(vector<Player*> players, Player* local_player, long seed) {
    ranges::sort(players, [](Player* a, Player* b) { return a->player_id <= b->player_id; });
    game_manager = std::make_unique<Game_Manager>(card_game, *card_game.Get_Network(), players,
                                                  local_player, seed);
    ecs = new ECS(application, seed);
    auto unit_components =
        vector{&Transform_Component::component_type, &Unit_Component::component_type};
    ecs->Register_System(new System(new Entity_Type(unit_components), Unit_Update));
    auto tower_components =
        vector{&Transform_Component::component_type, &Tower_Component::component_type};
    ecs->Register_System(new System(new Entity_Type(tower_components), Tower_Update));

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

    game_ui_manager = make_unique<Game_UI_Manager>(card_game, *ecs);
    if (static_cast<Card_Player*>(game_manager->local_player)->team == 1) {
        game_ui_manager->camera.rotation = 180;
    }
    card_game.Get_Network()->bind_rpc(
        "playcard", [this](Player_ID player_id, Entity_ID entity_id, float x, float y) {
            Card_Player* player = static_cast<Card_Player*>(game_manager->Get_Player(player_id));
            // Check if the card is in the hand
            if (ranges::find(player->deck->hand, entity_id) == player->deck->hand.end())
                return RPC_Manager::INVALID;
            // if (!card->Can_Play_Card(player, Vector2(x, y)))
            // return RPC_Manager::INVALID;

            // card->Play_Card(player, Vector2(x, y));
            return RPC_Manager::VALID_CALL_ON_CLIENTS;
        });
    card_game.Get_Network()->bind_rpc("discard", [this](Player_ID player_id, Entity_ID entity_id) {
        Card_Player* player = static_cast<Card_Player*>(game_manager->Get_Player(player_id));
        // Check if the card is in the hand
        if (ranges::find(player->deck->hand, entity_id) == player->deck->hand.end())
            return RPC_Manager::INVALID;

        // card->Discard_Card(player);
        return RPC_Manager::VALID_CALL_ON_CLIENTS;
    });

    ecs->Create_Entity_Type(Get_Unit_Entity_Type()->components, Create_Unit_UI);
    ecs->Create_Entity_Type(Get_Tower_Entity_Type()->components, Create_Tower_UI);
    ecs->Create_Entity_Type(vector{&Deck_Component::component_type}, Create_Deck_UI);
    ecs->Create_Entity_Type(Get_Unit_Card_Entity_Type()->components, Create_Card_UI);
    ecs->Create_Entity_Type(Get_Tower_Card_Entity_Type()->components, Create_Card_UI);

    Texture2D card_texture = LoadTextureFromImage(LoadImage("resources/Card.png"));
    card_datas.emplace_back(
        new Card_Data{card_texture, "Send Units", "Sends 7 units to the opponent.", 5});
    card_datas.emplace_back(
        new Card_Data{card_texture, "Send Units", "Sends 11 units to the opponent.", 8});
    card_datas.emplace_back(
        new Card_Data{card_texture, "Send Units", "Sends 18 units to the opponent.", 12});
    card_datas.emplace_back(
        new Card_Data{card_texture, "Tower", "Places a tower that shoots opposing units.", 15});

    vector<Entity_ID> starting_cards{};
    starting_cards.emplace_back(Init_Unit_Card(ecs->Create_Entity(Get_Unit_Card_Entity_Type()),
                                               card_datas[0], {7, unit_texture}));
    starting_cards.emplace_back(Init_Unit_Card(ecs->Create_Entity(Get_Unit_Card_Entity_Type()),
                                               card_datas[0], {7, unit_texture}));
    starting_cards.emplace_back(Init_Unit_Card(ecs->Create_Entity(Get_Unit_Card_Entity_Type()),
                                               card_datas[1], {11, unit_texture}));
    starting_cards.emplace_back(Init_Unit_Card(ecs->Create_Entity(Get_Unit_Card_Entity_Type()),
                                               card_datas[2], {18, unit_texture}));
    starting_cards.emplace_back(Init_Tower_Card(ecs->Create_Entity(Get_Tower_Card_Entity_Type()),
                                                card_datas[3], {0, true, 1, 90}));

    for (auto player : game_manager->players) {
        Card_Player* card_player = static_cast<Card_Player*>(player);
        if (card_player->team == -1)
            continue;

        card_player->deck = new Deck(*game_manager, card_player);
        game_manager->Add_Object(card_player->deck);
        for (auto& card : starting_cards) {
            card_player->deck->deck.emplace_back(
                Entity_Array::Get_Entity_Data(get<0>(ecs->Copy_Entity(card))).id);
        }
        card_player->deck->Shuffle_Deck();
        card_player->deck->Draw_Card(3);
        game_manager->Add_Object(new Base(*game_manager, *card_player,
                                          Get_Team_Path(card_player->team)->positions[0],
                                          Get_Team_Path(card_player->team), 20, 100));
    }

    for (Entity_ID starting_card : starting_cards) {
        ecs->Delete_Entity(starting_card);
    }
    starting_cards.clear();

    money_text->is_visible = static_cast<Card_Player*>(game_manager->local_player)->team != -1;
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
    game_ui_manager->Update_UI(delta_time, card_game.eui_ctx);

    Card_Player* local_player = static_cast<Card_Player*>(game_manager->local_player);

    auto mouse_pos = card_game.eui_ctx->input.mouse_position;
    auto world_pos = GetScreenToWorld2D(mouse_pos, game_ui_manager->camera);
    if (Tower_Card* tower_card = dynamic_cast<Tower_Card*>(active_card)) {
        if (Can_Place_Tower(world_pos, 50))
            DrawCircle(mouse_pos.x, mouse_pos.y, 75, ColorAlpha(LIGHTGRAY, .3f));
        DrawCircle(mouse_pos.x, mouse_pos.y, 20, local_player->team ? RED : BLUE);

        if (!card_game.eui_ctx->input.left_mouse_down) {
            // If the cursor is still over the card, cancel
            if (!static_cast<Card_UI*>(game_ui_manager->active_ui_objects[active_card])
                     ->is_hovered &&
                active_card->Can_Play_Card(local_player, Vector2(world_pos.x, world_pos.y)))
                this->card_game.Get_Network()->call_game_rpc(
                    "playcard", local_player->player_id, tower_card->id, world_pos.x, world_pos.y);
            active_card = nullptr;
        }
    } else if (active_card != nullptr && !card_game.eui_ctx->input.left_mouse_down) {
        // If the cursor is still over the card, cancel
        if (!static_cast<Card_UI*>(game_ui_manager->active_ui_objects[active_card])->is_hovered &&
            active_card->Can_Play_Card(local_player, Vector2(world_pos.x, world_pos.y)))
            this->card_game.Get_Network()->call_game_rpc("playcard", local_player->player_id,
                                                         active_card->id, world_pos.x, world_pos.y);
        active_card = nullptr;
    }

    money_text->Set_Text("Money: " + to_string(local_player->money));

    root_elem->Render();
}

bool Game_Scene::Can_Place_Tower(Vector2 pos, float min_dist) const {
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
    ecs->Update();
    game_manager->Update();
}

void Game_Scene::On_Disconnected() {
    card_game.set_ui_screen(MENU);
    card_game.Close_Network();
}

void Game_Scene::On_Server_Stop() {
    card_game.set_ui_screen(MENU);
    card_game.Close_Network();
}

void Game_Scene::Activate_Card(Card* card) {
    active_card = card;
}

Path* Game_Scene::Get_Team_Path(int team) const {
    return team == 0 ? f_path : r_path;
}

Color Game_Scene::Get_Team_Color(int team) {
    return team ? RED : BLUE;
}