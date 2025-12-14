#pragma once
#include <glad/glad.h>
#include <vector>
#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include "simple_mesh.hpp"

// spaceship mesh 
struct spaceshipMesh
{
    GLuint vao = 0;
    // number of vertices to render with gldrawArrays
    GLsizei vertexCount = 0;
    // y position of the bulbs in ship space
    float bulbsHeight;
    float bulbRadius;
};

spaceshipMesh create_spaceship_mesh();