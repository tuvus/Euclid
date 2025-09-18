#include "game_object.h"
#include "game_manager.h"

Game_Object::Game_Object(Game_Manager& game_manager, const Vector2 pos, const float rot,
                         float scale, Color color)
    : id(game_manager.Get_New_Id()), pos(pos), rot(rot), game_manager(game_manager), scale(scale),
      color(color) {
}

void Game_Object::Delete_Object() {
    game_manager.Delete_Object(this);
}

Game_Object::~Game_Object() {
}

long Game_Object::Get_Id() const {
    return id;
}
