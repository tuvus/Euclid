#pragma once
#include "card.h"
#include "unit.h"

struct Unit_Card_Component {
    static Component_Type component_type;
    Unit_Data* unit_data;
};

Entity_ID Init_Unit_Card(Entity entity, Card_Data* card_data, Unit_Data* unit_data) {
    Init_Card(entity, *card_data);
    auto* unit_card = std::get<1>(entity)->Get_Component<Unit_Card_Component>(
        std::get<0>(entity), &Unit_Card_Component::component_type);
    unit_card->unit_data = unit_data;
    return Entity_Array::Get_Entity_Data(std::get<0>(entity)).id;
}

Entity_Type* Get_Unit_Card_Entity_Type() {
    return new Entity_Type(
        vector{&Card_Component::component_type, &Unit_Card_Component::component_type});
}

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

    void Play_Card(Card_Player* card_player, Vector2 pos) override;

    void Update() override {}
};