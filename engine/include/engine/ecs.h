#pragma once
#include "application.h"

#include <cstring>
#include <functional>
#include <memory>
#include <random>
#include <raylib.h>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class ECS_Worker;
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
    std::string name;
    std::vector<Component_Type*> components;
    // The size of each entity and its components in bytes
    int entity_size;
    std::function<Object_UI*(Entity, Game_UI_Manager&)> ui_creation_function;
    std::function<void(Entity)> setup_function;
    std::function<void(Entity)> delete_function;

    Entity_Type(std::vector<Component_Type*> components);

    Entity_Type(std::vector<Component_Type*> components, std::string name,
                std::function<Object_UI*(Entity, Game_UI_Manager&)> ui_creation_function,
                std::function<void(Entity)> setup_function,
                std::function<void(Entity)> delete_function);

    /**
     * Finds if the other entity is a superset of this entity.
     * @return true if the other entity contains all components of this entity, false otherwise.
     */
    bool Is_Entity_Of_Type(Entity_Type* other) const;

    bool Is_Entity_Strictly_Of_type(Entity_Type* other) const;
};

// Singleton class that describes how each entity with a certain set of components is laid out.
class Entity_Array {
    struct Dynamic_Array {
        unsigned char* entities;
        int entity_count;
        int entity_capacity;
        Dynamic_Array* next;
    };
    Dynamic_Array array;
    pthread_mutex_t array_lock;
    // This is the count of the entities in the array at the start of the block
    int entity_count;

  public:
    Entity_Type entity_type;
    ECS& ecs;

    Entity_Array(ECS& ecs, Entity_Type entity_type);

    static Entity_Component& Get_Entity_Data(Entity entity) {
        return *reinterpret_cast<Entity_Component*>(std::get<0>(entity));
    }

    static Entity_ID Get_Entity_ID(Entity entity) { return Get_Entity_Data(entity).id; }

    template <typename T>
    T* Get_Component(Entity entity_data) {
        Component_Type* component_type = &T::component_type;
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

    std::tuple<unsigned char*, int> Create_Entity(ECS* ecs);

    /**
     * Copies all components from src_index to dst_index, does not include the entities id.
     */
    void Copy_Entity(int src_index, int dst_index);

    void Delete_Entity(ECS* ecs, int index);

    Entity Get_Entity(int index);

    /**
     * Cleans up and merges any entity_arrays.
     */
    void Clean_Up();

    inline int Count() const { return entity_count; }
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

struct Work_Data {
    const std::function<void(ECS* ecs, Entity entity)>& op;
    Entity_Array* entity_array;
    int starting_index;
    int ending_index;
    Work_Data* next;
};

class ECS {
    // Stores a list of the creator id, the entity array and the index of the newly created entity.
    std::vector<tuple<Entity_ID, Entity_Array*, int>> to_create;
    pthread_mutex_t to_create_mutex;
    std::vector<Entity_ID> to_delete;
    std::vector<ECS_Worker*> workers;
    ECS_Worker* main_thread;
    pthread_mutex_t work_mutex;
    Work_Data* work_start;
    Work_Data* work_end;
    bool in_block;

    void Complete_Work();

  public:
    Application& application;
    std::unordered_set<Entity_Array*> entity_arrays;
    std::vector<std::vector<System*>> blocks;
    std::unordered_map<Entity_ID, std::tuple<Entity, int>> entities_by_id;
    Entity_ID next_id = 1;
    std::function<void(Entity_ID)> on_add_entity;
    std::function<void(Entity_ID)> on_delete_entity;
    std::minstd_rand random;

    ECS(Application& application, long seed);
    ~ECS();

    void Update();

    Entity_Array*
    Create_Entity_Type(std::vector<Component_Type*> components, string name,
                       std::function<Object_UI*(Entity, Game_UI_Manager&)> ui_creation_function,
                       std::function<void(Entity)> setup_function = nullptr,
                       std::function<void(Entity)> delete_function = nullptr);

    Entity Create_Entity(Entity_Type* entity_type, Entity_ID creator_id);

    Entity Copy_Entity(Entity_ID to_copy_id, Entity_ID creator_id);

    /**
     * Schedules an Entity for deletion after the next block.
     * Does not accept zero or negative values for the entity_id.
     */
    void Delete_Entity(Entity_ID entity_id);

    /**
     * Schedules an Entity for deletion after the next block.
     * This version of Delete_Entity can handle entities that
     * haven't been assigned an ID yet and are being deleted
     * right after they have been created
     */
    void Delete_Entity(Entity entity);

    void Apply_Function_To_Entities(Entity_Type* entity_type,
                                    const std::function<void(ECS* ecs, Entity)>& op);

    Entity_Type_Iterator Get_Entities_Of_Type(Entity_Type* entity_type);

    Entity_Array* Get_Entities_Of_Exact_Type(Entity_Type* entity_type);

    void Register_System(System* system, int block_index);
    Work_Data* Get_Work(atomic_bool& atomic_bool);
    bool In_Block() const { return in_block; }
};

class ECS_Worker {
    pthread_t thread;
    ECS& ecs;
    atomic_bool doing_work;
    atomic_bool canceled;

  public:
    ECS_Worker(ECS& ecs, bool separate_thread = true);
    void Do_Worker_Loop();
    inline void Do_Work();
    ~ECS_Worker();
    constexpr bool Doing_Work();
};

struct Transform_Component {
    Vector2 pos;
    float rot;
    float scale;

  public:
    static Component_Type component_type;
};
