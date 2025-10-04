#pragma once
#include "path.h"

struct Unit_Component {
    Path* path;
    int section;
    // Linear interpolation between the previous point and the current point
    // 0 means the start position and 1 means the end position of the segment on the path
    float lerp;
    float speed;
    int team;
    bool spawned;
};