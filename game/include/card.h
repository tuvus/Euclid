#pragma once
#include "card_player.h"
#include "ecs.h"

#include <string>

class Game_Scene;
struct Card_Data {
    Texture2D& texture;
    std::string name;
    std::string desc;
    int cost;
    function<bool(Card_Player*, Entity, Vector2)> can_play_card;
    function<void(Card_Player*, Entity, Vector2)> play_card;
    function<void(Card_Player*, Entity)> discard_card;
};

struct Card_Component {
    static Component_Type component_type;
    Card_Data* card_data;
};

void Init_Card(Entity entity, Card_Data& card_data) {
    auto* card_component =
        std::get<1>(entity)->Get_Component<Card_Component>(entity, &Card_Component::component_type);
    card_component->card_data = &card_data;
}

bool Can_Play_Card(Card_Player*, Entity, Vector2);

void Play_Card(Card_Player*, Entity, Vector2);

void Discard_Card(Card_Player*, Entity);

Object_UI* Create_Card_UI(Entity entity, Game_UI_Manager& game_ui_manager);
