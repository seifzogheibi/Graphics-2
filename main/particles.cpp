#include "particles.hpp"
#include "texture.hpp"
#include <cstdlib>
#include <numbers>

// Static buffer for uploading positions to GPU
static Vec3f sParticlePositions[kMaxParticles];

// Find a free particle slot (returns -1 if none available)
static int allocParticle(ParticleSystem& ps)
{
    for (int i = 0; i < kMaxParticles; ++i)
    {
        if (ps.particles[i].life <= 0.0f)
            return i;
    }
    return -1;
}

void initParticleSystem(ParticleSystem& ps, const char* texturePath)
{
    // Initialize all particles as dead
    for (int i = 0; i < kMaxParticles; ++i)
        ps.particles[i].life = -1.0f;

    ps.aliveCount = 0;
    ps.emissionAccumulator = 0.0f;

    // Create VAO and VBO
    glGenVertexArrays(1, &ps.vao);
    glGenBuffers(1, &ps.vbo);

    glBindVertexArray(ps.vao);
    glBindBuffer(GL_ARRAY_BUFFER, ps.vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        kMaxParticles * sizeof(Vec3f),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vec3f),
        (void*)0
    );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Load particle texture
    ps.texture = load_texture_2d(texturePath, false);
}

void resetParticles(ParticleSystem& ps)
{
    for (int i = 0; i < kMaxParticles; ++i)
        ps.particles[i].life = -1.0f;

    ps.aliveCount = 0;
    ps.emissionAccumulator = 0.0f;
}

void emitParticles(
    ParticleSystem& ps,
    float dt,
    Vec3f const& enginePosPrev,
    Vec3f const& enginePosCurr,
    Vec3f const& forwardWS,
    Vec3f const& rightWS,
    Vec3f const& upWS
)
{
    ps.emissionAccumulator += ps.emissionRate * dt;
    int toSpawn = (int)ps.emissionAccumulator;
    if (toSpawn > 0)
        ps.emissionAccumulator -= (float)toSpawn;

    Vec3f baseVel = -forwardWS * 9.0f;

    const float spreadRadius   = 0.2f;
    const float verticalSpread = 0.4f;   // if you want some vertical noise

    for (int n = 0; n < toSpawn; ++n)
    {
        int idx = allocParticle(ps);
        if (idx < 0)
            break;

        // 1) Pick a random point along the engine path this frame
        float uPos = std::rand() / float(RAND_MAX);   // [0,1]
        Vec3f enginePos =
            enginePosPrev + (enginePosCurr - enginePosPrev) * uPos;

        // 2) Random offset in a disk around the nozzle
        float u1   = std::rand() / float(RAND_MAX);
        float u2r  = std::rand() / float(RAND_MAX);
        float r    = spreadRadius * std::sqrt(u1);
        float theta = 2.0f * std::numbers::pi_v<float> * u2r;

        float dx = r * std::cos(theta);
        float dz = r * std::sin(theta);
        float dy = (std::rand() / float(RAND_MAX) - 0.5f) * verticalSpread;

        Vec3f offset = rightWS * dx + upWS * dz + Vec3f{0.f, dy, 0.f};

        // 3) Velocity with jitter
        Vec3f jitter{
            (std::rand() / float(RAND_MAX) - 0.5f) * 6.0f,
            (std::rand() / float(RAND_MAX) - 0.5f) * 3.0f,
            (std::rand() / float(RAND_MAX) - 0.5f) * 6.0f
        };
        Vec3f vel = baseVel + jitter;

        // 4) Sub-frame "age" so particles of this batch are not all the same age
        float tFrac = std::rand() / float(RAND_MAX);   // [0,1]
        Vec3f substepOffset = -vel * (tFrac * dt);     // negative: as if they already moved a bit

        ps.particles[idx].pos  = enginePos + offset + substepOffset;
        ps.particles[idx].vel  = vel;

        float lifeRand = std::rand() / float(RAND_MAX);
        ps.particles[idx].life = 0.6f + 0.6f * lifeRand;   // [0.6, 1.2] s
    }
}


void updateParticles(ParticleSystem& ps, float dt)
{
    for (int i = 0; i < kMaxParticles; ++i)
    {
        if (ps.particles[i].life > 0.0f)
        {
            ps.particles[i].life -= dt;
            if (ps.particles[i].life > 0.0f)
            {
                ps.particles[i].pos =
                   ps.particles[i].pos + ps.particles[i].vel * dt;

                // Ground collision
                if (ps.particles[i].pos.y < -0.98f)
                    ps.particles[i].life = 0.0f;
            }
        }
    }
}

void uploadParticleData(ParticleSystem& ps)
{
    int alive = 0;
    for (int i = 0; i < kMaxParticles; ++i)
    {
        if (ps.particles[i].life > 0.0f){
            sParticlePositions[alive++] = ps.particles[i].pos;
    }}
    ps.aliveCount = alive;

    if (ps.aliveCount > 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, ps.vbo);
        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            ps.aliveCount * sizeof(Vec3f),
            sParticlePositions
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void renderParticles(
    ParticleSystem const& ps,
    GLuint programId,
    float const* viewProjMatrix,
    Vec3f const& camPos
)
{
    if (ps.aliveCount <= 0)
        return;

    glUseProgram(programId);

    // Enable alpha blending and disable depth writes
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glUniformMatrix4fv(0, 1, GL_TRUE, viewProjMatrix);
    glUniform1f(1, 6.0f); // point size
    glUniform3fv(4, 1, &camPos.x);

    Vec3f exhaustColor{ 0.9f, 0.9f, 1.0f };
    glUniform3fv(2, 1, &exhaustColor.x);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ps.texture);
    glUniform1i(3, 0);

    glBindVertexArray(ps.vao);
    glDrawArrays(GL_POINTS, 0, ps.aliveCount);
    glBindVertexArray(0);

    // Restore depth writes
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}