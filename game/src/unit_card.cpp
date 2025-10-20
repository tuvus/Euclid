#include "unit_card.h"

#include "game_scene.h"
using namespace std;

void Play_Card(Entity entity, Card_Player* card_player, Vector2 pos) {
    Play_Card(card_player, entity, pos);
    auto unit_card = get<1>(entity)->Get_Component<Unit_Card_Component>(
        entity, &Unit_Card_Component::component_type);
    for (int i = 0; i < unit_card->unit_count; i++) {
        auto components =
            vector{&Transform_Component::component_type, &Unit_Component::component_type};
        auto unit = get<1>(entity)->ecs.Create_Entity(new Entity_Type(components));
        Init_Unit(&get<1>(entity)->ecs, unit, card_player->path, 1, i * 10, card_player->team, .4f,
                  Game_Scene::Get_Team_Color(card_player->team));
    }
}

Component_Type Unit_Card_Component::component_type =
    Component_Type{"UnitCard", sizeof(Unit_Card_Component)};
