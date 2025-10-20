#pragma once
#include "application.h"

#include <cstring>
#include <functional>
#include <memory>
#include <numeric>
#include <random>
#include <raylib.h>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class Game_UI_Manager;
class Object_UI;
class Entity_Array;
class Entity_Type_Iterator;
class ECS;
typedef long Component_ID;

typedef long Entity_ID;
typedef std::tuple<unsigned char*, Entity_Array*> Entity;
struct Entity_Component {
    // If id = 0, then the entity is not being used
    Entity_ID id;
};

class Component_Type {
  public:
    std::string name;
    int size;
};

class Entity_Type {
  public:
    std::vector<Component_Type*> components;
    // The size of each entity and its components in bytes
    int entity_size;
    std::function<Object_UI*(Entity, Game_UI_Manager&)> ui_creation_function;

    Entity_Type(std::vector<Component_Type*> components);

    Entity_Type(std::vector<Component_Type*> components,
                std::function<Object_UI*(Entity, Game_UI_Manager&)> ui_creation_function);

    /**
     * Finds if the other entity is a superset of this entity.
     * @return true if the other entity contains all components of this entity, false otherwise.
     */
    bool Is_Entity_Of_Type(Entity_Type* other) const;

    bool Is_Entity_Strictly_Of_type(Entity_Type* other) const;
};

// Singleton class that describes how each entity with a certain set of components is laid out.
class Entity_Array {
  public:
    Entity_Type entity_type;
    unsigned char* entities;
    int entity_count;
    int entity_capacity;
    ECS& ecs;

    Entity_Array(ECS& ecs, const std::vector<Component_Type*>& components);

    static Entity_Component& Get_Entity_Data(Entity entity) {
        return *reinterpret_cast<Entity_Component*>(std::get<0>(entity));
    }

    static Entity_ID Get_Entity_ID(Entity entity) { return Get_Entity_Data(entity).id; }

    template <typename T>
    T* Get_Component(Entity entity_data, Component_Type* component_type) {
        unsigned char* entity = std::get<0>(entity_data);
        entity += sizeof(Entity_Component);
        for (auto& component : entity_type.components) {
            if (component_type == component)
                return reinterpret_cast<T*>(entity);
            entity += component->size;
        }
        throw std::invalid_argument("Failed to find the component_type " + component_type->name +
                                    " for an entity.");
    }

    std::tuple<unsigned char*, int> Create_Entity(ECS* ecs, Entity_ID id);

    /**
     * Copies all components from src_index to dst_index, does not include the entities id.
     */
    void Copy_Entity(int src_index, int dst_index);

    void Delete_Entity(ECS* ecs, int index);

    Entity Get_Entity(int index) { return tuple(entities + index * entity_type.entity_size, this); }
};

class System {
  public:
    Entity_Type* entity_type;
    std::function<void(ECS* ecs, Entity entity)> function;
};

class Entity_Iterator {
    Entity_Type_Iterator* type_iterator;

  public:
    int pos;
    int index;

    Entity_Iterator(Entity_Type_Iterator*);
    Entity_Iterator(Entity_Type_Iterator*, int pos);
    Entity operator*();
    void operator++();
    bool operator!=(Entity_Iterator) const;
};

class Entity_Type_Iterator {
    Entity_Type* entity_type;
    std::vector<Entity_Array*> arrays;

  public:
    Entity_Type_Iterator(ECS& ecs, Entity_Type* entity_type);
    Entity Get_Entity(int pos, int index);
    std::tuple<int, int> Next_Entity(int pos, int index);
    bool Has_Next_Entity(int pos, int index);
    Entity_Iterator begin();
    Entity_Iterator end();
};

class ECS {
    std::vector<Entity_ID> to_delete;

  public:
    Application& application;
    std::unordered_set<Entity_Array*> entity_components;
    std::unordered_set<System*> systems;
    std::unordered_map<Entity_ID, std::tuple<Entity, int>> entities_by_id;
    Entity_ID next_id = 1;
    std::function<void(Entity_ID)> on_add_entity;
    std::function<void(Entity_ID)> on_delete_entity;
    std::minstd_rand random;

    ECS(Application& application, long seed);

    void Update();

    Entity_Type*
    Create_Entity_Type(std::vector<Component_Type*> components,
                       std::function<Object_UI*(Entity, Game_UI_Manager&)> ui_creation_function);

    Entity Create_Entity(Entity_Type* entity_type);

    Entity Copy_Entity(Entity_ID entity_id);

    void Delete_Entity(Entity_ID entity_id);

    void Apply_Function_To_Entities(Entity_Type* entity_type,
                                    const std::function<void(ECS* ecs, Entity)>& op);

    Entity_Type_Iterator Get_Entities_Of_Type(Entity_Type* entity_type);

    Entity_Array* Get_Entities_Of_Exact_Type(Entity_Type* entity_type);

    void Register_System(System* system) { systems.emplace(system); }
};

struct Transform_Component {
    Vector2 pos;
    float rot;
    float scale;

  public:
    static Component_Type component_type;
};
