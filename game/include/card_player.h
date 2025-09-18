#pragma once
#include "deck.h"
#include "player.h"

class Card_Player : public Player {
  public:
    int team;
    int money;
    Deck* deck;

    Card_Player(Client_ID client_id, Player_ID player_id, int team)
        : Player(client_id, player_id), team(team), money(10) {}
};