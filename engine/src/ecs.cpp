#include "ecs.h"
#include "game_ui_manager.h"

#include <thread>
using namespace std;

Entity_Type::Entity_Type(std::vector<Component_Type*> components)
    : Entity_Type(components, nullptr, "") {
}

Entity_Type::Entity_Type(std::vector<Component_Type*> components,
                         std::function<Object_UI*(Entity, Game_UI_Manager&)> ui_creation_function,
                         string name)
    : components(std::move(components)), ui_creation_function(ui_creation_function), name(name) {
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
    pthread_mutex_init(&array_lock, nullptr);
    array = {nullptr, 0, 10000, nullptr};
    array.entities = new unsigned char[entity_type.entity_size * array.entity_capacity];
}

std::tuple<unsigned char*, int> Entity_Array::Create_Entity(ECS* ecs, Entity_ID id) {
    pthread_mutex_lock(&array_lock);
    Dynamic_Array* curr_arr = &array;
    while (curr_arr->next != nullptr)
        curr_arr = curr_arr->next;
    // Check if the array is full
    if (curr_arr->entity_count == curr_arr->entity_capacity) {
        // If the array is full then we allocate another array of twice the size
        // The new entity will be in the new array, however we must keep the old array
        // Other threads might be using the old array still
        // We will merge the two arrays in the clean_up phase
        Dynamic_Array* tmp = curr_arr;
        curr_arr =
            new Dynamic_Array{new unsigned char[entity_type.entity_size * tmp->entity_capacity * 2],
                              tmp->entity_count, tmp->entity_capacity * 2, nullptr};

        if (ecs->In_Block()) {
            tmp->next = curr_arr;
            cout << "Expand" << this->entity_type.name << endl;
        } else {
            memcpy(curr_arr->entities, tmp->entities, tmp->entity_count * entity_type.entity_size);
            delete tmp->entities;
            array = *curr_arr;
            delete curr_arr;
            curr_arr = &array;
            cout << "CopyExpand" << this->entity_type.name << endl;
        }
    }
    int index = curr_arr->entity_count;
    curr_arr->entity_count++;
    // cout << "Creating" << entity_type.name << id << endl;
    pthread_mutex_unlock(&array_lock);

    // Create and return the entity
    unsigned char* ptr = curr_arr->entities + index * entity_type.entity_size;
    std::memset(ptr, 0, entity_type.entity_size);
    Entity_Component& entity = Get_Entity_Data(tuple(ptr, this));
    entity.id = id;
    return std::tuple(ptr, index);
}

void Entity_Array::Copy_Entity(int src_index, int dst_index) {
    if (src_index >= array.entity_count)
        throw std::runtime_error("Index " + to_string(src_index) + " out of bound of size " +
                                 to_string(array.entity_count) + " for entity_array " +
                                 entity_type.name);
    if (dst_index >= array.entity_count)
        throw std::runtime_error("Index " + to_string(dst_index) + " out of bound of size " +
                                 to_string(array.entity_count) + " for entity_array " +
                                 entity_type.name);
    memcpy(array.entities + dst_index * entity_type.entity_size + sizeof(Entity_Component),
           array.entities + src_index * entity_type.entity_size + sizeof(Entity_Component),
           entity_type.entity_size - sizeof(Entity_Component));
}

void Entity_Array::Delete_Entity(ECS* ecs, int index) {
    pthread_mutex_lock(&array_lock);
    // cout << "Deleting" << entity_type.name << Get_Entity_Data(Get_Entity(index)).id << " " <<
    // index
    // << endl;
    if (index != array.entity_count - 1) {
        std::memcpy(array.entities + index * entity_type.entity_size,
                    array.entities + (array.entity_count - 1) * entity_type.entity_size,
                    entity_type.entity_size);
        Entity_ID entity_id = Get_Entity_Data(Get_Entity(index)).id;
        ecs->entities_by_id[entity_id] = tuple(Get_Entity(index), index);
    } else {
        Get_Entity_Data(Get_Entity(index)).id = 0;
    }
    array.entity_count--;
    entity_count--;
    pthread_mutex_unlock(&array_lock);
}

Entity Entity_Array::Get_Entity(int index) {
    if (index >= entity_count)
        throw std::runtime_error("Index " + to_string(index) + " out of bound of size " +
                                 to_string(entity_count) + " for entity_array " + entity_type.name);
    auto* curr_array = &array;
    while (index >= curr_array->entity_count) {
        // if (curr_array->next == nullptr)
        // throw std::runtime_error("Index " + to_string(index) + " out of bound of size " +
        // to_string(array.entity_count) + " for entity_array " +
        // entity_type.name);
        curr_array = curr_array->next;
    }
    return tuple(array.entities + index * entity_type.entity_size, this);
}

void Entity_Array::Clean_Up() {
    Dynamic_Array* end_array = &array;
    while (end_array->next != nullptr)
        end_array = end_array->next;
    entity_count = end_array->entity_count;
    // Check if there is merging to do
    if (end_array == &array)
        return;

    // For each array that isn't the last array, we need to copy its elements into the last array's
    // elements at the same index We can also delete any intermediate array because they are now
    // merged into the last array
    Dynamic_Array* curr_array = &array;
    int starting_index = 0;
    while (curr_array != end_array) {
        int entities_to_copy = curr_array->entity_count - starting_index;
        memcpy(end_array->entities + starting_index * entity_type.entity_size,
               curr_array->entities + starting_index * entity_type.entity_size,
               entities_to_copy * entity_type.entity_size);
        starting_index = curr_array->entity_count;
        delete curr_array->entities;
        auto next_arr = curr_array->next;
        if (curr_array != &array)
            delete curr_array;
        curr_array = next_arr;
    }
    // Replace the current array
    array = *end_array;
    delete end_array;
    // Update all entities that have moved in the ECS entity to id map because their location has
    // changed
    for (int i = 0; i < starting_index; i++) {
        Entity entity = Get_Entity(i);
        Entity_ID e_id = Get_Entity_Data(entity).id;
        if (e_id == 0)
            cerr << "Bad Entity ID!" << endl;
        ecs.entities_by_id[e_id] = tuple(entity, i);
    }
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
    for (auto entity_array : ecs.entity_arrays) {
        if (entity_array->entity_type.Is_Entity_Of_Type(entity_type) && entity_array->Count() > 0)
            arrays.emplace_back(entity_array);
    }
}

Entity Entity_Type_Iterator::Get_Entity(int pos, int index) {
    return arrays[pos]->Get_Entity(index);
}

std::tuple<int, int> Entity_Type_Iterator::Next_Entity(int pos, int index) {
    index++;
    if (arrays[pos]->Count() == index) {
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
    : application(application), work_start(nullptr), work_end(nullptr),
      main_thread(new ECS_Worker(*this, false)) {
    pthread_mutex_init(&to_create_mutex, nullptr);
    entity_arrays = unordered_set<Entity_Array*>();
    blocks = vector<vector<System*>>();
    entities_by_id = unordered_map<Entity_ID, tuple<Entity, int>>();
    to_delete = vector<Entity_ID>();
    random = minstd_rand(seed);
    workers = vector<ECS_Worker*>();
    pthread_mutex_init(&work_mutex, nullptr);
    for (int i = 0; i < 30; i++) {
        workers.emplace_back(new ECS_Worker(*this));
    }
}

ECS::~ECS() {
    for (auto worker : workers) {
        delete worker;
    }
}

void ECS::Update() {
    for (auto systems : blocks) {
        in_block = true;
        for (auto system : systems)
            Apply_Function_To_Entities(system->entity_type, system->function);
        Complete_Work();
        in_block = false;
        for (auto entity_array : entity_arrays)
            entity_array->Clean_Up();

        // At this point we have the valid index of the new entities stored in to_create
        // We must add these to entities_by_id before doing any deletions
        // this is because deletions will re-arrange the entity arrays and invalidate the indicies
        // in to_create
        for (auto [entity_id, entity_array_index] : to_create) {
            if (get<0>(entity_array_index) != nullptr)
                entities_by_id.emplace(
                    entity_id,
                    tuple(get<0>(entity_array_index)->Get_Entity(get<1>(entity_array_index)),
                          get<1>(entity_array_index)));
        }

        for (Entity_ID entity_id : to_delete) {
            if (!entities_by_id.contains(entity_id))
                continue;
            auto [entity, index] = entities_by_id[entity_id];
            entities_by_id.erase(entity_id);
            get<1>(entity)->Delete_Entity(this, index);
            if (to_create.contains(entity_id))
                to_create.erase(entity_id);
            if (on_delete_entity)
                on_delete_entity(entity_id);
        }
        to_delete.clear();

        for (auto [entity_id, entity_array_index] : to_create) {
            if (on_add_entity != nullptr)
                on_add_entity(entity_id);
        }
        to_create.clear();
    }
}

Entity_Array*
ECS::Create_Entity_Type(std::vector<Component_Type*> components,
                        std::function<Object_UI*(Entity, Game_UI_Manager&)> ui_creation_function,
                        string name) {
    auto* new_array = new Entity_Array(
        *this, Entity_Type(std::move(components), std::move(ui_creation_function), name));
    entity_arrays.emplace(new_array);
    return new_array;
}

Entity ECS::Create_Entity(Entity_Type* entity_type) {
    auto search = std::ranges::find_if(entity_arrays, [entity_type](Entity_Array* e) {
        return e->entity_type.Is_Entity_Strictly_Of_type(entity_type);
    });
    Entity_Array* e_array;
    if (search == entity_arrays.end()) {
        e_array = new Entity_Array(*this, entity_type->components);
        entity_arrays.emplace(e_array);
    } else {
        e_array = *search;
    }
    Entity_ID id = next_id++;
    auto [entity, index] = e_array->Create_Entity(this, id);
    if (!in_block) {
        to_create.emplace(id, tuple(nullptr, -1));
        entities_by_id.emplace(id, tuple(tuple(entity, e_array), index));
    } else {
        pthread_mutex_lock(&to_create_mutex);
        to_create.emplace(id, tuple(e_array, index));
        pthread_mutex_unlock(&to_create_mutex);
    }
    return tuple(entity, e_array);
}

Entity ECS::Copy_Entity(Entity_ID entity_id) {
    auto [old_entity, old_entity_index] = entities_by_id[entity_id];
    Entity_ID id = next_id++;
    auto [new_entity, new_entity_index] = get<1>(old_entity)->Create_Entity(this, id);
    get<1>(old_entity)->Copy_Entity(old_entity_index, new_entity_index);
    if (!in_block) {
        to_create.emplace(id, tuple(nullptr, -1));
        entities_by_id.emplace(id, tuple(tuple(new_entity, get<1>(old_entity)), new_entity_index));
    } else {
        pthread_mutex_lock(&to_create_mutex);
        to_create.emplace(id, tuple(get<1>(old_entity), new_entity_index));
        pthread_mutex_unlock(&to_create_mutex);
    }
    return make_tuple(new_entity, get<1>(old_entity));
}

void ECS::Delete_Entity(Entity_ID entity_id) {
    pthread_mutex_lock(&to_create_mutex);
    if (to_create.contains(entity_id))
        to_create.erase(entity_id);
    to_delete.emplace_back(entity_id);
    pthread_mutex_unlock(&to_create_mutex);
}

void ECS::Apply_Function_To_Entities(Entity_Type* entity_type,
                                     const std::function<void(ECS* ecs, Entity entity)>& op) {
    for (auto entity_array : entity_arrays) {
        if (!entity_array->entity_type.Is_Entity_Of_Type(entity_type) || entity_array->Count() == 0)
            continue;
        int start_index = 0;
        int end_index = min(29, entity_array->Count() - 1);
        pthread_mutex_lock(&work_mutex);
        while (start_index <= end_index && start_index <= entity_array->Count() - 1) {
            auto work = new Work_Data{op, entity_array, start_index, end_index, nullptr};
            if (work_end == nullptr) {
                work_start = work;
            } else {
                work_end->next = work;
            }
            work_end = work;
            start_index = end_index + 1;
            end_index = min(start_index + 29, entity_array->Count() - 1);
        }
        pthread_mutex_unlock(&work_mutex);
    }
}

Entity_Type_Iterator ECS::Get_Entities_Of_Type(Entity_Type* entity_type) {
    return Entity_Type_Iterator(*this, entity_type);
}

Entity_Array* ECS::Get_Entities_Of_Exact_Type(Entity_Type* entity_type) {
    auto e_array = *std::ranges::find_if(entity_arrays, [entity_type](Entity_Array* e) {
        return e->entity_type.Is_Entity_Strictly_Of_type(entity_type);
    });
    return e_array;
}

void ECS::Register_System(System* system, int block_index) {
    for (int i = 0; i <= block_index; i++) {
        if (i == blocks.size())
            blocks.emplace_back();
    }
    blocks[block_index].emplace_back(system);
}

Work_Data* ECS::Get_Work(atomic_bool& doing_work) {
    Work_Data* data = nullptr;
    pthread_mutex_lock(&work_mutex);
    data = work_start;
    if (data != nullptr) {
        work_start = data->next;
        if (work_start == nullptr)
            work_end = nullptr;
    }
    doing_work = data != nullptr;
    pthread_mutex_unlock(&work_mutex);
    return data;
}

void ECS::Complete_Work() {
    while (true) {
        pthread_mutex_lock(&work_mutex);
        if (work_end != nullptr) {
            pthread_mutex_unlock(&work_mutex);
            main_thread->Do_Work();
            continue;
        }

        bool has_work = false;
        for (auto worker : workers) {
            if (worker->Doing_Work()) {
                has_work = true;
                break;
            }
        }
        if (!has_work) {
            pthread_mutex_unlock(&work_mutex);
            return;
        }
        pthread_mutex_unlock(&work_mutex);
        this_thread::yield();
    }
}

void* Worker_Function(void* worker) {
    static_cast<ECS_Worker*>(worker)->Do_Worker_Loop();
    return nullptr;
}

ECS_Worker::ECS_Worker(ECS& ecs, bool separate_thread) : ecs(ecs), canceled(false) {
    if (separate_thread)
        pthread_create(&thread, nullptr, Worker_Function, this);
}

void ECS_Worker::Do_Worker_Loop() {
    while (!canceled) {
        Do_Work();
    }
}

void ECS_Worker::Do_Work() {
    auto work = ecs.Get_Work(doing_work);
    if (work == nullptr) {
        doing_work = false;
        this_thread::yield();
        return;
    }
    for (int i = work->starting_index; i <= work->ending_index; i++) {
        work->op(&ecs, work->entity_array->Get_Entity(i));
    }
}

ECS_Worker::~ECS_Worker() {
    canceled = true;
    pthread_cancel(thread);
}

constexpr bool ECS_Worker::Doing_Work() {
    return doing_work;
}

Component_Type Transform_Component::component_type =
    Component_Type{"Transform", sizeof(Transform_Component)};
