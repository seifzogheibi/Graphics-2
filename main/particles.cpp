#include "particles.hpp"
#include "texture.hpp"
#include <cstdlib>
#include <numbers>

// upload particle positions to GPU
static Vec3f sParticlePositions[kMaxParticles];

// reuse empty slots from particles that died
static int allocParticle(ParticleSystem& ps)
{
    for (int i = 0; i < kMaxParticles; ++i)
    {
        if (ps.particles[i].life <= 0.0f)
            return i;
    }
    // no free slots
    return -1;
}

void initialize_ParticleSystem(ParticleSystem& ps, const char* texturePath)
{
    // start with all particles being dead (none are visible)
    for (int i = 0; i < kMaxParticles; ++i)
        ps.particles[i].life = -1.0f;

    ps.alive_count = 0;
    // make an accumulator so that particles spawn smoothly
    ps.emission_accumulator = 0.0f;

    // vao and vbo for particle
    glGenVertexArrays(1, &ps.vao);
    glGenBuffers(1, &ps.vbo);

    glBindVertexArray(ps.vao);
    glBindBuffer(GL_ARRAY_BUFFER, ps.vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        kMaxParticles * sizeof(Vec3f),
        0,
        GL_DYNAMIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        // xyz
        3,
        GL_FLOAT, 
        GL_FALSE,
        // stride is one particle position
        sizeof(Vec3f),
        0
    );

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Load particle texture
    ps.texture = load_texture_2d(texturePath, false);
}

// remove all particles and reset
void resetParticles(ParticleSystem& ps)
{
    for (int i = 0; i < kMaxParticles; ++i)
        ps.particles[i].life = -1.0f;

    ps.alive_count = 0;
    ps.emission_accumulator = 0.0f;
}


void emitParticles(
    ParticleSystem& ps,
    float dt,
    Vec3f const& previous_engine_position,
    Vec3f const& current_engine_position,
    Vec3f const& forward,
    Vec3f const& right,
    Vec3f const& up
)
{
    // particles are produced in a rate per secionds
    ps.emission_accumulator += ps.emission_rate * dt;
    // spawn as many paritcles as possible (decimals are truncated)
    int toSpawn = (int)ps.emission_accumulator;
    // count how many particles are left to spawn
    if (toSpawn > 0)
        ps.emission_accumulator -= (float)toSpawn;

    // how fast particles shoot out, base velocity is calculated in negative since particles are emitted opposite the spaceship
    Vec3f base_velocity = -forward * 7.0f;

    // how wide and far the particle spread
    const float spread_radius = 0.2f;
    const float spread_length = 0.4f;

    // spawn particles individually
    for (int n = 0; n < toSpawn; ++n)
    {
        // use empty slots defined earlier
        int idx = allocParticle(ps);
        if (idx < 0)
            break;

        // spawn at a random point that the rocket has passed
        float initial_position = std::rand() / float(RAND_MAX);
        // start from the base of the rocket (engine/exhaust)
        Vec3f nozzle = -forward * 0.2f;
        // interpolate between engine positions to define particles path 
        Vec3f engine_position = previous_engine_position + (current_engine_position - previous_engine_position) * initial_position + nozzle;

        // generate particle uniform distribution at random to avoid more particles being created in one place than another
        float u1   = std::rand() / float(RAND_MAX);
        float u2r  = std::rand() / float(RAND_MAX);
        float r    = spread_radius * std::sqrt(u1);
        float angle = 2.0f * std::numbers::pi_v<float> * u2r;

        // convert polar to x/z offsets
        float dx = r * std::cos(angle);
        float dz = r * std::sin(angle);
        float dy = (std::rand() / float(RAND_MAX) - 0.5f) * spread_length;

        // convert to world space orientation
        Vec3f offset = right * dx + up * dz + Vec3f{0.f, dy, 0.f};

        // jitter to make particles have more natural affect (x=z)
        Vec3f jitter{
            (std::rand() / float(RAND_MAX) - 0.5f) * 6.0f,
            (std::rand() / float(RAND_MAX) - 0.5f) * 3.0f,
            (std::rand() / float(RAND_MAX) - 0.5f) * 6.0f
        };

        // final velocity of particles
        Vec3f pVelocity = base_velocity + jitter;

        // randomize start age of each particle
        float age = std::rand() / float(RAND_MAX);
        Vec3f substepOffset = -pVelocity * (age * dt);

        // particle intial position
        ps.particles[idx].position  = engine_position + offset + substepOffset;
        // particle velpcity
        ps.particles[idx].velocity  = pVelocity;

        // randomise lifetime of each particle
        float life = std::rand() / float(RAND_MAX);
        ps.particles[idx].life = 0.6f + 0.6f * life;
    }
}


void updateParticles(ParticleSystem& ps, float dt)
{
    // update each particle
    for (int i = 0; i < kMaxParticles; ++i)
    {
        // only alive particles
        if (ps.particles[i].life > 0.0f)
        {
            // decrease life time by time elapsed
            ps.particles[i].life -= dt;
            //  update position if it still alive
            if (ps.particles[i].life > 0.0f)
            {
                ps.particles[i].position = ps.particles[i].position + ps.particles[i].velocity * dt;

                // particles dont go below the ground
                if (ps.particles[i].position.y < -0.98f)
                    ps.particles[i].life = 0.0f;
            }
        }
    }
}

// upload alive partivles to buffer
void uploadParticleData(ParticleSystem& ps)
{
    int alive = 0;
    for (int i = 0; i < kMaxParticles; ++i)
    {
        if (ps.particles[i].life > 0.0f){
            sParticlePositions[alive++] = ps.particles[i].position;
    }}
    ps.alive_count = alive;

    if (ps.alive_count > 0)
    {
        // upload alive particles positions to GPU
        glBindBuffer(GL_ARRAY_BUFFER, ps.vbo);
        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            ps.alive_count * sizeof(Vec3f),
            sParticlePositions
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void renderParticles(
    ParticleSystem const& ps,
    GLuint programId,
    float const* viewProjMatrix,
    Vec3f const& camPosition
)
{
    if (ps.alive_count <= 0){
        return;
    }
    
    // use particle shader
    glUseProgram(programId);

    // alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // disable depth writing so particles dont block each other
    glDepthMask(GL_FALSE);

    // ---- set uniforms by NAME (macOS GLSL 410 safe) ----
    auto U = [&](const char* name) -> GLint {
        return glGetUniformLocation(programId, name);
    };

    // view-projection
    if (GLint loc = U("uViewProj"); loc >= 0)
        glUniformMatrix4fv(loc, 1, GL_TRUE, viewProjMatrix);

    // particle base size
    if (GLint loc = U("uBaseSize"); loc >= 0)
        glUniform1f(loc, 6.0f);

    // camera position
    if (GLint loc = U("uCameraPos"); loc >= 0)
        glUniform3fv(loc, 1, &camPosition.x);

    // optional tint color (only if the shader actually uses uColor)
     Vec3f exhaustColor{ 0.9f, 0.9f, 1.0f };
     if (GLint loc = U("uColor"); loc >= 0)
        glUniform3fv(loc, 1, &exhaustColor.x);

    // particle texture (sampler = texture unit index)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ps.texture);
    if (GLint loc = U("uTexture"); loc >= 0)
        glUniform1i(loc, 0);

    // draw
    glBindVertexArray(ps.vao);
    // each vertex is a sprite not triangle (GL_POINTS)
    glDrawArrays(GL_POINTS, 0, ps.alive_count);
    glBindVertexArray(0);

    // re-enable depth for normal scene rendering
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//    // use particle shader
//    glUseProgram(programId);
//
//    // alpha blending
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    // disable depth writing so particles dont block each other
//    glDepthMask(GL_FALSE);
//
//    // upload view projection matrix
//    glUniformMatrix4fv(0, 1, GL_TRUE, viewProjMatrix);
//    // uplaod particle size
//    glUniform1f(1, 6.0f);
//    // upload camera positioning
//    glUniform3fv(4, 1, &camPosition.x);
//
//    // Vec3f exhaustColor{ 0.9f, 0.9f, 1.0f };
//    // glUniform3fv(2, 1, &exhaustColor.x);
//
//    // use particle texture
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, ps.texture);
//    glUniform1i(3, 0);
//
//    glBindVertexArray(ps.vao);
//    // each vertex is a sprite not triangle (GL_POINTS) 
//    glDrawArrays(GL_POINTS, 0, ps.alive_count);
//    glBindVertexArray(0);
//
//    // renable depth for normal scene rending
//    glDepthMask(GL_TRUE);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
