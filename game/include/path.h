#pragma once
#include "emath.h"
#include <vector>

class Path {
  public:
    int index;
    const std::vector<Vector2> positions;

    Path(int index, std::vector<Vector2> positions)
        : index(index), positions(std::move(positions)) {}
    float Get_Rotation_On_Path(int progress) const {
        if (progress > positions.size())
            return 0;
        return Get_Rotation_From_Positions(positions[progress], positions[progress + 1]);
    }
};