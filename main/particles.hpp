#ifndef PARTICLES_HPP
#define PARTICLES_HPP

#include <glad/glad.h>
#include "../vmlib/vec3.hpp"

// Maximum number of particles
constexpr int kMaxParticles = 70000;

// Single particle data
struct Particle
{
    Vec3f pos;
    Vec3f vel;
    float life;
};

// Particle system state
struct ParticleSystem
{
    Particle particles[kMaxParticles];
    int aliveCount = 0;
    float emissionAccumulator = 0.0f;
    float emissionRate = 15000.0f; // particles per second

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint texture = 0;
};

// Initialize particle system (creates VAO/VBO, loads texture)
void initParticleSystem(ParticleSystem& ps, const char* texturePath);

// Reset all particles to dead state
void resetParticles(ParticleSystem& ps);

// Emit new particles from engine position
void emitParticles(
    ParticleSystem& ps,
    float dt,
    Vec3f const& enginePos,
    Vec3f const& forwardWS,
    Vec3f const& rightWS,
    Vec3f const& upWS
);

// Update particle positions and kill dead/ground-collision particles
void updateParticles(ParticleSystem& ps, float dt);

// Upload alive particle positions to GPU
void uploadParticleData(ParticleSystem& ps);

// Render particles (call between glUseProgram and setting uniforms externally)
void renderParticles(
    ParticleSystem const& ps,
    GLuint programId,
    float const* viewProjMatrix,
    Vec3f const& camPos
);

#endif // PARTICLES_HPP