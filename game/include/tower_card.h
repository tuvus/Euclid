#pragma once
#include "card.h"
#include "tower.h"

struct Tower_Card_Component {
    static Component_Type component_type;
    int team;
    bool spawned;
    int reload;
    float range;
};

Entity_ID Init_Tower_Card(Entity entity, Card_Data* card_data,
                          Tower_Card_Component tower_card_component) {
    Init_Card(entity, *card_data);
    auto* tower_card = std::get<1>(entity)->Get_Component<Tower_Card_Component>(
        std::get<0>(entity), &Tower_Card_Component::component_type);
    tower_card->team = tower_card_component.team;
    tower_card->spawned = true;
    tower_card->reload = tower_card_component.reload;
    tower_card->range = tower_card_component.range;
    return Entity_Array::Get_Entity_Data(std::get<0>(entity)).id;
}

Entity_Type* Get_Tower_Card_Entity_Type() {
    return new Entity_Type(vector{&UI_Component::component_type, &Card_Component::component_type,
                                  &Tower_Card_Component::component_type});
}
