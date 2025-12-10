// ufo.hpp
#pragma once

#include <glad/glad.h>
#include <vector>
#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include "simple_mesh.hpp"

// Small GL wrapper for a mesh
struct MeshGL
{
    GLuint  vao        = 0;
    GLsizei vertexCount = 0;
};

// Data main needs after building the UFO
struct UfoMesh
{
    MeshGL mesh;           // VAO

    // For lights / engine offsets if you want to reuse them:
    float  bulbRingY;
    float  bulbRadius;
};

// Build the complete UFO mesh (base + top) and upload to GPU
UfoMesh create_ufo_mesh();


