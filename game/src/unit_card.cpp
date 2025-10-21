#include "unit_card.h"

#include "game_scene.h"
#include "unit.h"
using namespace std;

Entity_ID Init_Unit_Card(Entity entity, Card_Data* card_data,
                         Unit_Card_Component unit_card_component) {
    Init_Card(entity, *card_data);
    auto* unit_card = std::get<1>(entity)->Get_Component<Unit_Card_Component>(
        entity, &Unit_Card_Component::component_type);
    unit_card->unit_count = unit_card_component.unit_count;
    unit_card->unit_texture = unit_card_component.unit_texture;
    return Entity_Array::Get_Entity_Data(entity).id;
}

void Play_Card(Entity entity, Card_Player* card_player, Vector2 pos) {
    Play_Card(card_player, entity, pos);
    auto unit_card = get<1>(entity)->Get_Component<Unit_Card_Component>(
        entity, &Unit_Card_Component::component_type);
    for (int i = 0; i < unit_card->unit_count; i++) {
        auto components =
            vector{&Transform_Component::component_type, &Unit_Component::component_type};
        auto unit = get<1>(entity)->ecs.Create_Entity(new Entity_Type(components));
        Init_Unit(&get<1>(entity)->ecs, unit, card_player->path, 1, i * 10, card_player->team,
                  unit_card->unit_texture, .4f, Game_Scene::Get_Team_Color(card_player->team));
    }
}

Entity_Type* Get_Unit_Card_Entity_Type() {
    return new Entity_Type(vector{&UI_Component::component_type, &Card_Component::component_type,
                                  &Unit_Card_Component::component_type});
}

Component_Type Unit_Card_Component::component_type =
    Component_Type{"UnitCard", sizeof(Unit_Card_Component)};
