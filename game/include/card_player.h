#pragma once
#include "deck.h"
#include "path.h"
#include "player.h"

class Card_Player : public Player {
  public:
    int team;
    int money;
    Entity deck;
    vector<Path*> paths;
    Entity active_card;

    Card_Player(Client_ID client_id, Player_ID player_id, int team)
        : Player(client_id, player_id), team(team), money(10), paths(vector<Path*>{}),
          active_card(tuple(nullptr, nullptr)) {}

    Card_Player(Player_ID player_id, int team)
        : Player(player_id), team(team), money(10), paths(vector<Path*>{}) {}

    Deck_Component* Get_Deck() {
        return get<1>(deck)->Get_Component<Deck_Component>(deck, &Deck_Component::component_type);
    }
};