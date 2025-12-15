#include "measuring_performance.hpp"

#ifdef ENABLE_GPU_PROFILING

#include <print>

// Convert to milliseconds
static double ns_to_ms(GLuint64 ns)
{ return double(ns) * 1e-6;}

void gpuInit(GPUProfiler& p)
{
    // Create query object for each buffer and timestamp
    for (int b = 0; b < kQueryBuffers; ++b)
    glGenQueries(kNumTimestamps, p.q[b]);
    p.write = 0;
    p.frame = 0;
    p.samples = 0;
    p.accTerrain = p.accSpaceShip = p.accLpads = p.accTotal = 0.0;
    p.accCpuFrame = p.accCpuSubmit = 0.0;
    p.lastFrame = Clock::now();
    p.ready = true;
}

void gpuDestroy(GPUProfiler& p)
{
    if (!p.ready) return;
    for (int b = 0; b < kQueryBuffers; ++b)
    glDeleteQueries(kNumTimestamps, p.q[b]);
    p.ready = false;
}

void gpuBegin(GPUProfiler& p)
{
    if (!p.ready) return;
    // timestamp at frame start
    glQueryCounter(p.q[p.write][(int)Stamp::FrameStart], GL_TIMESTAMP);

    // Cpu measures actual frame time
    // skips first frame
    auto now = Clock::now();
    if (p.frame > 0)
    {
        Secondsf dt = now - p.lastFrame;
        p.accCpuFrame += dt.count() * 1000.0;
    }
    p.lastFrame = now;
}
void gpuStamp(GPUProfiler& p, Stamp s, bool doProfile)
{ 
    if (!p.ready || !doProfile) return;
    glQueryCounter(p.q[p.write][(int)s], GL_TIMESTAMP);
}
// Start cpu time for submit
void cpuSubmitBegin(GPUProfiler& p)
{
    if (!p.ready) return;
    p.submitStart = Clock::now();
}
// End cpu summit time
void cpuSubmitEnd(GPUProfiler& p)
{
    if (!p.ready) return;

    auto end = Clock::now();
    Secondsf dt = end - p.submitStart;
    p.accCpuSubmit += dt.count() * 1000.0;
}
void gpuCollectResults(GPUProfiler& p)
{
    if (!p.ready) return;

    // Read from the oldest buffer so the gpu doesnt stall
    int read = (p.write + 1) % kQueryBuffers;

    if (p.frame >= kQueryBuffers)
    {
        GLint available = 0;
        glGetQueryObjectiv(p.q[read][(int)Stamp::FrameEnd],GL_QUERY_RESULT_AVAILABLE,&available);
        // Only reads the results if they are available
        if (available)
        {
            GLuint64 t[kNumTimestamps]{};
            for (int i = 0; i < kNumTimestamps; ++i)
                glGetQueryObjectui64v(p.q[read][i], GL_QUERY_RESULT, &t[i]);
            // section times in ms
            double terrain = ns_to_ms(t[(int)Stamp::TerrainEnd] - t[(int)Stamp::FrameStart]);
            double spaceship = ns_to_ms(t[(int)Stamp::SpaceshipEnd] - t[(int)Stamp::TerrainEnd]);
            double pads = ns_to_ms(t[(int)Stamp::PadsEnd] - t[(int)Stamp::SpaceshipEnd]);
            double total = ns_to_ms(t[(int)Stamp::FrameEnd] - t[(int)Stamp::FrameStart]);

            p.accTerrain += terrain;
            p.accSpaceShip += spaceship;
            p.accLpads += pads;
            p.accTotal += total;
            p.samples++;
        }
    }

    // Moves to the next buffer for the next frame
    p.write = (p.write + 1) % kQueryBuffers;
    p.frame++;

    if (p.samples >= kSampleFrames)
    {
        double inv = 1.0 / double(p.samples);
        double avgTerrain = p.accTerrain * inv;
        double avgSpaceship = p.accSpaceShip * inv;
        double avgPads = p.accLpads * inv;
        double avgTotal = p.accTotal * inv;
        double avgCpuF = p.accCpuFrame * inv;
        double avgCpuSub = p.accCpuSubmit * inv;

        std::print("\nPerformance Results {} frames\n", p.samples);
        std::print("GPU Timing:\n");
        std::print("Terrain:     {:7.3f} ms\n", avgTerrain);
        std::print("Spaceship:   {:7.3f} ms\n", avgSpaceship);
        std::print("Landing Pads:{:7.3f} ms\n", avgPads);
        std::print("Total GPU:   {:7.3f} ms\n", avgTotal, 1000.0 / avgTotal);
        std::print("CPU Timing:\n");
        std::print("Frame-to-Frame:{:7.3f} ms ({:.1f} FPS actual)\n", avgCpuF, 1000.0 / avgCpuF);
        std::print("Submit Time:   {:7.3f} ms\n", avgCpuSub);

        p.accTerrain = p.accSpaceShip = p.accLpads = p.accTotal = 0.0; p.accCpuFrame = p.accCpuSubmit = 0.0; p.samples = 0;
    }
}
#endif