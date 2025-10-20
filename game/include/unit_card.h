#pragma once
#include "card.h"
#include "unit.h"

struct Unit_Card_Component {
    static Component_Type component_type;
    int unit_count;
    Texture2D& unit_texture;
};

Entity_ID Init_Unit_Card(Entity entity, Card_Data* card_data,
                         Unit_Card_Component unit_card_component) {
    Init_Card(entity, *card_data);
    auto* unit_card = std::get<1>(entity)->Get_Component<Unit_Card_Component>(
        entity, &Unit_Card_Component::component_type);
    unit_card->unit_count = unit_card_component.unit_count;
    unit_card->unit_texture = unit_card_component.unit_texture;
    return Entity_Array::Get_Entity_Data(entity).id;
}

inline Entity_Type* Get_Unit_Card_Entity_Type() {
    return new Entity_Type(vector{&UI_Component::component_type, &Card_Component::component_type,
                                  &Unit_Card_Component::component_type});
}
