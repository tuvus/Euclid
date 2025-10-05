#include "unit_card.h"
#include "game_scene.h"

void Unit_Card::Play_Card(Card_Player* card_player, Vector2 pos) {
    Card::Play_Card(card_player, pos);
    for (int i = 0; i < unit_count; i++) {
        auto components = vector<Component_Type*>();
        components.emplace_back(&Transform_Component::component_type);
        components.emplace_back(&Unit_Component::component_type);
        auto entity = game_scene.ecs->Create_Entity(new Entity_Type(components));

        game_manager.Add_Object(new Unit(
            game_scene.ecs, game_manager, unit_data, game_scene.Get_Team_Path(card_player->team), 1,
            i * 10, card_player->team, .4f, Game_Scene::Get_Team_Color(card_player->team),
            Entity_Array::Get_Entity_Data(entity).id));
    }
}
