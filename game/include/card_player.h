#pragma once
#include "deck.h"
#include "path.h"
#include "player.h"

class Card_Player : public Player {
  public:
    ECS* ecs;
    Entity_ID base_id;
    int team;
    int money;
    Entity_ID deck_id;
    vector<Path*> paths;
    Entity active_card;

    Card_Player(Client_ID client_id, Player_ID player_id, int team)
        : Player(client_id, player_id), team(team), money(10), paths(vector<Path*>{}),
          active_card(tuple(nullptr, nullptr)) {}

    Card_Player(Player_ID player_id, int team)
        : Player(player_id), team(team), money(10), paths(vector<Path*>{}) {}

    Deck_Component* Get_Deck() const {
        auto deck = get<0>(ecs->entities_by_id[deck_id]);
        return get<1>(deck)->Get_Component<Deck_Component>(deck);
    }
};