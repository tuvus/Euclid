#include "ecs.h"
#include "game_ui_manager.h"

#include <thread>
using namespace std;

Entity_Type::Entity_Type(std::vector<Component_Type*> components)
    : Entity_Type(components, nullptr) {
}

Entity_Type::Entity_Type(std::vector<Component_Type*> components,
                         std::function<Object_UI*(Entity, Game_UI_Manager&)> ui_creation_function)
    : components(std::move(components)), ui_creation_function(ui_creation_function) {
    entity_size =
        std::accumulate(this->components.begin(), this->components.end(), sizeof(Entity_Component),
                        [](const int sum, const Component_Type* c) { return sum + c->size; });
}

bool Entity_Type::Is_Entity_Of_Type(Entity_Type* other) const {
    return std::ranges::all_of(components, [other](Component_Type* c) {
        return std::ranges::find(other->components, c) != other->components.end();
    });
}

bool Entity_Type::Is_Entity_Strictly_Of_type(Entity_Type* other) const {
    if (components.size() != other->components.size())
        return false;
    return Is_Entity_Of_Type(other);
}

Entity_Array::Entity_Array(ECS& ecs, Entity_Type entity_type)
    : ecs(ecs), entity_type(Entity_Type(entity_type)), entity_count(0) {
    entity_capacity = 10;
    entities = new unsigned char[entity_type.entity_size * entity_capacity];
}

std::tuple<unsigned char*, int> Entity_Array::Create_Entity(ECS* ecs, Entity_ID id) {
    if (entity_count == entity_capacity) {
        unsigned char* new_entities =
            new unsigned char[entity_type.entity_size * entity_capacity * 2];
        memcpy(new_entities, entities, entity_count * entity_type.entity_size);
        delete entities;
        entities = new_entities;
        for (int i = 0; i < entity_count; i++) {
            Entity entity = Get_Entity(i);
            Entity_ID e_id = Get_Entity_Data(entity).id;
            ecs->entities_by_id[e_id] = tuple(entity, i);
        }
        entity_capacity *= 2;
    }
    unsigned char* ptr = entities + entity_count * entity_type.entity_size;
    std::memset(ptr, 0, entity_type.entity_size);
    Entity_Component& entity = Get_Entity_Data(tuple(ptr, this));
    entity.id = id;
    entity_count++;
    return std::tuple(ptr, entity_count - 1);
}

void Entity_Array::Copy_Entity(int src_index, int dst_index) {
    memcpy(entities + dst_index * entity_type.entity_size + sizeof(Entity_Component),
           entities + src_index * entity_type.entity_size + sizeof(Entity_Component),
           entity_type.entity_size - sizeof(Entity_Component));
}

void Entity_Array::Delete_Entity(ECS* ecs, int index) {
    if (index != entity_count - 1) {
        std::memcpy(entities + index * entity_type.entity_size,
                    entities + (entity_count - 1) * entity_type.entity_size,
                    entity_type.entity_size);
        Entity_ID entity_id = Get_Entity_Data(Get_Entity(index)).id;
        ecs->entities_by_id[entity_id] = tuple(Get_Entity(index), index);
    } else {
        Get_Entity_Data(Get_Entity(index)).id = 0;
    }
    entity_count--;
}

Entity_Iterator::Entity_Iterator(Entity_Type_Iterator* type_iterator)
    : type_iterator(type_iterator), pos(0), index(0) {
}

Entity_Iterator::Entity_Iterator(Entity_Type_Iterator* type_iterator, int pos)
    : type_iterator(type_iterator), pos(pos), index(0) {
}

Entity Entity_Iterator::operator*() {
    return type_iterator->Get_Entity(pos, index);
}

void Entity_Iterator::operator++() {
    auto [pos, index] = type_iterator->Next_Entity(this->pos, this->index);
    this->pos = pos;
    this->index = index;
}

bool Entity_Iterator::operator!=(Entity_Iterator other) const {
    return this->pos != other.pos || this->index != other.index;
}

Entity_Type_Iterator::Entity_Type_Iterator(ECS& ecs, Entity_Type* entity_type)
    : entity_type(entity_type) {
    arrays = vector<Entity_Array*>();
    for (auto entity_array : ecs.entity_components) {
        if (entity_array->entity_type.Is_Entity_Of_Type(entity_type) &&
            entity_array->entity_count > 0)
            arrays.emplace_back(entity_array);
    }
}

Entity Entity_Type_Iterator::Get_Entity(int pos, int index) {
    return arrays[pos]->Get_Entity(index);
}

std::tuple<int, int> Entity_Type_Iterator::Next_Entity(int pos, int index) {
    index++;
    if (arrays[pos]->entity_count == index) {
        pos++;
        index = 0;
    }
    return make_tuple(pos, index);
}

bool Entity_Type_Iterator::Has_Next_Entity(int pos, int index) {
    return get<0>(Next_Entity(pos, index)) != arrays.size();
}

Entity_Iterator Entity_Type_Iterator::begin() {
    return Entity_Iterator(this);
}

Entity_Iterator Entity_Type_Iterator::end() {
    return Entity_Iterator(this, arrays.size());
}

ECS::ECS(Application& application, long seed)
    : application(application), work_start(nullptr), work_end(nullptr) {
    entity_components = std::unordered_set<Entity_Array*>();
    systems = std::unordered_set<System*>();
    entities_by_id = std::unordered_map<Entity_ID, std::tuple<Entity, int>>();
    to_delete = vector<Entity_ID>();
    random = std::minstd_rand(seed);
    workers = vector<ECS_Worker*>();
    pthread_mutex_init(&work_mutex, nullptr);
    for (int i = 0; i < 32; i++) {
        workers.emplace_back(new ECS_Worker(*this));
    }
}

ECS::~ECS() {
    for (auto worker : workers) {
        delete worker;
    }
}

void ECS::Update() {
    for (Entity_ID entity_id : to_create) {
        if (on_add_entity != nullptr)
            on_add_entity(entity_id);
    }
    to_create.clear();
    for (auto system : systems) {
        Apply_Function_To_Entities(system->entity_type, system->function);
    }
    Wait_Until_Work_Is_Complete();
    for (Entity_ID entity_id : to_delete) {
        if (!entities_by_id.contains(entity_id))
            continue;
        auto [entity, index] = entities_by_id[entity_id];
        entities_by_id.erase(entity_id);
        get<1>(entity)->Delete_Entity(this, index);
        if (on_delete_entity)
            on_delete_entity(entity_id);
    }
    to_delete.clear();
}

Entity_Array*
ECS::Create_Entity_Type(std::vector<Component_Type*> components,
                        std::function<Object_UI*(Entity, Game_UI_Manager&)> ui_creation_function) {
    auto* new_array = new Entity_Array(
        *this, Entity_Type(std::move(components), std::move(ui_creation_function)));
    entity_components.emplace(new_array);
    return new_array;
}

Entity ECS::Create_Entity(Entity_Type* entity_type) {
    auto search = std::ranges::find_if(entity_components, [entity_type](Entity_Array* e) {
        return e->entity_type.Is_Entity_Strictly_Of_type(entity_type);
    });
    Entity_Array* e_array;
    if (search == entity_components.end()) {
        e_array = new Entity_Array(*this, entity_type->components);
        entity_components.emplace(e_array);
    } else {
        e_array = *search;
    }
    Entity_ID id = next_id++;
    auto [entity, index] = e_array->Create_Entity(this, id);
    entities_by_id.emplace(id, tuple(tuple(entity, e_array), index));
    to_create.emplace(id);
    return tuple(entity, e_array);
}

Entity ECS::Copy_Entity(Entity_ID entity_id) {
    auto [old_entity, old_entity_index] = entities_by_id[entity_id];
    Entity_ID id = next_id++;
    auto [new_entity, new_entity_index] = get<1>(old_entity)->Create_Entity(this, id);
    get<1>(old_entity)->Copy_Entity(old_entity_index, new_entity_index);
    entities_by_id.emplace(id, tuple(tuple(new_entity, get<1>(old_entity)), new_entity_index));
    to_create.emplace(id);
    return make_tuple(new_entity, get<1>(old_entity));
}

void ECS::Delete_Entity(Entity_ID entity_id) {
    if (to_create.contains(entity_id))
        to_create.erase(entity_id);
    to_delete.emplace_back(entity_id);
}

void ECS::Apply_Function_To_Entities(Entity_Type* entity_type,
                                     const std::function<void(ECS* ecs, Entity entity)>& op) {
    for (auto entity_array : entity_components) {
        if (!entity_array->entity_type.Is_Entity_Of_Type(entity_type) ||
            entity_array->entity_count == 0)
            continue;
        int start_index = 0;
        int end_index = min(9, entity_array->entity_count - 1);
        pthread_mutex_lock(&work_mutex);
        while (start_index <= end_index) {
            auto work = new Work_Data{op, entity_array, start_index, end_index, nullptr};
            if (work_end == nullptr) {
                work_start = work;
            } else {
                work_end->next = work;
            }
            work_end = work;
            start_index = end_index + 1;
            end_index = min(end_index + 10, entity_array->entity_count);
        }
        pthread_mutex_unlock(&work_mutex);
    }
}

Entity_Type_Iterator ECS::Get_Entities_Of_Type(Entity_Type* entity_type) {
    return Entity_Type_Iterator(*this, entity_type);
}

Entity_Array* ECS::Get_Entities_Of_Exact_Type(Entity_Type* entity_type) {
    auto e_array = *std::ranges::find_if(entity_components, [entity_type](Entity_Array* e) {
        return e->entity_type.Is_Entity_Strictly_Of_type(entity_type);
    });
    return e_array;
}

Work_Data* ECS::Get_Work() {
    Work_Data* data = nullptr;
    pthread_mutex_lock(&work_mutex);
    data = work_start;
    if (data != nullptr) {
        work_start = data->next;
        if (work_start == nullptr)
            work_end = nullptr;
    }
    pthread_mutex_unlock(&work_mutex);
    return data;
}

void ECS::Wait_Until_Work_Is_Complete() {
    while (true) {
        pthread_mutex_lock(&work_mutex);
        if (work_end == nullptr) {
        }
        pthread_mutex_unlock(&work_mutex);
        this_thread::yield();
    }
}

void* Worker_Function(void* worker) {
    static_cast<ECS_Worker*>(worker)->DoWork();
    return nullptr;
}

ECS_Worker::ECS_Worker(ECS& ecs) : ecs(ecs), canceled(false) {
    pthread_create(&thread, nullptr, Worker_Function, this);
}

void ECS_Worker::DoWork() {
    while (!canceled) {
        auto work = ecs.Get_Work();
        if (work == nullptr) {
            this_thread::yield();
            continue;
        }
        for (int i = work->starting_index; i <= work->ending_index; i++) {
            work->op(&ecs, work->entity_array->Get_Entity(i));
        }
    }
}

ECS_Worker::~ECS_Worker() {
    canceled = true;
    pthread_cancel(thread);
}

Component_Type Transform_Component::component_type =
    Component_Type{"Transform", sizeof(Transform_Component)};
