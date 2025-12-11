#include "measuring_performance.hpp"

#ifdef ENABLE_GPU_PROFILING

#include <print>
#include <cstdio>

void initProfiler(GPUProfiler& profiler)
{
    // Generate all query objects
    for (int buf = 0; buf < kQueryBufferCount; ++buf)
    {
        glGenQueries(kNumTimestamps, profiler.queries[buf]);
    }
    
    profiler.writeIndex = 0;
    profiler.frameCount = 0;
    profiler.validSamples = 0;
    
    profiler.terrainTimeAccum = 0.0;
    profiler.ufoTimeAccum = 0.0;
    profiler.padsTimeAccum = 0.0;
    profiler.particlesTimeAccum = 0.0;
    profiler.totalGpuTimeAccum = 0.0;
    profiler.cpuFrameTimeAccum = 0.0;
    profiler.cpuSubmitTimeAccum = 0.0;
    
    profiler.lastFrameTime = Clock::now();
    profiler.initialised = true;
}

void profilerBeginFrame(GPUProfiler& profiler)
{
    if (!profiler.initialised) return;
    
    // Record frame start timestamp on GPU
    glQueryCounter(profiler.queries[profiler.writeIndex][0], GL_TIMESTAMP);
    
    // Record CPU frame start
    Clock::time_point now = Clock::now();
    Secondsf frameDelta = now - profiler.lastFrameTime;
    profiler.cpuFrameTimeAccum += frameDelta.count() * 1000.0; // to ms
    profiler.lastFrameTime = now;
    
    profiler.frameStartCpu = now;
}

void profilerMarkTerrainEnd(GPUProfiler& profiler)
{
    if (!profiler.initialised) return;
    glQueryCounter(profiler.queries[profiler.writeIndex][1], GL_TIMESTAMP);
}

// Called after UFO rendering (timestamp[2])
void profilerMarkUfoEnd(GPUProfiler& profiler)
{
    if (!profiler.initialised) return;
    glQueryCounter(profiler.queries[profiler.writeIndex][2], GL_TIMESTAMP);
}

// Called after landing pads rendering (timestamp[3])
void profilerMarkPadsEnd(GPUProfiler& profiler)
{
    if (!profiler.initialised) return;
    glQueryCounter(profiler.queries[profiler.writeIndex][3], GL_TIMESTAMP);
}

void profilerEndFrame(GPUProfiler& profiler)
{
    if (!profiler.initialised) return;
    glQueryCounter(profiler.queries[profiler.writeIndex][4], GL_TIMESTAMP);
}

void profilerCpuSubmitStart(GPUProfiler& profiler)
{
    if (!profiler.initialised) return;
    profiler.submitStartCpu = Clock::now();
}

void profilerCpuSubmitEnd(GPUProfiler& profiler)
{
    if (!profiler.initialised) return;
    profiler.submitEndCpu = Clock::now();
    
    Secondsf submitDelta = profiler.submitEndCpu - profiler.submitStartCpu;
    profiler.cpuSubmitTimeAccum += submitDelta.count() * 1000.0; // to ms
}

void profilerCollectResults(GPUProfiler& profiler)
{
    if (!profiler.initialised) return;
    
    // Try to read results from the oldest buffer (avoids stalling)
    // We read from a buffer that is kQueryBufferCount-1 frames old
    int readIndex = (profiler.writeIndex + 1) % kQueryBufferCount;
    
    // Only start reading after we've filled all buffers
    if (profiler.frameCount >= kQueryBufferCount)
    {
        // Check if results are available (non-blocking)
        GLint available = 0;
        glGetQueryObjectiv(
            profiler.queries[readIndex][kNumTimestamps - 1],
            GL_QUERY_RESULT_AVAILABLE,
            &available
        );
        
        if (available)
        {
            // Read all timestamps
            GLuint64 timestamps[kNumTimestamps];
            for (int i = 0; i < kNumTimestamps; ++i)
            {
                glGetQueryObjectui64v(
                    profiler.queries[readIndex][i],
                    GL_QUERY_RESULT,
                    &timestamps[i]
                );
            }
            
            // Calculate times in milliseconds (timestamps are in nanoseconds)
            // Order: Terrain[0->1], UFO[1->2], Pads[2->3], Particles[3->4]
            double terrainTime = (timestamps[1] - timestamps[0]) / 1000000.0;
            double ufoTime = (timestamps[2] - timestamps[1]) / 1000000.0;
            double padsTime = (timestamps[3] - timestamps[2]) / 1000000.0;
            double particlesTime = (timestamps[4] - timestamps[3]) / 1000000.0;
            double totalTime = (timestamps[4] - timestamps[0]) / 1000000.0;
            
            // Accumulate
            profiler.terrainTimeAccum += terrainTime;
            profiler.ufoTimeAccum += ufoTime;
            profiler.padsTimeAccum += padsTime;
            profiler.particlesTimeAccum += particlesTime;
            profiler.totalGpuTimeAccum += totalTime;
            
            profiler.validSamples++;
        }
    }
    
    // Move to next buffer
    profiler.writeIndex = (profiler.writeIndex + 1) % kQueryBufferCount;
    profiler.frameCount++;
    
    // Print results periodically
    if (profiler.validSamples >= kSampleFrames)
    {
        double invSamples = 1.0 / profiler.validSamples;
        
        double avgTerrain = profiler.terrainTimeAccum * invSamples;
        double avgUfo = profiler.ufoTimeAccum * invSamples;
        double avgPads = profiler.padsTimeAccum * invSamples;
        double avgParticles = profiler.particlesTimeAccum * invSamples;
        double avgTotalGpu = profiler.totalGpuTimeAccum * invSamples;
        double avgCpuFrame = profiler.cpuFrameTimeAccum * invSamples;
        double avgCpuSubmit = profiler.cpuSubmitTimeAccum * invSamples;
        
        std::print("\n========== Performance Results ({} frames) ==========\n", 
                   profiler.validSamples);
        std::print("GPU Timing:\n");
        std::print("  Terrain:       {:7.3f} ms\n", avgTerrain);
        std::print("  Spaceship:     {:7.3f} ms\n", avgUfo);
        std::print("  Landing Pads:  {:7.3f} ms\n", avgPads);
        std::print("  Particles:     {:7.3f} ms\n", avgParticles);
        std::print("  Total GPU:     {:7.3f} ms ({:.1f} FPS potential)\n", 
                   avgTotalGpu, 1000.0 / avgTotalGpu);
        std::print("CPU Timing:\n");
        std::print("  Frame-to-Frame:{:7.3f} ms ({:.1f} FPS actual)\n", 
                   avgCpuFrame, 1000.0 / avgCpuFrame);
        std::print("  Submit Time:   {:7.3f} ms\n", avgCpuSubmit);
        std::print("====================================================\n\n");
        
        // Reset accumulators
        profiler.terrainTimeAccum = 0.0;
        profiler.ufoTimeAccum = 0.0;
        profiler.padsTimeAccum = 0.0;
        profiler.particlesTimeAccum = 0.0;
        profiler.totalGpuTimeAccum = 0.0;
        profiler.cpuFrameTimeAccum = 0.0;
        profiler.cpuSubmitTimeAccum = 0.0;
        profiler.validSamples = 0;
    }
}

void destroyProfiler(GPUProfiler& profiler)
{
    if (!profiler.initialised) return;
    
    for (int buf = 0; buf < kQueryBufferCount; ++buf)
    {
        glDeleteQueries(kNumTimestamps, profiler.queries[buf]);
    }
    
    profiler.initialised = false;
}

#endif // ENABLE_GPU_PROFILING