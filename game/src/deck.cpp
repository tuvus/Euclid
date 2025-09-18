#include "deck.h"

#include "deck_ui.h"
#include "game_manager.h"

#include <algorithm>
#include <raymath.h>

Deck::Deck(Game_Manager& game_manager, Card_Player* player)
    : Game_Object(game_manager, Vector2Zero(), 0, 1, WHITE), player(player) {
}

void Deck::Draw_Card(int cards) {
    for (int i = 0; i < cards; i++) {
        if (deck.empty())
            Shuffle_Discard_Into_Deck();
        if (deck.empty())
            break;

        hand.emplace_back(deck.front());
        deck.pop_front();
    }
}

void Deck::Shuffle_Discard_Into_Deck() {
    for (auto card : discard) {
        deck.emplace_back(card);
    }
    discard.clear();
    Shuffle_Deck();
}

void Deck::Shuffle_Deck() {
    std::ranges::shuffle(deck, game_manager.random);
}

void Deck::Update() {
}

void Deck::Discard_Card(Card* card) {
    hand.erase(ranges::find(hand, card));
    discard.emplace_back(card);
}

Object_UI* Deck::Create_UI_Object(Game_UI_Manager& game_ui_manager) {
    return new Deck_UI(this, game_ui_manager);
}
