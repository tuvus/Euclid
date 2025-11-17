#include "deck.h"
#include "deck_ui.h"

#include <algorithm>

void Init_Deck(Entity entity, Card_Player* player, Game_Scene* game_scene) {
    auto deck = std::get<1>(entity)->Get_Component<Deck_Component>(entity);
    deck->player = player;
    deck->deck = std::deque<Entity_ID>();
    deck->hand = std::vector<Entity_ID>();
    deck->discard = std::vector<Entity_ID>();
    deck->game_scene = game_scene;
}

void Draw_Card(Entity entity, int cards) {
    auto deck = std::get<1>(entity)->Get_Component<Deck_Component>(entity);
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
    auto deck = std::get<1>(entity)->Get_Component<Deck_Component>(entity);
    for (auto card : deck->discard) {
        deck->deck.emplace_back(card);
    }
    deck->discard.clear();
    Shuffle_Deck(entity);
}

void Shuffle_Deck(Entity entity) {
    auto deck = std::get<1>(entity)->Get_Component<Deck_Component>(entity);
    std::ranges::shuffle(deck->deck, get<1>(entity)->ecs.random);
}

void Discard_Deck_Card(Entity entity, Entity_ID card) {
    auto deck = std::get<1>(entity)->Get_Component<Deck_Component>(entity);
    deck->hand.erase(ranges::find(deck->hand, card));
    deck->discard.emplace_back(card);
}

Object_UI* Create_Deck_UI(Entity entity, Game_UI_Manager& game_ui_manager) {
    auto deck = std::get<1>(entity)->Get_Component<Deck_Component>(entity);
    return new Deck_UI(entity, game_ui_manager, deck->player);
}

Entity_Type* Get_Deck_Entity_Type() {
    return new Entity_Type(vector{&Deck_Component::component_type});
}

Component_Type Deck_Component::component_type = Component_Type{"Deck", sizeof(Deck_Component)};
