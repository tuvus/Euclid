#include "ecs.h"
using namespace std;

Entity_Type::Entity_Type(std::vector<Component_Type*> components)
    : components(std::move(components)) {
    entity_size =
        std::accumulate(this->components.begin(), this->components.end(), sizeof(Entity),
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

Entity_Array::Entity_Array(const std::vector<Component_Type*>& components)
    : entity_type(Entity_Type(components)), entity_count(0) {
    entities_capacity = 100;
    entities = new unsigned char[entity_type.entity_size * entities_capacity];
}

std::tuple<unsigned char*, int> Entity_Array::Create_Entity(int id) {
    unsigned char* ptr = entities + entity_count * entity_type.entity_size;
    std::memset(ptr, 0, entity_type.entity_size);
    Entity& entity = Get_Entity_Data(ptr);
    entity.id = id;
    entity_count++;
    return std::tuple(ptr, entity_count - 1);
}

void Entity_Array::Delete_Entity(ECS* ecs, int index) {
    if (index != entity_count - 1) {
        std::memcpy(entities + index * entity_type.entity_size,
                    entities + entity_count * (entity_type.entity_size - 1),
                    entity_type.entity_size);
        Entity_ID entity_id = Get_Entity_Data(Get_Entity(index)).id;
        ecs->entities_by_id[entity_id] = tuple(Get_Entity(index), index, this);
    }
    entity_count--;
}

ECS::ECS() {
    entity_components = std::unordered_set<Entity_Array*>();
    systems = std::unordered_set<System*>();
    entities_by_id =
        std::unordered_map<Entity_ID, std::tuple<unsigned char*, int, Entity_Array*>>();
}

void ECS::Update() {
    for (auto system : systems) {
        Apply_Function_To_Entities(system->entity_type, system->function);
    }
}

std::tuple<unsigned char*, Entity_Array*> ECS::Create_Entity(Entity_Type* entity_type) {
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
    auto [entity, index] = e_array->Create_Entity(next_id++);
    entities_by_id.emplace(Entity_Array::Get_Entity_Data(entity).id,
                           std::tuple(entity, index, e_array));
    return std::tuple(entity, e_array);
}

void ECS::Delete_Entity(Entity_ID entity_id) {
    auto [entity, index, entity_array] = entities_by_id[entity_id];
    entities_by_id.erase(entity_id);
    entity_array->Delete_Entity(this, index);
}

void ECS::Apply_Function_To_Entities(
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

Entity_Array* ECS::Get_Entities_Of_Exact_Type(Entity_Type* entity_type) {
    auto e_array = *std::ranges::find_if(entity_components, [entity_type](Entity_Array* e) {
        return e->entity_type.Is_Entity_Strictly_Of_type(entity_type);
    });
    return e_array;
}

Component_Type Transform_Component::component_type =
    Component_Type{"Transform", sizeof(Transform_Component)};
