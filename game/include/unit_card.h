#pragma once
#include "card.h"
#include "unit.h"

class Unit_Card : public Card {
  public:
    Unit_Data& unit_data;
    int unit_count;

    Unit_Card(Game_Manager& game_manager, Game_Scene& game_scene, Card_Data& card_data,
              Unit_Data& unit_data, int unit_count)
        : Card(game_manager, game_scene, card_data), unit_data(unit_data), unit_count(unit_count) {}

    Card* Clone() override {
        return new Unit_Card(game_manager, game_scene, card_data, unit_data, unit_count);
    }

    void Play_Card(Card_Player* card_player, Vector2 pos) override {
        Card::Play_Card(card_player, pos);
        for (int i = 0; i < unit_count; i++) {
            game_manager.Add_Object(
                new Unit(game_manager, unit_data,
                         card_player->team == 0 ? game_scene.f_path : game_scene.r_path, 1, i * 10,
                         card_player->team, .4f, card_player->team ? RED : BLUE));
        }
    }

    void Update() override {}
};