#include "card.h"

#include "card_ui.h"
#include <raymath.h>

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

Object_UI* Card::Create_UI_Object(Game_UI_Manager& game_ui_manager) {
    return new Card_UI(this, game_ui_manager);
}
