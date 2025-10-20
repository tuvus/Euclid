#include "deck.h"
#include "deck_ui.h"

#include <algorithm>

void Draw_Card(Entity entity, int cards) {
    auto deck =
        std::get<1>(entity)->Get_Component<Deck_Component>(entity, &Deck_Component::component_type);
    for (int i = 0; i < cards; i++) {
        if (deck->deck.empty())
            Shuffle_Discard_Into_Deck(entity);
        if (deck->deck.empty())
            break;

        deck->hand.emplace_back(deck->deck.front());
        deck->deck.pop_front();
    }
}

void Shuffle_Discard_Into_Deck(Entity entity) {
    auto deck =
        std::get<1>(entity)->Get_Component<Deck_Component>(entity, &Deck_Component::component_type);
    for (auto card : deck->discard) {
        deck->deck.emplace_back(card);
    }
    deck->discard.clear();
    Shuffle_Deck(entity);
}

void Shuffle_Deck(Entity entity) {
    auto deck =
        std::get<1>(entity)->Get_Component<Deck_Component>(entity, &Deck_Component::component_type);
    std::ranges::shuffle(deck->deck, get<1>(entity)->ecs.random);
}

void Discard_Deck_Card(Entity entity, Entity_ID card) {
    auto deck =
        std::get<1>(entity)->Get_Component<Deck_Component>(entity, &Deck_Component::component_type);
    deck->hand.erase(ranges::find(deck->hand, card));
    deck->discard.emplace_back(card);
}

Object_UI* Create_Deck_UI(Entity entity, Game_UI_Manager& game_ui_manager) {
    auto deck =
        std::get<1>(entity)->Get_Component<Deck_Component>(entity, &Deck_Component::component_type);
    return new Deck_UI(entity, game_ui_manager, deck->player);
}

Component_Type Deck_Component::component_type = Component_Type{"Deck", sizeof(Deck_Component)};
