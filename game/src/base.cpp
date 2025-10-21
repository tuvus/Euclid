#include "base.h"

#include "card_player.h"
#include "path.h"
#include "tower_card.h"

#include <random>
#include <raymath.h>

using namespace std;

void Init_Base(Entity entity, Card_Player& card_player, Vector2 pos, Path* path,
               int base_income_speed, int max_health) {
    auto transform = get<1>(entity)->Get_Component<Transform_Component>(
        entity, &Transform_Component::component_type);
    auto base =
        get<1>(entity)->Get_Component<Base_Component>(entity, &Base_Component::component_type);
    transform->pos = pos;
    transform->rot = 0;
    transform->scale = 1;
    base->card_player = card_player;
    base->path = path;
    base->base_income_speed = base_income_speed;
    base->max_health = max_health;
    base->health = max_health;
    base->time_until_income = 0;
}

void Update_Base(ECS* ecs, Entity entity) {
    auto base =
        get<1>(entity)->Get_Component<Base_Component>(entity, &Base_Component::component_type);
    if (--base->time_until_income == 0) {
        base->card_player.money++;
        base->time_until_income = 40;
    }

    if (base->card_player.Get_Deck()->hand.empty()) {
        Draw_Card(base->card_player.deck, 3);
    }

    if (base->card_player.ai && !base->card_player.Get_Deck()->hand.empty()) {
        std::uniform_int_distribution<int> hand_dist(0,
                                                     base->card_player.Get_Deck()->hand.size() - 1);
        int card_to_play = hand_dist(ecs->random);
        Entity card_entity =
            get<0>(ecs->entities_by_id[base->card_player.Get_Deck()->hand[card_to_play]]);
        auto card =
            get<1>(card_entity)
                ->Get_Component<Card_Component>(card_entity, &Card_Component::component_type);
        if (get<1>(card_entity)->entity_type.Is_Entity_Of_Type(Get_Tower_Card_Entity_Type())) {
            int i = 0;
            for (auto pos : base->path->positions) {
                i++;
                static uniform_int_distribution<int> tiny_offset(-20, 20);
                Vector2 r_pos = Vector2(pos.x + 50 + abs(tiny_offset(ecs->random)),
                                        pos.y + tiny_offset(ecs->random));
                if (Can_Play_Card(&base->card_player, card_entity, r_pos)) {
                    Play_Card(&base->card_player, card_entity, r_pos);
                    break;
                }
                Vector2 l_pos = Vector2(pos.x - 50 - abs(tiny_offset(ecs->random)),
                                        pos.y + tiny_offset(ecs->random));
                if (Can_Play_Card(&base->card_player, card_entity, l_pos)) {
                    Play_Card(&base->card_player, card_entity, l_pos);
                    break;
                }
            }

            // If we couldn't play the card due to lack of open positions discard it
            if (base->card_player.Get_Deck()->hand.size() > card_to_play &&
                base->card_player.Get_Deck()->hand[card_to_play] ==
                    Entity_Array::Get_Entity_Data(card_entity).id &&
                base->card_player.money >= card->card_data->cost) {
                Discard_Card(&base->card_player, card_entity);
            }

        } else if (Can_Play_Card(&base->card_player, card_entity, Vector2Zero())) {
            Play_Card(&base->card_player, card_entity, Vector2Zero());
        }
    }
}

Entity_Type* Get_Base_Entity_Type() {
    return new Entity_Type(
        vector{&Transform_Component::component_type, &Base_Component::component_type});
}

Component_Type Base_Component::component_type = Component_Type{"Base", sizeof(Base_Component)};
