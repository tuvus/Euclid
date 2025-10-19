#include "tower_card.h"

#include "game_scene.h"

Tower_Card::Tower_Card(Game_Manager& game_manager, Game_Scene& game_scene, Card_Data& card_data,
                       Tower_Data& tower_data)
    : Card(game_manager, game_scene, card_data), tower_data(tower_data) {
}

Card* Tower_Card::Clone() {
    return new Tower_Card(game_manager, game_scene, card_data, tower_data);
}

bool Tower_Card::Can_Play_Card(Card_Player* card_player, Vector2 pos) {
    return Card::Can_Play_Card(card_player, pos) && game_scene.Can_Place_Tower(pos, 50);
}

void Tower_Card::Play_Card(Card_Player* player, Vector2 pos) {
    Card::Play_Card(player, pos);
    game_manager.Add_Object(new Tower(game_manager, tower_data, Vector2(pos.x, pos.y), 150,
                                      player->team, .4f, player->team ? RED : BLUE));
}
