#ifndef VHS_CUDA_KERNELS_H
#define VHS_CUDA_KERNELS_H

#include "VhsParams.h"
#include <cstdint>

// Stub CUDA dispatch — VHS plugin currently macOS-Metal only.
// Linux/Windows builds get a CPU passthrough fallback handled in
// LofiFxVhsPlugin.cpp.
void cudaDispatchVhs(void* streamPtr,
                     void* srcBufPtr, void* dstBufPtr,
                     void* outputCstBufPtr,
                     void* inputCstBufPtr,
                     void* osdBufPtr, int osdW, int osdH,
                     const VhsParams& params);

void* cudaUploadLut(void* streamPtr, const float* data, int lutSize);
void  cudaFreeLut(void* devPtr);
void* cudaUploadOsd(void* streamPtr, const uint8_t* mask, int w, int h);
void  cudaFreeOsd(void* devPtr);

#endif
