#include "card.h"

#include "card_ui.h"

bool Can_Play_Card(Card_Player* player, Entity entity, Vector2 pos) {
    auto* card =
        get<1>(entity)->Get_Component<Card_Component>(entity, &Card_Component::component_type);
    return player->money >= card->card_data->cost;
}

void Play_Card(Card_Player* player, Entity entity, Vector2 pos) {
    auto* card =
        get<1>(entity)->Get_Component<Card_Component>(entity, &Card_Component::component_type);
    player->money -= card->card_data->cost;
}

void Discard_Card(Card_Player* player, Entity entity) {
    player->deck->Discard_Card(entity);
}

Component_Type Card_Component::component_type = Component_Type{"Card", sizeof(Card_Component)};
