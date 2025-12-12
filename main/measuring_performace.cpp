#include "measuring_performance.hpp"

#ifdef ENABLE_GPU_PROFILING

#include <print>

static double ns_to_ms(GLuint64 ns)
{
    return double(ns) / 1'000'000.0;
}

void gpuInit(GPUProfiler& p)
{
    for (int b = 0; b < kQueryBufferCount; ++b)
        glGenQueries(kNumTimestamps, p.q[b]);

    p.write = 0;
    p.frame = 0;
    p.samples = 0;

    p.accTerrain = p.accUfo = p.accPads = p.accTotal = 0.0;
    p.accCpuFrame = p.accCpuSubmit = 0.0;

    p.lastFrame = Clock::now();
    p.initialised = true;
}

void gpuDestroy(GPUProfiler& p)
{
    if (!p.initialised) return;

    for (int b = 0; b < kQueryBufferCount; ++b)
        glDeleteQueries(kNumTimestamps, p.q[b]);

    p.initialised = false;
}

void gpuBegin(GPUProfiler& p)
{
    if (!p.initialised) return;

    // GPU: start-of-frame stamp
    glQueryCounter(p.q[p.write][(int)Stamp::FrameStart], GL_TIMESTAMP);

    // CPU: frame-to-frame (avoid weird first sample)
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
    if (!p.initialised || !doProfile) return;
    glQueryCounter(p.q[p.write][(int)s], GL_TIMESTAMP);
}

void cpuSubmitBegin(GPUProfiler& p)
{
    if (!p.initialised) return;
    p.submitStart = Clock::now();
}

void cpuSubmitEnd(GPUProfiler& p)
{
    if (!p.initialised) return;

    auto end = Clock::now();
    Secondsf dt = end - p.submitStart;
    p.accCpuSubmit += dt.count() * 1000.0;
}

void gpuEndAndCollect(GPUProfiler& p)
{
    if (!p.initialised) return;

    // Read from the oldest buffer (one-behind works with kQueryBufferCount=3)
    int read = (p.write + 1) % kQueryBufferCount;

    if (p.frame >= kQueryBufferCount)
    {
        GLint available = 0;
        glGetQueryObjectiv(
            p.q[read][(int)Stamp::FrameEnd],
            GL_QUERY_RESULT_AVAILABLE,
            &available
        );

        if (available)
        {
            GLuint64 t[kNumTimestamps]{};
            for (int i = 0; i < kNumTimestamps; ++i)
                glGetQueryObjectui64v(p.q[read][i], GL_QUERY_RESULT, &t[i]);

            // section times (ms)
            double terrain = ns_to_ms(t[(int)Stamp::TerrainEnd] - t[(int)Stamp::FrameStart]);
            double ufo = ns_to_ms(t[(int)Stamp::UfoEnd]     - t[(int)Stamp::TerrainEnd]);
            double pads = ns_to_ms(t[(int)Stamp::PadsEnd]    - t[(int)Stamp::UfoEnd]);
            double total = ns_to_ms(t[(int)Stamp::FrameEnd]   - t[(int)Stamp::FrameStart]);

            p.accTerrain += terrain;
            p.accUfo += ufo;
            p.accPads += pads;
            p.accTotal += total;

            p.samples++;
        }
    }

    // advance ring buffer
    p.write = (p.write + 1) % kQueryBufferCount;
    p.frame++;

    if (p.samples >= kSampleFrames)
    {
        double inv = 1.0 / double(p.samples);

        double avgTerrain = p.accTerrain * inv;
        double avgUfo = p.accUfo * inv;
        double avgPads = p.accPads * inv;
        double avgTotal = p.accTotal * inv;
        double avgCpuF = p.accCpuFrame * inv;
        double avgCpuSub = p.accCpuSubmit * inv;

        std::print("\nPerformance Results ({} frames)\n", p.samples);
        std::print("GPU Timing (GL_TIMESTAMP queries):\n");
        std::print("Terrain:     {:7.3f} ms\n", avgTerrain);
        std::print("Spaceship:   {:7.3f} ms\n", avgUfo);
        std::print("Landing Pads:{:7.3f} ms\n", avgPads);
        std::print("Total GPU:   {:7.3f} ms ({:.1f} FPS potential)\n", avgTotal, 1000.0 / avgTotal);

        std::print("CPU Timing (std::chrono):\n");
        std::print("  Frame-to-Frame:{:7.3f} ms ({:.1f} FPS actual)\n", avgCpuF, 1000.0 / avgCpuF);
        std::print("  Submit Time:   {:7.3f} ms\n", avgCpuSub);

        // reset
        p.accTerrain = p.accUfo = p.accPads = p.accTotal = 0.0;
        p.accCpuFrame = p.accCpuSubmit = 0.0;
        p.samples = 0;
    }
}

#endif // ENABLE_GPU_PROFILING