#include "base.h"

#include "card_player.h"
#include "path.h"
#include "tower_card.h"
#include "unit_card.h"

#include <random>
#include <raymath.h>
#include <utility>

using namespace std;

void Init_Base(Entity entity, Game_Scene* game_scene, vector<Card_Player*> players,
               Entity_ID other_base_id, int team, Vector2 pos, vector<Path*> paths,
               int base_income_speed, int max_health) {
    auto transform = get<1>(entity)->Get_Component<Transform_Component>(entity);
    auto base = get<1>(entity)->Get_Component<Base_Component>(entity);
    transform->pos = pos;
    transform->rot = 0;
    transform->scale = 1;
    base->game_scene = game_scene;
    base->players = std::move(players);
    base->other_base_id = other_base_id;
    base->team = team;
    base->paths = paths;
    base->base_income_speed = base_income_speed;
    base->max_health = max_health;
    base->health = max_health;
    base->time_until_income = 0;
    base->units_on_path = vector<vector<Entity_ID>*>();
    for (auto path : paths) {
        base->units_on_path.emplace_back(new vector<Entity_ID>());
    }
}

void Base_Update(ECS* ecs, Entity entity) {
    auto base = get<1>(entity)->Get_Component<Base_Component>(entity);
    if (--base->time_until_income <= 0) {
        for (auto player : base->players) {
            player->money++;
        }
        base->time_until_income = base->base_income_speed;
    }

    for (auto player : base->players) {
        if (player->Get_Deck()->hand.empty()) {
            Draw_Card(get<0>(ecs->entities_by_id[player->deck_id]), 3);
        }
        if (player->ai && !player->Get_Deck()->hand.empty()) {
            std::uniform_int_distribution<int> hand_dist(0, player->Get_Deck()->hand.size() - 1);
            int card_to_play = hand_dist(ecs->random);
            Entity card_entity =
                get<0>(ecs->entities_by_id[player->Get_Deck()->hand[card_to_play]]);
            if (get<0>(card_entity) == nullptr)
                return;
            auto card = get<1>(card_entity)->Get_Component<Card_Component>(card_entity);
            if (get<1>(card_entity)->entity_type.Is_Entity_Of_Type(Get_Tower_Card_Entity_Type())) {
                Try_Placing_Tower(ecs, card_to_play, card_entity, card, player, base);
            } else if (card->card_data->can_play_card(player, card_entity, Vector2Zero())) {
                static uniform_int_distribution<int> path_selector(0, INT_MAX);
                Vector2 pos =
                    base->paths[path_selector(ecs->random) % base->paths.size()]->positions[0];
                card->card_data->play_card(player, card_entity, pos);
            }
        }
    }
}

void Try_Placing_Tower(ECS* ecs, int card_index, Entity card_entity, Card_Component* card,
                       Card_Player* card_player, Base_Component* base) {
    int max_path_length = 0;
    for (auto path1 : base->paths) {
        max_path_length = max(max_path_length, static_cast<int>(path1->positions.size()));
    }
    for (int i = 5; i < 15; i++) {
        for (auto path : base->paths) {
            if (path->positions.size() <= i)
                continue;
            Vector2 pos = path->positions[i];
            static uniform_int_distribution<int> tiny_offset(-20, 20);
            Vector2 r_pos = Vector2(pos.x + 50 + abs(tiny_offset(ecs->random)),
                                    pos.y + tiny_offset(ecs->random));
            if (card->card_data->can_play_card(card_player, card_entity, r_pos)) {
                card->card_data->play_card(card_player, card_entity, r_pos);
                return;
            }
            Vector2 l_pos = Vector2(pos.x - 50 - abs(tiny_offset(ecs->random)),
                                    pos.y + tiny_offset(ecs->random));
            if (card->card_data->can_play_card(card_player, card_entity, l_pos)) {
                card->card_data->play_card(card_player, card_entity, l_pos);
                return;
            }
        }
    }

    // If we couldn't play the card due to lack of open positions discard it
    if (card_player->Get_Deck()->hand.size() > card_index &&
        card_player->Get_Deck()->hand[card_index] ==
            Entity_Array::Get_Entity_Data(card_entity).id &&
        card_player->money >= card->card_data->cost) {
        card->card_data->discard_card(card_player, card_entity);
    }
}

Entity_Type* Get_Base_Entity_Type() {
    return new Entity_Type(
        vector{&Transform_Component::component_type, &Base_Component::component_type});
}

Component_Type Base_Component::component_type = Component_Type{"Base", sizeof(Base_Component)};
