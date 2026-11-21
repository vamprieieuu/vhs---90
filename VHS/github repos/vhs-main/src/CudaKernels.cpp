// Stub CUDA implementation — VHS plugin v1 supports macOS / Metal only.
// Linux/Windows builds compile this file but never execute the dispatch
// (the OFX plugin reports only Metal as supported).

#include "CudaKernels.h"

void cudaDispatchVhs(void*, void*, void*, void*, void*,
                     void*, int, int, const VhsParams&) {}

void* cudaUploadLut(void*, const float*, int) { return nullptr; }
void  cudaFreeLut(void*) {}
void* cudaUploadOsd(void*, const uint8_t*, int, int) { return nullptr; }
void  cudaFreeOsd(void*) {}
