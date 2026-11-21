#ifndef VHS_METAL_KERNELS_H
#define VHS_METAL_KERNELS_H

#include "VhsParams.h"
#include <cstdint>

#ifdef __APPLE__

bool metalInit(void* cmdQueuePtr);
void metalRelease();

void* metalUploadLut(void* cmdQueuePtr, const float* data, int lutSize);
void  metalFreeLut(void* lutBufferPtr);

// Upload an OSD mask (R8 grayscale, full-frame size).
// Returns id<MTLBuffer> as void*.
void* metalUploadOsd(void* cmdQueuePtr, const uint8_t* mask,
                     int width, int height);
void  metalFreeOsd(void* osdBufferPtr);

// Run the full VHS pipeline (5 passes through ping-pong textures).
void metalDispatchVhs(void* cmdQueuePtr,
                      void* srcBufPtr, void* dstBufPtr,
                      void* outputCstBufPtr,
                      void* inputCstBufPtr,
                      void* osdBufPtr, int osdW, int osdH,
                      const VhsParams& params,
                      bool waitForCompletion = false);

#endif // __APPLE__
#endif // VHS_METAL_KERNELS_H
