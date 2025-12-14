#ifndef PARTICLES_HPP
#define PARTICLES_HPP

#include <glad/glad.h>
#include "../vmlib/vec3.hpp"

constexpr int kMaxParticles = 70000;

// a particle has a position, velocity, and lifetime
struct Particle
{
    Vec3f position;
    Vec3f velocity;
    float life;
};

// this stores all the data required to handle a particle
struct ParticleSystem
{
    Particle particles[kMaxParticles];
    int alive_count = 0;
    float emission_accumulator = 0.0f;
    float emission_rate = 15000.0f;
    // rendering
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint texture = 0;
};

// initialize particles and load texture
void initialize_ParticleSystem(ParticleSystem& ps, const char* texturePath);

// remove all particles at reset
void resetParticles(ParticleSystem& ps);

// spawn particles based on emission rate and engine position
void emitParticles(
    ParticleSystem& ps,
    float dt,
    Vec3f const& previous_engine_position,
    Vec3f const& current_engine_position,
    Vec3f const& forward,
    Vec3f const& right,
    Vec3f const& up
);

// update positions and lifetime 
void updateParticles(ParticleSystem& ps, float dt);

// upload to gpu
void uploadParticleData(ParticleSystem& ps);

// render using point sprites
void renderParticles(
    ParticleSystem const& ps,
    GLuint programId,
    float const* viewProjMatrix,
    Vec3f const& camPosition
);

#endif