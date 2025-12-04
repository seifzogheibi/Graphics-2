#pragma once

#include <vector>
#include <cstdint>

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

// Adjust this to match your actual vertex format used for OBJ meshes
struct Vertex
{
    Vec3f position;
    Vec3f normal;
    Vec3f color;
};

using Index = std::uint32_t;

// Build UFO geometry at the origin.
// Fills outVertices/outIndices with positions, normals, and colors.
void buildUfo(std::vector<Vertex>& outVertices,
              std::vector<Index>&  outIndices);
