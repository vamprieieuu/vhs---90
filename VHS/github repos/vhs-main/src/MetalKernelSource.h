#ifndef METAL_KERNEL_SOURCE_H
#define METAL_KERNEL_SOURCE_H

// ============================================================
// Lofi FX VHS — Metal runtime shader source.
//
// ALL VHS shader math lives in VhsKernelCore.inc and is shared with
// the CUDA path. This file:
//   1. Pulls in the core text (as a C++ string literal, embedded by
//      CMake via configure_file from VhsKernelCore.inc).
//   2. Prepends a Metal-specific PREFIX that #includes <metal_stdlib>
//      and defines the platform-abstraction macros the core uses.
//   3. Appends a Metal-specific SUFFIX with the three `kernel void`
//      entry points that forward to the DEVFN body functions in the
//      core.
//
// To keep Metal and CUDA in sync, edit VhsKernelCore.inc ONLY. Only
// touch this file if you are genuinely adding a new platform macro,
// a new kernel entry point, or a new buffer binding.
// ============================================================

#include <string>
#include "VhsKernelCoreText.h"   // generated: `vhsKernelCoreSource`

namespace {

// ------------------------------------------------------------
// Metal-specific prefix: maps our abstraction macros onto MSL.
// ------------------------------------------------------------
static const char* kVhsMetalPrefix = R"METAL_PREFIX(
#include <metal_stdlib>
using namespace metal;

#define DEVFN             inline
#define GLOBAL_RO(T)      device const T*
#define GLOBAL_RW(T)      device T*
#define CONSTANT_REF(T)   constant T&
#define READ_TEX          texture2d<float, access::sample>
#define WRITE_TEX         texture2d<float, access::write>
#define MAKE_FLOAT3(a,b,c)    float3((a),(b),(c))
#define MAKE_FLOAT4(a,b,c,d)  float4((a),(b),(c),(d))
#define MAKE_INT2(a,b)        int2((a),(b))
#define MAKE_INT3(a,b,c)      int3((a),(b),(c))
#define EXP2 precise::exp2
#define LOG2 precise::log2

constexpr sampler vhs_linear_sampler(filter::linear, address::clamp_to_edge);

#define TEX_SAMPLE(t,u,v)  (t).sample(vhs_linear_sampler, float2((u),(v)))
#define TEX_WRITE(t,x,y,v) (t).write((v), uint2(uint(x), uint(y)))
)METAL_PREFIX";

// ------------------------------------------------------------
// Metal-specific suffix: the three `kernel void` entry points.
// Each simply extracts the thread coordinate and forwards to the
// DEVFN body in the shared core.
// ------------------------------------------------------------
static const char* kVhsMetalSuffix = R"METAL_SUFFIX(
kernel void vhsEncodeKernel(
    device const float*    srcBuf       [[buffer(0)]],
    constant VhsParams&    p            [[buffer(2)]],
    device const float*    inputCstLut  [[buffer(5)]],
    device const uint8_t*  osdMask      [[buffer(6)]],
    constant int2&         osdSize      [[buffer(7)]],
    texture2d<float, access::write> outTex [[texture(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    int x = int(gid.x), y = int(gid.y);
    if (x >= p.width || y >= p.height) return;
    vhs_encode_body(x, y, srcBuf, p, inputCstLut, osdMask,
                    osdSize.x, osdSize.y, outTex);
}

kernel void vhsWarpKernel(
    constant VhsParams&              p      [[buffer(2)]],
    texture2d<float, access::sample> inTex  [[texture(0)]],
    texture2d<float, access::write>  outTex [[texture(1)]],
    uint2 gid [[thread_position_in_grid]])
{
    int x = int(gid.x), y = int(gid.y);
    if (x >= p.width || y >= p.height) return;
    vhs_warp_body(x, y, p, inTex, outTex);
}

kernel void vhsBandwidthKernel(
    constant VhsParams&              p      [[buffer(2)]],
    texture2d<float, access::sample> inTex  [[texture(0)]],
    texture2d<float, access::write>  outTex [[texture(1)]],
    uint2 gid [[thread_position_in_grid]])
{
    int x = int(gid.x), y = int(gid.y);
    if (x >= p.width || y >= p.height) return;
    vhs_bandwidth_body(x, y, p, inTex, outTex);
}

kernel void vhsVerticalKernel(
    constant VhsParams&              p      [[buffer(2)]],
    texture2d<float, access::sample> inTex  [[texture(0)]],
    texture2d<float, access::write>  outTex [[texture(1)]],
    uint2 gid [[thread_position_in_grid]])
{
    int x = int(gid.x), y = int(gid.y);
    if (x >= p.width || y >= p.height) return;
    vhs_bandwidth_rows_body(x, y, p, inTex, outTex);
}

kernel void vhsFinalKernel(
    device float*          dstBuf        [[buffer(1)]],
    constant VhsParams&    p             [[buffer(2)]],
    device const float*    outputCstLut  [[buffer(3)]],
    texture2d<float, access::sample> inTex [[texture(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    int x = int(gid.x), y = int(gid.y);
    if (x >= p.width || y >= p.height) return;
    vhs_color_body(x, y, dstBuf, p, outputCstLut, inTex);
}
)METAL_SUFFIX";

inline const std::string& getMetalKernelSource() {
    static const std::string s =
        std::string(kVhsMetalPrefix) +
        std::string(vhsKernelCoreSource) +
        std::string(kVhsMetalSuffix);
    return s;
}

} // namespace

#endif // METAL_KERNEL_SOURCE_H
