#include "game_manager.h"

#include "player.h"
#include <ranges>
#include <utility>

using namespace std;

Game_Manager::Game_Manager(Application& application, Network& network, vector<Player*> players,
                           Player* local_player, long seed)
    : application(application), network(network), local_player(local_player),
      players(std::move(players)) {
    player_steps = unordered_map<Player_ID, long>();
    objects = unordered_map<Obj_ID, Game_Object*>();
    random = std::minstd_rand(seed);

    if (!network.Is_Server()) {
        network.bind_rpc("setrandseed", [this](const long new_step) {
            On_Receive_Step_Update(new_step);
            return RPC_Manager::VALID_CALL_ON_CLIENTS;
        });
    }

    network.bind_rpc("stepupdate", [this](const long new_step) {
        On_Receive_Step_Update(new_step);
        return RPC_Manager::VALID_CALL_ON_CLIENTS;
    });
    if (network.Is_Server())
        network.bind_rpc("minstepupdate", [this](const long player_id, const long min_step) {
            On_Receive_Player_Step_Update(player_id, min_step);
            return RPC_Manager::VALID;
        });
    if (!network.Is_Server()) {
        network.call_rpc(false, "minstepupdate", local_player->player_id, step);
    }
}

Game_Manager::~Game_Manager() {
}

void Game_Manager::On_Receive_Step_Update(long new_step) {
    max_step = max(max_step, new_step);
}

void Game_Manager::On_Receive_Player_Step_Update(Player_ID player_id, long player_min_step) {
    player_steps[player_id] = max(player_steps[player_id], player_min_step);
    min_step = step;
    for (auto const& [id, player_step] : player_steps) {
        min_step = min(min_step, player_step);
    }
}

void Game_Manager::Update() {
    // If there are players still loading in the game then we will wait for them to load
    if (min_step == -1)
        return;

    // If we are the server and the players have reached a close enough step
    // We can continue on and proceed with the next step
    if (network.Is_Server() && step == max_step && min_step > step - 3) {
        max_step++;
    }
    if (step < max_step) {
        application.Get_Network()->Process_Step_Rpcs(step);

        for (const auto object : objects | views::values) {
            object->Update();
        }

        for (const Game_Object* object : objects_to_delete) {
            objects.erase(const_cast<Game_Object*>(object)->id);
            delete const_cast<Game_Object*>(object);
        }
        objects_to_delete.clear();
        step++;
        if (network.Is_Server()) {
            network.call_rpc(true, "stepupdate", step);
            if (player_steps.empty()) {
                min_step = step;
            }
        } else {
            network.call_rpc(false, "minstepupdate", local_player->player_id, step);
        }
    }
}

void Game_Manager::Add_Object(Game_Object* object) {
    objects.emplace(object->id, object);
    if (on_add_object != nullptr)
        on_add_object(object);
}

void Game_Manager::Delete_Object(Game_Object* object) {
    objects_to_delete.push_back(object);
    if (on_delete_object != nullptr)
        on_delete_object(object);
}

long Game_Manager::Get_New_Id() {
    return next_id++;
}

long Game_Manager::Get_Current_Step() const {
    return step;
}

Game_Object* Game_Manager::Get_Object(Obj_ID id) {
    return objects[id];
}

Player* Game_Manager::Get_Player(Player_ID player_id) {
    return *ranges::find_if(players,
                            [player_id](Player* player) { return player->player_id == player_id; });
}

vector<Game_Object*> Game_Manager::Get_All_Objects() {
    vector<Game_Object*> ret{};
    for (auto obj : objects | views::values) {
        ret.emplace_back(obj);
    }
    return ret;
}
