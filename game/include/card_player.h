#pragma once
#include "deck.h"
#include "path.h"
#include "player.h"

class Card_Player : public Player {
  public:
    int team;
    int money;
    Entity deck;
    Path* path;

    Card_Player(Client_ID client_id, Player_ID player_id, int team, Path* path)
        : Player(client_id, player_id), team(team), money(10), path(path) {}

    Card_Player(Player_ID player_id, int team)
        : Player(player_id), team(team), money(10), path(nullptr) {}
};