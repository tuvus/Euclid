#pragma once
#include "card.h"
#include "tower.h"

struct Tower_Card_Component {
    static Component_Type component_type;
    Tower_Data* tower_data;
};

Entity_ID Init_Tower_Card(Entity entity, Card_Data* card_data, Tower_Data* tower_data) {
    Init_Card(entity, *card_data);
    auto* tower_card = std::get<1>(entity)->Get_Component<Tower_Card_Component>(
        std::get<0>(entity), &Tower_Card_Component::component_type);
    tower_card->tower_data = tower_data;
    return Entity_Array::Get_Entity_Data(std::get<0>(entity)).id;
}

Entity_Type* Get_Tower_Card_Entity_Type() {
    return new Entity_Type(
        vector{&Card_Component::component_type, &Tower_Card_Component::component_type});
}

class Tower_Card : public Card {
  public:
    Tower_Data& tower_data;

    Tower_Card(Game_Manager& game_manager, Game_Scene& game_scene, Card_Data& card_data,
               Tower_Data& tower_data);

    Card* Clone() override;

    bool Can_Play_Card(Card_Player* card_player, Vector2 pos) override;

    void Play_Card(Card_Player* player, Vector2 pos) override;

    void Update() override {}
};