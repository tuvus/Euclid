#include "tower_card.h"

#include "game_scene.h"
#include "tower.h"

#include <raymath.h>

using namespace std;

Entity_ID Init_Tower_Card(Entity entity, Card_Data* card_data,
                          Tower_Card_Component tower_card_component, Texture2D* texture,
                          float scale, Color color) {
    Init_Card(entity, *card_data, texture, scale, color);
    auto* tower_card = std::get<1>(entity)->Get_Component<Tower_Card_Component>(
        entity, &Tower_Card_Component::component_type);
    tower_card->tower_texture = tower_card_component.tower_texture;
    tower_card->team = tower_card_component.team;
    tower_card->spawned = true;
    tower_card->reload = tower_card_component.reload;
    tower_card->damage = tower_card_component.damage;
    tower_card->range = tower_card_component.range;
    return Entity_Array::Get_Entity_Data(entity).id;
}

bool Can_Play_Tower_Card(Card_Player* card_player, Entity entity, Vector2 pos) {
    return Can_Play_Card(card_player, entity, pos) &&
           Can_Place_Tower(entity, card_player->paths, pos, 50);
}

bool Can_Place_Tower(Entity tower_card, vector<Path*> paths, Vector2 pos, float min_dist) {
    for (auto path : paths) {
        for (auto entity : get<1>(tower_card)->ecs.Get_Entities_Of_Type(Get_Tower_Entity_Type())) {
            auto transform = get<1>(entity)->Get_Component<Transform_Component>(
                entity, &Transform_Component::component_type);
            if (Vector2Distance(pos, transform->pos) <= min_dist)
                return false;
        }
        for (auto position : path->positions) {
            if (Vector2Distance(pos, position) <= min_dist)
                return false;
        }
    }
    return true;
}

void Play_Tower_Card(Card_Player* player, Entity entity, Vector2 pos) {
    Play_Card(player, entity, pos);
    auto* tower_card = std::get<1>(entity)->Get_Component<Tower_Card_Component>(
        entity, &Tower_Card_Component::component_type);
    auto tower = get<1>(entity)->ecs.Create_Entity(Get_Tower_Entity_Type());
    Init_Tower(tower, Vector2(pos.x, pos.y), tower_card->reload, tower_card->range,
               tower_card->damage, player->team, tower_card->tower_texture, .4f,
               Game_Scene::Get_Team_Color(player->team));
}

Entity_Type* Get_Tower_Card_Entity_Type() {
    return new Entity_Type(vector{&UI_Component::component_type, &Card_Component::component_type,
                                  &Tower_Card_Component::component_type});
}

Component_Type Tower_Card_Component::component_type =
    Component_Type{"TowerCard", sizeof(Tower_Card_Component)};
