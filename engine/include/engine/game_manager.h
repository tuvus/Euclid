#pragma once
#include "application.h"
#include "game_object.h"

#include <random>

class Player;
typedef long Player_ID;

class Game_Manager {
  public:
    Application& application;
    Network& network;

  private:
    /* The current count of the step */
    long step = 0;
    /* The maximum step that we are allowed to simulate to */
    long max_step = 0;
    /* The lowest step that a client is on.
     * Used by the server to determine if it should continue or wait. */
    long min_step = 0;
    /* Used on the server to store what steps */
    std::unordered_map<Player_ID, long> player_steps;
    /* Called on clients from the server to set the max step */
    void On_Receive_Step_Update(long max_step);
    /* Called on the server from the client to inform the server on its current step */
    void On_Receive_Player_Step_Update(Player_ID player_id, long min_step);
    long next_id = 0;

    unordered_map<Obj_ID, Game_Object*> objects;
    vector<Game_Object*> objects_to_delete;

  public:
    Player* local_player;
    vector<Player*> players;
    std::function<void(Game_Object*)> on_add_object;
    std::function<void(Game_Object*)> on_delete_object;

    Game_Manager(Application&, Network&, vector<Player*>, Player*, long seed);
    ~Game_Manager();
    void Update();
    void Add_Object(Game_Object* object);
    void Delete_Object(Game_Object* object);
    long Get_New_Id();
    long Get_Current_Step() const;
    Game_Object* Get_Object(Obj_ID);
    Player* Get_Player(Player_ID);
    vector<Game_Object*> Get_All_Objects();

    std::minstd_rand random;
};
