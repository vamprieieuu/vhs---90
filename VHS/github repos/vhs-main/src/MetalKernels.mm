#ifdef __APPLE__

#import <Metal/Metal.h>
#include <string>
#include "MetalKernels.h"
#include "MetalKernelSource.h"

// ============================================================
// Module-level Metal state
// ============================================================
static id<MTLLibrary>              s_lib       = nil;
static id<MTLComputePipelineState> s_psoEncode  = nil;
static id<MTLComputePipelineState> s_psoWarp    = nil;
static id<MTLComputePipelineState> s_psoBand    = nil;
static id<MTLComputePipelineState> s_psoVert    = nil;
static id<MTLComputePipelineState> s_psoFinal   = nil;

// Two ping-pong textures for the YCbCr intermediate (rgba16Float, A unused).
static id<MTLTexture>              s_texA     = nil;
static id<MTLTexture>              s_texB     = nil;
static int                         s_texW     = 0;
static int                         s_texH     = 0;

// Tiny dummy buffers / textures so kernel arg slots are always bound.
static id<MTLBuffer>               s_dumLut   = nil;
static id<MTLBuffer>               s_dumOsd   = nil;

bool metalInit(void* cmdQueuePtr) {
    @autoreleasepool {
        if (s_lib && s_psoEncode && s_psoWarp && s_psoBand && s_psoVert && s_psoFinal) return true;
        id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)cmdQueuePtr;
        if (!queue) { NSLog(@"LofiFxVhs: metalInit nil queue"); return false; }
        id<MTLDevice> device = queue.device;
        if (!device) { NSLog(@"LofiFxVhs: nil device"); return false; }

        NSError* err = nil;
        if (!s_lib) {
            MTLCompileOptions* opts = [[MTLCompileOptions alloc] init];
            if (@available(macOS 15.0, *)) {
                opts.mathMode = MTLMathModeFast;
            } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                opts.fastMathEnabled = YES;
#pragma clang diagnostic pop
            }
            const std::string& src = getMetalKernelSource();
            s_lib = [device newLibraryWithSource:
                       [NSString stringWithUTF8String:src.c_str()]
                     options:opts error:&err];
            if (!s_lib) {
                NSLog(@"LofiFxVhs: shader compile failed: %@", err);
                return false;
            }
            NSLog(@"LofiFxVhs: Metal library compiled successfully");
        }

        auto makePso = [&](NSString* name, id<MTLComputePipelineState>* out) {
            if (*out) return;
            id<MTLFunction> fn = [s_lib newFunctionWithName:name];
            if (!fn) { NSLog(@"LofiFxVhs: missing kernel %@", name); return; }
            NSError* e = nil;
            *out = [device newComputePipelineStateWithFunction:fn error:&e];
            if (!*out) NSLog(@"LofiFxVhs: pipeline %@ failed: %@", name, e);
        };
        makePso(@"vhsEncodeKernel",    &s_psoEncode);
        makePso(@"vhsWarpKernel",      &s_psoWarp);
        makePso(@"vhsBandwidthKernel", &s_psoBand);
        makePso(@"vhsVerticalKernel",  &s_psoVert);
        makePso(@"vhsFinalKernel",     &s_psoFinal);

        if (!s_dumLut)
            s_dumLut = [device newBufferWithLength:16 options:MTLResourceStorageModeShared];
        if (!s_dumOsd)
            s_dumOsd = [device newBufferWithLength:16 options:MTLResourceStorageModeShared];

        return s_psoEncode && s_psoWarp && s_psoBand && s_psoVert && s_psoFinal;
    }
}

void metalRelease() {
    @autoreleasepool {
        s_psoEncode = nil; s_psoWarp = nil; s_psoBand = nil; s_psoVert = nil; s_psoFinal = nil;
        s_texA = nil; s_texB = nil; s_texW = 0; s_texH = 0;
        s_dumLut = nil; s_dumOsd = nil;
        s_lib = nil;
    }
}

void* metalUploadLut(void* cmdQueuePtr, const float* data, int lutSize) {
    @autoreleasepool {
        id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)cmdQueuePtr;
        if (!queue || !data || lutSize <= 0) return nullptr;
        size_t bytes = (size_t)lutSize * lutSize * lutSize * 3 * sizeof(float);
        id<MTLBuffer> buf = [queue.device newBufferWithBytes:data length:bytes
                                          options:MTLResourceStorageModeShared];
        if (!buf) return nullptr;
        return (void*)CFBridgingRetain(buf);
    }
}
void metalFreeLut(void* p) { if (p) CFBridgingRelease(p); }

void* metalUploadOsd(void* cmdQueuePtr, const uint8_t* mask, int w, int h) {
    @autoreleasepool {
        id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)cmdQueuePtr;
        if (!queue || !mask || w <= 0 || h <= 0) return nullptr;
        size_t bytes = (size_t)w * h;
        id<MTLBuffer> buf = [queue.device newBufferWithBytes:mask length:bytes
                                          options:MTLResourceStorageModeShared];
        if (!buf) return nullptr;
        return (void*)CFBridgingRetain(buf);
    }
}
void metalFreeOsd(void* p) { if (p) CFBridgingRelease(p); }

// Ensure the ping-pong textures are large enough.
static void ensureTextures(id<MTLDevice> device, int w, int h) {
    if (s_texA && s_texW >= w && s_texH >= h) return;
    MTLTextureDescriptor* d =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA16Float
                                                           width:(NSUInteger)w
                                                          height:(NSUInteger)h
                                                       mipmapped:NO];
    d.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
    d.storageMode = MTLStorageModePrivate;
    s_texA = [device newTextureWithDescriptor:d];
    s_texB = [device newTextureWithDescriptor:d];
    s_texW = w; s_texH = h;
}

void metalDispatchVhs(void* cmdQueuePtr,
                      void* srcBufPtr, void* dstBufPtr,
                      void* outputCstBufPtr,
                      void* inputCstBufPtr,
                      void* osdBufPtr, int osdW, int osdH,
                      const VhsParams& params,
                      bool waitForCompletion)
{
    if (!s_psoEncode || !s_psoWarp || !s_psoBand || !s_psoVert || !s_psoFinal) return;
    @autoreleasepool {
        id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)cmdQueuePtr;
        id<MTLDevice> device = queue.device;
        ensureTextures(device, params.width, params.height);

        id<MTLBuffer> src = (__bridge id<MTLBuffer>)srcBufPtr;
        id<MTLBuffer> dst = (__bridge id<MTLBuffer>)dstBufPtr;

        id<MTLBuffer> ocst = outputCstBufPtr ? (__bridge id<MTLBuffer>)outputCstBufPtr : s_dumLut;
        id<MTLBuffer> icst = inputCstBufPtr  ? (__bridge id<MTLBuffer>)inputCstBufPtr  : s_dumLut;
        id<MTLBuffer> osd  = osdBufPtr      ? (__bridge id<MTLBuffer>)osdBufPtr      : s_dumOsd;
        int osdSize[2] = { osdBufPtr ? osdW : 0, osdBufPtr ? osdH : 0 };

        id<MTLCommandBuffer> cmd = [queue commandBuffer];

        MTLSize grid = MTLSizeMake((NSUInteger)params.width, (NSUInteger)params.height, 1);

        // Pass 1 — encode (writes texA)
        {
            id<MTLComputeCommandEncoder> enc = [cmd computeCommandEncoder];
            [enc setComputePipelineState:s_psoEncode];
            [enc setBuffer:src offset:0 atIndex:0];
            [enc setBytes:&params length:sizeof(VhsParams) atIndex:2];
            [enc setBuffer:icst offset:0 atIndex:5];
            [enc setBytes:&params length:sizeof(VhsParams) atIndex:2];
            [enc setBuffer:icst offset:0 atIndex:5];
            [enc setBuffer:osd  offset:0 atIndex:6];
            [enc setBytes:&osdSize length:sizeof(osdSize) atIndex:7];
            [enc setTexture:s_texA atIndex:0];
            NSUInteger tw = s_psoEncode.threadExecutionWidth;
            NSUInteger th = s_psoEncode.maxTotalThreadsPerThreadgroup / tw;
            [enc dispatchThreads:grid threadsPerThreadgroup:MTLSizeMake(tw, th, 1)];
            [enc endEncoding];
        }

        // Pass 2 — geometric warp (texA → texB)
        {
            id<MTLComputeCommandEncoder> enc = [cmd computeCommandEncoder];
            [enc setComputePipelineState:s_psoWarp];
            [enc setBytes:&params length:sizeof(VhsParams) atIndex:2];
            [enc setTexture:s_texA atIndex:0];
            [enc setTexture:s_texB atIndex:1];
            NSUInteger tw = s_psoWarp.threadExecutionWidth;
            NSUInteger th = s_psoWarp.maxTotalThreadsPerThreadgroup / tw;
            [enc dispatchThreads:grid threadsPerThreadgroup:MTLSizeMake(tw, th, 1)];
            [enc endEncoding];
        }

        // Pass 3 — horizontal bandwidth (texB → texA)
        {
            id<MTLComputeCommandEncoder> enc = [cmd computeCommandEncoder];
            [enc setComputePipelineState:s_psoBand];
            [enc setBytes:&params length:sizeof(VhsParams) atIndex:2];
            [enc setTexture:s_texB atIndex:0];
            [enc setTexture:s_texA atIndex:1];
            NSUInteger tw = s_psoBand.threadExecutionWidth;
            NSUInteger th = s_psoBand.maxTotalThreadsPerThreadgroup / tw;
            [enc dispatchThreads:grid threadsPerThreadgroup:MTLSizeMake(tw, th, 1)];
            [enc endEncoding];
        }

        // Pass 4 — vertical luma bandwidth (texA → texB)
        // SKIPPED when interlace is active — vertical filtering blends
        // adjacent lines from different fields, destroying the comb pattern.
        // The horizontal bandwidth pass already provides the VHS softness.
        id<MTLTexture> finalSrcTex = s_texB;
        if (params.interlaceMode == 0) {
            {
                id<MTLComputeCommandEncoder> enc = [cmd computeCommandEncoder];
                [enc setComputePipelineState:s_psoVert];
                [enc setBytes:&params length:sizeof(VhsParams) atIndex:2];
                [enc setTexture:s_texA atIndex:0];
                [enc setTexture:s_texB atIndex:1];
                NSUInteger tw = s_psoVert.threadExecutionWidth;
                NSUInteger th = s_psoVert.maxTotalThreadsPerThreadgroup / tw;
                [enc dispatchThreads:grid threadsPerThreadgroup:MTLSizeMake(tw, th, 1)];
                [enc endEncoding];
            }
        } else {
            finalSrcTex = s_texA;  // skip pass 4, final reads from texA
        }

        // Pass 5 — final colour (reads finalSrcTex, writes RGBA dst)
        {
            id<MTLComputeCommandEncoder> enc = [cmd computeCommandEncoder];
            [enc setComputePipelineState:s_psoFinal];
            [enc setBuffer:dst offset:0 atIndex:1];
            [enc setBytes:&params length:sizeof(VhsParams) atIndex:2];
            [enc setBuffer:ocst offset:0 atIndex:3];
            [enc setTexture:finalSrcTex atIndex:0];
            NSUInteger tw = s_psoFinal.threadExecutionWidth;
            NSUInteger th = s_psoFinal.maxTotalThreadsPerThreadgroup / tw;
            [enc dispatchThreads:grid threadsPerThreadgroup:MTLSizeMake(tw, th, 1)];
            [enc endEncoding];
        }

        [cmd commit];
        if (waitForCompletion) [cmd waitUntilCompleted];
    }
}

#endif // __APPLE__
