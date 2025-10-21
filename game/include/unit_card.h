#pragma once
#include "card.h"

struct Unit_Card_Component {
    static Component_Type component_type;
    int unit_count;
    Texture2D* unit_texture;
};

Entity_ID Init_Unit_Card(Entity entity, Card_Data* card_data,
                         Unit_Card_Component unit_card_component);

Entity_Type* Get_Unit_Card_Entity_Type();
