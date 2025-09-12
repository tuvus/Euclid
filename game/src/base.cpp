#include "base.h"

#include "card_player.h"
#include "path.h"
#include "tower_card.h"

#include <random>
#include <raymath.h>

Base::Base(Game_Manager& game_manager, Card_Player& card_player, Vector2 pos, Path* path,
           int base_income_speed, int max_health)
    : Game_Object(game_manager, pos, 0, 1, WHITE), card_player(card_player), path(path),
      time_until_income(base_income_speed), health(max_health) {
}

void Base::Update() {
    if (--time_until_income == 0) {
        card_player.money++;
        time_until_income = 40;
    }

    if (card_player.deck->hand.empty()) {
        card_player.deck->Draw_Card(3);
    }

    if (card_player.ai && !card_player.deck->hand.empty()) {
        std::uniform_int_distribution<int> hand_dist(0, card_player.deck->hand.size() - 1);
        int card_to_play = hand_dist(game_manager.random);
        Card* card = card_player.deck->hand[card_to_play];
        if (Tower_Card* tower_card = dynamic_cast<Tower_Card*>(card)) {
            int i = 0;
            for (auto pos : path->positions) {
                i++;
                static uniform_int_distribution<int> tiny_offset(-20, 20);
                Vector2 r_pos = Vector2(pos.x + 50 + abs(tiny_offset(game_manager.random)),
                                        pos.y + tiny_offset(game_manager.random));
                if (tower_card->Can_Play_Card(&card_player, r_pos)) {
                    tower_card->Play_Card(&card_player, r_pos);
                    break;
                }
                Vector2 l_pos = Vector2(pos.x - 50 - abs(tiny_offset(game_manager.random)),
                                        pos.y + tiny_offset(game_manager.random));
                if (tower_card->Can_Play_Card(&card_player, l_pos)) {
                    tower_card->Play_Card(&card_player, l_pos);
                    break;
                }
            }

            // If we couldn't play the card due to lack of open positions discard it
            if (card_player.deck->hand.size() > card_to_play &&
                card_player.deck->hand[card_to_play] == card &&
                card_player.money >= card->card_data.cost) {
                tower_card->Discard_Card(&card_player);
            }

        } else if (card->Can_Play_Card(&card_player, Vector2Zero())) {
            card->Play_Card(&card_player, Vector2Zero());
        }
    }
}
