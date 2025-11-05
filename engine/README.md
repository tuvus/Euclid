# Euclid Game Engine

## Entity Component System Design

The purpose of the ECS design is to place entities that behave similarly close together
in order to reduce the number of cache misses and reduce the number of pointers to increase performance.
Another reason that the ECS design might be helpful is to allow for deterministic parallelism.
Entities are instances of game objects and can hold any amount of components on them.
Each component is of a certain component type and holds data for the entity.
Systems are functions that work on all entities that have a certain set of components.

- To create a component type a struct needs to be made with a member variable of type Component_type.
- To create a component call the ECS::Create_Entity method with the desired component_types.
- If you want entities of a certain type to be rendered you need to make a UI_Object class and
  call ECS::Create_Entity_Type giving it a function that creates the entity.
- To create a system that applies to a certain set of entities call ECS::Register_System and pass it the function and
  entity type.
- For now each system runs serially but the idea is to change them to run in parallel later.