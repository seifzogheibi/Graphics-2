// ufo.hpp
#pragma once
#include <vector>
#include "../vmlib/vec3.hpp"

// Build a single non-indexed triangle list.
// outBaseVertexCount  = number of vertices for the “grey base” part
// outTopVertexCount   = number of vertices for the “blue dome + antenna” part
void buildUfoFlatArrays(
    std::vector<Vec3f>& outPositions,
    std::vector<Vec3f>& outNormals,
    int& outBaseVertexCount,
    int& outTopVertexCount, 
    int& outBodyStart,
    int& outBodyCount,
    int& outEngineStart,
    int& outEngineCount,
    int& outFinsStart,
    int& outFinsCount,
    int& outBulbsStart,
    int& outBulbsCount,
    int& outTopStart,
    int& outTopCount
);
