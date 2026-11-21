#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>
namespace vhs90 {
struct DeviceConnectionParams {
    float amount=0.0f;          // 0..1
    float density=0.35f;        // 0..1
    float nodeSize=1.5f;        // pixels
    float speed=1.0f;           // cycles per second
    float distribution=0.5f;    // 0=even grid, 1=random field
    float maxLinkDistance=0.18f;// normalized frame diagonal
    std::uint32_t seed=1;
    std::int64_t frame=0;
    float framesPerSecond=29.97f;
    float red=0.38f,green=0.92f,blue=0.78f;
    bool premultipliedAlpha=true;
};
// Draws a deterministic, animated network of device nodes and connecting
// signal paths. Source and destination must be distinct RGBA buffers.
bool processDeviceConnections(ConstImageView src,ImageView dst,
                              const DeviceConnectionParams& params) noexcept;
} // namespace vhs90
