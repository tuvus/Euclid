#pragma once
#include "card_player.h"
#include "ecs.h"
#include "game_object.h"

#include <string>

class Game_Scene;
struct Card_Data {
    Texture2D& texture;
    std::string name;
    std::string desc;
    int cost;
};

struct Card_Component {
    static Component_Type component_type;
    Card_Data* card_data;
};

void Init_Card(Entity entity, Card_Data& card_data) {
    auto* card_component = std::get<1>(entity)->Get_Component<Card_Component>(
        std::get<0>(entity), &Card_Component::component_type);
    card_component->card_data = &card_data;
}

bool Can_Play_Card(Entity card, Entity player);

Object_UI* Create_Card_UI(Entity entity, Game_UI_Manager& game_ui_manager);

class Card : public Game_Object {
  public:
    Card_Data& card_data;
    Game_Scene& game_scene;

    Card(Game_Manager& game_manager, Game_Scene& game_scene, Card_Data& card_data);

    virtual Card* Clone() = 0;

    virtual bool Can_Play_Card(Card_Player* card_player, Vector2 pos);

    virtual void Play_Card(Card_Player* card_player, Vector2 pos);

    virtual void Discard_Card(Card_Player* card_player);

    Object_UI* Create_UI_Object(Game_UI_Manager& game_ui_manager) override;
};