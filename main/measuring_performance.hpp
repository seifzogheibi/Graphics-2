#ifndef MEASURING_PERFORMANCE_HPP
#define MEASURING_PERFORMANCE_HPP

#include <glad/glad.h>
#include "defaults.hpp"
#include <chrono>

// Define ENABLE_GPU_PROFILING to enable performance measurements
// Build with: -DENABLE_GPU_PROFILING
// Or uncomment the line below:
#define ENABLE_GPU_PROFILING

#ifdef ENABLE_GPU_PROFILING

// Number of timestamp points per frame:
// 0: Frame start
// 1: After terrain
// 2: After landing pads
// 3: After spaceship
// 4: Frame end (after particles)
constexpr int kNumTimestamps = 5;

// Triple buffering to avoid GPU stalls when reading query results
constexpr int kQueryBufferCount = 3;

// How many frames to accumulate before printing results
constexpr int kSampleFrames = 120;

struct GPUProfiler
{
    // Query objects: [buffer index][timestamp index]
    GLuint queries[kQueryBufferCount][kNumTimestamps];
    
    // Which buffer we're currently writing to
    int writeIndex = 0;
    
    // Frame count for cycling through buffers
    int frameCount = 0;
    
    // Accumulated GPU times (in milliseconds)
    double terrainTimeAccum = 0.0;
    double padsTimeAccum = 0.0;
    double ufoTimeAccum = 0.0;
    double particlesTimeAccum = 0.0;
    double totalGpuTimeAccum = 0.0;
    
    // Accumulated CPU times (in milliseconds)
    double cpuFrameTimeAccum = 0.0;
    double cpuSubmitTimeAccum = 0.0;
    
    // Sample count for averaging
    int validSamples = 0;
    
    // CPU timing points
    Clock::time_point frameStartCpu;
    Clock::time_point submitStartCpu;
    Clock::time_point submitEndCpu;
    Clock::time_point lastFrameTime;
    
    // Has the profiler been initialised?
    bool initialised = false;
};

// Initialise the profiler (call once after OpenGL context is created)
void initProfiler(GPUProfiler& profiler);

// Call at the very start of rendering (after glClear)
void profilerBeginFrame(GPUProfiler& profiler);

// Call after terrain rendering
void profilerMarkTerrainEnd(GPUProfiler& profiler);

// Call after landing pads rendering
void profilerMarkPadsEnd(GPUProfiler& profiler);

// Call after spaceship rendering
void profilerMarkUfoEnd(GPUProfiler& profiler);

// Call at the end of all rendering (after particles, before UI)
void profilerEndFrame(GPUProfiler& profiler);

// Mark CPU submit start (call before first draw call)
void profilerCpuSubmitStart(GPUProfiler& profiler);

// Mark CPU submit end (call after last draw call, before swap)
void profilerCpuSubmitEnd(GPUProfiler& profiler);

// Collect results from previous frames (non-blocking)
// Prints results every kSampleFrames frames
void profilerCollectResults(GPUProfiler& profiler);

// Clean up query objects
void destroyProfiler(GPUProfiler& profiler);

#else // ENABLE_GPU_PROFILING not defined - empty stubs

struct GPUProfiler {};

inline void initProfiler(GPUProfiler&) {}
inline void profilerBeginFrame(GPUProfiler&) {}
inline void profilerMarkTerrainEnd(GPUProfiler&) {}
inline void profilerMarkPadsEnd(GPUProfiler&) {}
inline void profilerMarkUfoEnd(GPUProfiler&) {}
inline void profilerEndFrame(GPUProfiler&) {}
inline void profilerCpuSubmitStart(GPUProfiler&) {}
inline void profilerCpuSubmitEnd(GPUProfiler&) {}
inline void profilerCollectResults(GPUProfiler&) {}
inline void destroyProfiler(GPUProfiler&) {}

#endif // ENABLE_GPU_PROFILING

#endif // MEASURING_PERFORMANCE_HPP