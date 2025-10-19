#include "card.h"

#include "card_ui.h"
#include <raymath.h>

bool Can_Play_Card(Entity card, Entity player) {
    auto* card_component =
        get<1>(card)->Get_Component<Card_Component>(get<0>(card), &Card_Component::component_type);
    // auto* player_component =
    // get<1>(player)->Get_Component<

    return card_player->money >= card_component->card_data->cost;
}

Card::Card(Game_Manager& game_manager, Game_Scene& game_scene, Card_Data& card_data)
    : Game_Object(game_manager, Vector2Zero(), 0, 1, WHITE), game_scene(game_scene),
      card_data(card_data) {
}

bool Card::Can_Play_Card(Card_Player* card_player, Vector2 pos) {
    return card_player->money >= card_data.cost;
}

void Card::Play_Card(Card_Player* card_player, Vector2 pos) {
    card_player->money -= card_data.cost;
    card_player->deck->Discard_Card(this);
}

void Card::Discard_Card(Card_Player* card_player) {
    card_player->deck->Discard_Card(this);
}

Object_UI* Create_Card_UI(Entity entity, Game_UI_Manager& game_ui_manager) {
    return new Card_UI(entity, game_ui_manager);
}

Component_Type Card_Component::component_type = Component_Type{"Card", sizeof(Card_Component)};
