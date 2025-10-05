#pragma once
#include <algorithm>
#include <cstring>
#include <functional>
#include <numeric>
#include <raylib.h>
#include <stdexcept>
#include <string.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

typedef long Component_ID;

struct Entity {
    // If id = 0, then the entity is not being used
    int id;
    int index;
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

    Entity_Type(std::vector<Component_Type*> components) : components(std::move(components)) {
        entity_size = sizeof(Entity) + std::accumulate(components.begin(), components.end(), 0,
                                                       [](const int sum, const Component_Type* c) {
                                                           return sum + c->size;
                                                       });
    }

    /**
     * Finds if the other entity is a superset of this entity.
     * @return true if the other entity contains all components of this entity, false otherwise.
     */
    bool Is_Entity_Of_Type(Entity_Type* other) const {
        return std::ranges::all_of(components, [other](Component_Type* c) {
            return std::ranges::find(other->components, c) != other->components.end();
        });
    }

    bool Is_Entity_Strictly_Of_type(Entity_Type* other) const {
        if (components.size() != other->components.size())
            return false;
        return Is_Entity_Of_Type(other);
    }
};

// Singleton class that describes how each entity with a certain set of components is laid out.
class Entity_Array {
  public:
    Entity_Type entity_type;
    unsigned char* entities;
    int entity_count;
    int entities_capacity;

    Entity_Array(const std::vector<Component_Type*>& components)
        : entity_type(Entity_Type(components)), entity_count(0) {
        entities_capacity = 100;
        entities = new unsigned char[entity_type.entity_size * entities_capacity];
    }

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
        throw std::invalid_argument("Failed to find a component_type for an entity.");
    }

    unsigned char* Create_Entity(int id) {
        unsigned char* ptr = entities + entity_count * entity_type.entity_size;
        std::memset(ptr, 0, entity_type.entity_size);
        Entity& entity = Get_Entity_Data(ptr);
        entity.id = id;
        entity.index = entity_count;
        entity_count++;
        return ptr;
    }

    void Delete_Entity(int index) {
        std::memcpy(entities + index * entity_type.entity_size,
                    entities + entity_count * (entity_type.entity_size - 1),
                    entity_type.entity_size);
        entity_count--;
        Get_Entity_Data(Get_Entity(index)).index = index;
    }

    unsigned char* Get_Entity(int index) const {
        return entities + index * entity_type.entity_size;
    }
};

class ECS {
    std::unordered_set<Entity_Array*> entity_components;

  public:
    ECS() { entity_components = std::unordered_set<Entity_Array*>(); }
    int next_id = 1;

    std::tuple<unsigned char*, Entity_Array&> Create_Entity(Entity_Type* entity_type) {
        auto search = std::ranges::find_if(entity_components, [entity_type](Entity_Array* e) {
            return e->entity_type.Is_Entity_Strictly_Of_type(entity_type);
        });
        Entity_Array* e_array;
        if (search == entity_components.end()) {
            e_array = new Entity_Array(entity_type->components);
            entity_components.emplace(e_array);
        } else {
            e_array = *search;
        }
        return std::tuple<unsigned char*, Entity_Array&>(e_array->Create_Entity(next_id++),
                                                         *e_array);
    }

    void Apply_Function_To_Entities(
        Entity_Type* entity_type,
        const std::function<void(ECS* ecs, Entity_Array*, unsigned char*)>& op) {
        for (const auto& entity_array : entity_components) {
            if (!entity_array->entity_type.Is_Entity_Of_Type(entity_type))
                continue;
            for (int i = 0; i < entity_array->entity_count; i++) {
                unsigned char* entity = entity_array->Get_Entity(i);
                if (Entity_Array::Get_Entity_Data(entity).id != 0)
                    op(this, entity_array, entity_array->Get_Entity(i));
            }
        }
    }

    Entity_Array* Get_Entities_Of_Exact_Type(Entity_Type* entity_type) {
        auto e_array = *std::ranges::find_if(entity_components, [entity_type](Entity_Array* e) {
            return e->entity_type.Is_Entity_Strictly_Of_type(entity_type);
        });
        return e_array;
    }
};

struct Transform_Component {
    Vector2 pos;
    float rot;
    float scale;

  public:
    static Component_Type component_type;
};
