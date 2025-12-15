#ifndef MEASURING_PERFORMANCE_HPP
#define MEASURING_PERFORMANCE_HPP

#include <glad/glad.h>
#include "defaults.hpp"

 #define ENABLE_GPU_PROFILING

// Stamps that measure how long each section takes
enum class Stamp : int
{ FrameStart  = 0, TerrainEnd  = 1, SpaceshipEnd = 2, PadsEnd = 3, FrameEnd = 4};

#ifdef ENABLE_GPU_PROFILING

constexpr int kNumTimestamps = 5; // number of stamps each frame
constexpr int kQueryBuffers = 3; // frames kept before reading results
constexpr int kSampleFrames = 200;

struct GPUProfiler
{
    GLuint q[kQueryBuffers][kNumTimestamps]{};
    int write = 0;
    int frame = 0;
    int samples = 0;
    // accumulated times in ms
    double accTerrain = 0.0;
    double accSpaceShip = 0.0;
    double accLpads = 0.0;
    double accTotal = 0.0;
    // CPU times in ms
    double accCpuFrame = 0.0;
    double accCpuSubmit = 0.0;

    Clock::time_point lastFrame{}; // frame to frame timing
    Clock::time_point submitStart{};// submit time
    bool ready = false;
};

void gpuInit(GPUProfiler& p);
void gpuDestroy(GPUProfiler& p);
void gpuBegin(GPUProfiler& p);
void gpuStamp(GPUProfiler& p, Stamp s, bool doProfile = true);
void cpuSubmitBegin(GPUProfiler& p);
void cpuSubmitEnd(GPUProfiler& p);
void gpuCollectResults(GPUProfiler& p);

#else

struct GPUProfiler {};

inline void gpuInit(GPUProfiler&) {}
inline void gpuDestroy(GPUProfiler&) {}
inline void gpuBegin(GPUProfiler&) {}
inline void gpuStamp(GPUProfiler&, Stamp, bool = true) {}
inline void cpuSubmitBegin(GPUProfiler&) {}
inline void cpuSubmitEnd(GPUProfiler&) {}
inline void gpuCollectResults(GPUProfiler&) {}

#endif

#endif