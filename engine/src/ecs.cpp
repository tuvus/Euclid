#include "ecs.h"

Component_Type Transform_Component::component_type =
    Component_Type{"Transform", sizeof(Transform_Component)};
