#include "card.h"
#include "card_ui.h"

void Init_Card(Entity entity, Card_Data& card_data, Texture2D* texture, float scale, Color color) {
    auto* card_component =
        get<1>(entity)->Get_Component<Card_Component>(entity, &Card_Component::component_type);
    card_component->card_data = &card_data;
    auto* ui = get<1>(entity)->Get_Component<UI_Component>(entity, &UI_Component::component_type);
    ui->texture = texture;
    ui->scale = scale;
    ui->color = color;
}

bool Can_Play_Card(Card_Player* player, Entity entity, Vector2 pos) {
    auto* card =
        get<1>(entity)->Get_Component<Card_Component>(entity, &Card_Component::component_type);
    return player->money >= card->card_data->cost;
}

void Play_Card(Card_Player* player, Entity entity, Vector2 pos) {
    auto* card =
        get<1>(entity)->Get_Component<Card_Component>(entity, &Card_Component::component_type);
    player->money -= card->card_data->cost;
}

void Discard_Card(Card_Player* player, Entity entity) {
    Discard_Deck_Card(player->deck, Entity_Array::Get_Entity_Data(entity).id);
}

Object_UI* Create_Card_UI(Entity entity, Game_UI_Manager& game_ui_manager) {
    return new Card_UI(entity, game_ui_manager);
}

Component_Type Card_Component::component_type = Component_Type{"Card", sizeof(Card_Component)};
