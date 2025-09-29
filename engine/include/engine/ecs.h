#pragma once
#include <algorithm>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

typedef long Component_ID;

class Component_Type {
  public:
    int size;
};

class Entity_Type {
  public:
    std::vector<Component_Type*> components;
    // The size of each entity and its components in bytes
    int entity_size;

    Entity_Type(std::vector<Component_Type*> components) : components(std::move(components)) {
        entity_size =
            std::accumulate(components.begin(), components.end(), 0,
                            [](const int sum, const Component_Type* c) { return sum + c->size; });
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
    void* entities;
    int entity_count;

    Entity_Array(const std::vector<Component_Type*>& components) : entity_type(Entity_Type(components)) {
        entity_count = 10;
        entities = new unsigned char[entity_type.entity_size * entity_count];
    }

    template <typename T>
    T* Get_Component(char* entity, Component_Type* component_type) {
        for (int i = 0; i < components.size(); i++) {
            if (component_type == components[i])
                return static_cast<T*>(entity);
            entity += components[i]->size;
        }
        throw std::invalid_argument("Failed to find a component_type for an entity.");
    }

    void* Create_Entity() {
        entities.emplace_back();
        return entities.back();
    }

    void* Get_Entity(int index) { return entities[index * entity_size]; }
};

class ECS {
    std::unordered_set<Entity_Array*> entity_components;

  public:
    void* Create_Entity(Entity_Type* entity_type) {
        Entity_Type* e_type =
            *std::ranges::find_if(entity_components, [entity_type](Entity_Array* e) {
                return e->entity_type.Is_Entity_Strictly_Of_type(entity_type);
            });
        if (e_type == nullptr) {
            entity_components.emplace(entity_type);
            e_type = entity_type;
        }
        return e_type->Create_Entity();
    }

    void Apply_Function_To_Entities(Entity_Type* entity_type,
                                    const std::function<void(Entity_Array*, void*)>& op) {
        for (const auto& entity_array : entity_components) {
            if (!entity_array->entity_type.Is_Entity_Of_Type(entity_type))
                continue;
            for (int i = 0; i < entity_array->entity_count; i++) {
                op(entity_array, entity_array->Get_Entity(i));
            }
        }
    }
};
