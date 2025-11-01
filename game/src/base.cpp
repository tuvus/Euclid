#include "base.h"

#include "card_player.h"
#include "path.h"
#include "tower_card.h"
#include "unit_card.h"

#include <random>
#include <raymath.h>

using namespace std;

void Init_Base(Entity entity, Card_Player* card_player, Vector2 pos, vector<Path*> paths,
               int base_income_speed, int max_health) {
    auto transform = get<1>(entity)->Get_Component<Transform_Component>(entity);
    auto base = get<1>(entity)->Get_Component<Base_Component>(entity);
    transform->pos = pos;
    transform->rot = 0;
    transform->scale = 1;
    base->card_player = card_player;
    base->paths = paths;
    base->base_income_speed = base_income_speed;
    base->max_health = max_health;
    base->health = max_health;
    base->time_until_income = 0;
}

void Base_Update(ECS* ecs, Entity entity) {
    auto base = get<1>(entity)->Get_Component<Base_Component>(entity);
    if (--base->time_until_income <= 0) {
        base->card_player->money++;
        base->time_until_income = 40;
    }

    if (base->card_player->Get_Deck()->hand.empty()) {
        Draw_Card(base->card_player->deck, 3);
    }

    if (base->card_player->ai && !base->card_player->Get_Deck()->hand.empty()) {
        std::uniform_int_distribution<int> hand_dist(0, base->card_player->Get_Deck()->hand.size() -
                                                            1);
        int card_to_play = hand_dist(ecs->random);
        Entity card_entity =
            get<0>(ecs->entities_by_id[base->card_player->Get_Deck()->hand[card_to_play]]);
        if (get<0>(card_entity) == nullptr)
            return;
        auto card = get<1>(card_entity)->Get_Component<Card_Component>(card_entity);
        if (get<1>(card_entity)->entity_type.Is_Entity_Of_Type(Get_Tower_Card_Entity_Type())) {
            Try_Placing_Tower(ecs, card_to_play, card_entity, card, base);
        } else if (card->card_data->can_play_card(base->card_player, card_entity, Vector2Zero())) {
            static uniform_int_distribution<int> path_selector(0, INT_MAX);
            Vector2 pos =
                base->paths[path_selector(ecs->random) % base->paths.size()]->positions[0];
            card->card_data->play_card(base->card_player, card_entity, pos);
        }
    }
}

void Try_Placing_Tower(ECS* ecs, int card_index, Entity card_entity, Card_Component* card,
                       Base_Component* base) {
    int max_path_length = 0;
    for (auto path1 : base->paths) {
        max_path_length = max(max_path_length, static_cast<int>(path1->positions.size()));
    }
    for (int i = 0; i < max_path_length; i++) {
        for (auto path : base->paths) {
            if (path->positions.size() <= i)
                continue;
            Vector2 pos = path->positions[i];
            static uniform_int_distribution<int> tiny_offset(-20, 20);
            Vector2 r_pos = Vector2(pos.x + 50 + abs(tiny_offset(ecs->random)),
                                    pos.y + tiny_offset(ecs->random));
            if (card->card_data->can_play_card(base->card_player, card_entity, r_pos)) {
                card->card_data->play_card(base->card_player, card_entity, r_pos);
                return;
            }
            Vector2 l_pos = Vector2(pos.x - 50 - abs(tiny_offset(ecs->random)),
                                    pos.y + tiny_offset(ecs->random));
            if (card->card_data->can_play_card(base->card_player, card_entity, l_pos)) {
                card->card_data->play_card(base->card_player, card_entity, l_pos);
                return;
            }
        }
    }

    // If we couldn't play the card due to lack of open positions discard it
    if (base->card_player->Get_Deck()->hand.size() > card_index &&
        base->card_player->Get_Deck()->hand[card_index] ==
            Entity_Array::Get_Entity_Data(card_entity).id &&
        base->card_player->money >= card->card_data->cost) {
        card->card_data->discard_card(base->card_player, card_entity);
    }
}

Entity_Type* Get_Base_Entity_Type() {
    return new Entity_Type(
        vector{&Transform_Component::component_type, &Base_Component::component_type});
}

Component_Type Base_Component::component_type = Component_Type{"Base", sizeof(Base_Component)};
