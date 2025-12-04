#pragma once

#include <vector>
#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"

// Build a UFO as a flat triangle list, with per-vertex positions + normals.
// Output is ready to upload like your terrain: positions -> location 0, normals -> location 1.
void buildUfoFlatArrays(std::vector<Vec3f>& outPositions,
                        std::vector<Vec3f>& outNormals);

