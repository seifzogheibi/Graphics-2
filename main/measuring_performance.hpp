#ifndef MEASURING_PERFORMANCE_HPP
#define MEASURING_PERFORMANCE_HPP

#include <glad/glad.h>
#include "defaults.hpp"

// Recommended: enable via build flags -DENABLE_GPU_PROFILING
// If you want it always-on, uncomment the next line.
 #define ENABLE_GPU_PROFILING

// Needs to exist even when profiling is disabled (stubs use it)
enum class Stamp : int
{
    FrameStart  = 0,
    TerrainEnd  = 1,
    UfoEnd = 2,
    PadsEnd = 3,
    FrameEnd = 4
};

#ifdef ENABLE_GPU_PROFILING

constexpr int kNumTimestamps = 5;
constexpr int kQueryBufferCount = 3;
constexpr int kSampleFrames = 10;

struct GPUProfiler
{
    GLuint q[kQueryBufferCount][kNumTimestamps]{};

    int write = 0;
    int frame = 0;
    int samples = 0;

    double accTerrain = 0.0;
    double accUfo = 0.0;
    double accPads = 0.0;
    double accTotal = 0.0;

    double accCpuFrame = 0.0;
    double accCpuSubmit = 0.0;

    Clock::time_point lastFrame{};
    Clock::time_point submitStart{};

    bool initialised = false;
};

void gpuInit(GPUProfiler& p);
void gpuDestroy(GPUProfiler& p);

void gpuBegin(GPUProfiler& p);
void gpuStamp(GPUProfiler& p, Stamp s, bool doProfile = true);

void cpuSubmitBegin(GPUProfiler& p);
void cpuSubmitEnd(GPUProfiler& p);

void gpuEndAndCollect(GPUProfiler& p);

#else

struct GPUProfiler {};

inline void gpuInit(GPUProfiler&) {}
inline void gpuDestroy(GPUProfiler&) {}
inline void gpuBegin(GPUProfiler&) {}
inline void gpuStamp(GPUProfiler&, Stamp, bool = true) {}
inline void cpuSubmitBegin(GPUProfiler&) {}
inline void cpuSubmitEnd(GPUProfiler&) {}
inline void gpuEndAndCollect(GPUProfiler&) {}

#endif

#endif // MEASURING_PERFORMANCE_HPP