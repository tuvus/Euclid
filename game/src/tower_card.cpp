#include "tower_card.h"

#include "game_scene.h"

#include <raymath.h>

using namespace std;

bool Can_Play_Tower_Card(Card_Player* card_player, Entity entity, Vector2 pos) {
    return Can_Play_Card(card_player, entity, pos) &&
           Can_Place_Tower(entity, card_player->path, pos, 50);
}

bool Can_Place_Tower(Entity tower_card, Path* path, Vector2 pos, float min_dist) {
    for (auto entity : get<1>(tower_card)->ecs.Get_Entities_Of_Type(Get_Tower_Card_Entity_Type())) {
        auto transform = get<1>(entity)->Get_Component<Transform_Component>(
            entity, &Transform_Component::component_type);
        if (Vector2Distance(pos, transform->pos) <= min_dist)
            return false;
    }
    for (auto position : path->positions) {
        if (Vector2Distance(pos, position) <= min_dist)
            return false;
    }
    return true;
}

void Play_Tower_Card(Card_Player* player, Entity entity, Vector2 pos) {
    Play_Card(player, entity, pos);
    auto tower = get<1>(entity)->ecs.Create_Entity(Get_Tower_Entity_Type());
    Init_Tower(tower, Vector2(pos.x, pos.y), 150, player->team, .4f,
               Game_Scene::Get_Team_Color(player->team));
}

Component_Type Tower_Card_Component::component_type =
    Component_Type{"TowerCard", sizeof(Tower_Card_Component)};
