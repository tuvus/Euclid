#pragma once
#include <algorithm>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

typedef long Component_ID;

class Component_Type {
  public:
    int size;
};

// Singleton class that describes how each entity with a certain set of components is laid out.
class Entity_Type {
    std::vector<Component_Type*> components;

  public:
    std::vector<void*> entities;
    int entity_size;

    Entity_Type(std::vector<Component_Type*> components) : components(components) {
        entity_size =
            std::accumulate(components.begin(), components.end(), 0,
                            [](const int sum, const Component_Type* c) { return sum + c->size; });
        entities = std::vector<void*>();
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

    void* Get_Entity(int index) { return entities[index * entity_size]; }

    /**
     * Finds if the other entity is a superset of this entity.
     * @return true if the other entity contains all components of this entity, false otherwise.
     */
    bool Is_Entity_Of_Type(Entity_Type* other) const {
        return std::ranges::all_of(components, [other](Component_Type* c) {
            return std::ranges::find(other->components, c) != other->components.end();
        });
    }
};

class ECS {
    std::unordered_set<Entity_Type*> entity_components;

  public:
    void Apply_Function_To_Entities(Entity_Type* entity_type,
                                    const std::function<void(Entity_Type*, void*)>& op) {
        for (const auto& entity_component : entity_components) {
            if (!entity_component->Is_Entity_Of_Type(entity_type))
                continue;
            for (const auto& entity : entity_component->entities) {
                op(entity_component, entity);
            }
        }
    }
};
