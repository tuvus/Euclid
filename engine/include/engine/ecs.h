#pragma once
#include "application.h"

#include <algorithm>
#include <cstring>
#include <functional>
#include <memory>
#include <numeric>
#include <raylib.h>
#include <stdexcept>
#include <string.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class Entity_Type_Iterator;
class ECS;
typedef long Component_ID;

typedef long Entity_ID;
struct Entity {
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

    Entity_Type(std::vector<Component_Type*> components);

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

    Entity_Array(const std::vector<Component_Type*>& components);

    static Entity& Get_Entity_Data(unsigned char* entity) {
        return *reinterpret_cast<Entity*>(entity);
    }

    template <typename T>
    T* Get_Component(unsigned char* entity, Component_Type* component_type) {
        entity += sizeof(Entity);
        for (auto& component : entity_type.components) {
            if (component_type == component)
                return reinterpret_cast<T*>(entity);
            entity += component->size;
        }
        throw std::invalid_argument("Failed to find the component_type " + component_type->name +
                                    " for an entity.");
    }

    std::tuple<unsigned char*, int> Create_Entity(ECS* ecs, Entity_ID id);

    void Delete_Entity(ECS* ecs, int index);

    unsigned char* Get_Entity(int index) const {
        return entities + index * entity_type.entity_size;
    }
};

class System {
  public:
    Entity_Type* entity_type;
    std::function<void(ECS* ecs, Entity_Array*, unsigned char*)> function;
};

class Entity_Iterator {
    Entity_Type_Iterator* type_iterator;

  public:
    int pos;
    int index;

    Entity_Iterator(Entity_Type_Iterator*);
    Entity_Iterator(Entity_Type_Iterator*, int pos);
    std::tuple<unsigned char*, Entity_Array*> operator*();
    void operator++();
    bool operator!=(Entity_Iterator) const;
};

class Entity_Type_Iterator {
    Entity_Type* entity_type;
    std::vector<Entity_Array*> arrays;

  public:
    Entity_Type_Iterator(ECS& ecs, Entity_Type* entity_type);
    std::tuple<unsigned char*, Entity_Array*> Get_Entity(int pos, int index);
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
    std::unordered_map<Entity_ID, std::tuple<unsigned char*, int, Entity_Array*>> entities_by_id;
    Entity_ID next_id = 1;

    ECS(Application& application);
    void Update();

    std::tuple<unsigned char*, Entity_Array*> Create_Entity(Entity_Type* entity_type);

    void Delete_Entity(Entity_ID entity_id);

    void Apply_Function_To_Entities(
        Entity_Type* entity_type,
        const std::function<void(ECS* ecs, Entity_Array*, unsigned char*)>& op);

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
