// ============================================================
// Lofi FX VHS — shared GPU kernel source.
//
// THIS FILE IS THE SINGLE SOURCE OF TRUTH FOR ALL VHS SHADER MATH.
// It is written against a small abstraction-macro layer so the same
// text compiles on both Metal (as a runtime-compiled string) and
// CUDA (as a .cu file #include).
//
// HOW IT IS CONSUMED
//   Metal : CMake embeds this file's text into VhsKernelCoreText.h
//           as a C++ string literal. MetalKernelSource.h concatenates
//           a Metal-specific PREFIX (defining the abstraction macros
//           for MSL) + this text + a Metal-specific SUFFIX (the three
//           MSL kernel entry points that forward to the DEVFN bodies
//           defined here). The final string is handed to
//           MTLDevice newLibraryWithSource.
//
//   CUDA  : A future CudaKernels.cu file does
//              #include "CudaKernelSupport.h"   // helper_math, macros
//              #include "VhsKernelCore.inc"
//              #include "CudaKernelEntries.h"  // __global__ wrappers
//           and compiles via nvcc normally.
//
// REQUIRED MACROS (must be defined by the platform wrapper before
// this file is consumed):
//   DEVFN              qualifier for device-scope inline functions
//                      (Metal: `inline`, CUDA: `__device__ inline`)
//   GLOBAL_RO(T)       read-only global memory pointer type
//                      (Metal: `device const T*`,
//                       CUDA:  `const T* __restrict__`)
//   GLOBAL_RW(T)       read/write global memory pointer type
//                      (Metal: `device T*`,
//                       CUDA:  `T* __restrict__`)
//   CONSTANT_REF(T)    small constant-memory parameter reference
//                      (Metal: `constant T&`,
//                       CUDA:  `const T&`)
//   READ_TEX           sampleable 2D float4 texture type
//   WRITE_TEX          writeable 2D float4 texture type
//   TEX_SAMPLE(t,u,v)  sample a READ_TEX at normalised UV, returns float4
//   TEX_WRITE(t,x,y,v) write a float4 to a WRITE_TEX at integer pixel
//   MAKE_FLOAT3(a,b,c) / MAKE_FLOAT4(a,b,c,d) / MAKE_INT2 / MAKE_INT3
//   MAKE_FLOAT3_FROM_INT3(v)  int3 -> float3 componentwise
//   EXP2 / LOG2        highest-precision exp2/log2 available
//
// In addition, this file assumes float3/int3/float4 operator overloads
// exist (vector add/sub/mul, scalar broadcast, min/max/clamp). Metal
// provides these natively; on CUDA, include a helper_math-equivalent
// header before this one.
//
// DO NOT add Metal-specific (`precise::`, `.sample(`, `access::write`,
// `kernel void`, `[[buffer(N)]]`, `device`, `constant`) or CUDA-specific
// (`__device__`, `tex2D`, `surf2Dwrite`) tokens directly to this file.
// Use the abstraction macros above. If you need a new abstraction,
// add it to BOTH platform prefixes and document it in the list above.
// ============================================================

// ============================================================
// Shared param struct — MUST match VhsParams.h exactly.
// (VhsParams.h is the host-side version; this one is the device-side
// mirror. Compiler pads both identically because all fields are
// naturally-aligned int or float.)
// ============================================================
struct VhsParams {
    int   width;
    int   height;
    int   srcRowBytes;
    int   dstRowBytes;
    float renderScale;
    int   frame_index;
    int   seed_offset;

    int   tvStandard;
    float bandwidthLines;
    float bandwidthRows;
    float chromaBandwidth;
    int   palSwitch;

    float compositeStrength;
    float dotCrawlGain;
    float crossColorGain;
    float compositeNoise;

    float lumaNoise;
    float chromaNoise;
    float noiseTintHue;

    int   interlaceMode;
    float interlaceAmount;
    int   fieldOrder;

    int   headSwitchEnable;
    float headSwitchHeight;
    float headSwitchIntensity;
    int   trackingErrorEnable;
    float trackingErrorAmount;
    float trackingErrorSpeed;
    float trackingErrorHeight;
    float trackingErrorOpacity;
    float dropoutDensity;
    float dropoutLength;

    float lineJitterPx;
    float topSkewPx;
    float topSkewDecay;
    float edgePincushion;

    float colorFade;
    float hueTint;
    float blackLift;
    float whiteKnee;
    float dynamicRangeStops;

    int   osdEnable;
    int   osdElements;
    int   osdPositionDate;
    int   osdPositionTime;
    int   osdPositionRec;
    int   osdPositionMode;
    int   osdPositionCounter;
    int   osdPositionSpeed;
    int   osdGlyphSet;
    int   osdShadow;
    float osdScale;
    int   osdPlayMode;
    int   osdSpeedLabel;
    int   osdRecBlink;
    int   osdStartYear;
    int   osdStartMonth;
    int   osdStartDay;
    int   osdStartHour;
    int   osdStartMinute;
    int   osdStartSecond;
    int   osdDateFormat;

    int   inTransform;
    int   inputCstLutSize;
    int   outTransform;
    int   cstLutSize;

    // ---- Dropout height ----
    float dropoutHeight;

    // ---- Dropout softness / opacity ----
    float dropoutOpacity;
    float dropoutSoftness;

    // ---- Channel registration ----
    float chromaOffsetR;
    float chromaOffsetB;

    // ---- Mist / CRT bloom ----
    int   mistEnabled;
    float mistAmount;
    float mistSpread;
    float mistThreshold;
    float mistVeil;
};

// ============================================================
// Pixel read helpers (RGBA float32 interleaved, row-stride in bytes)
// ============================================================
DEVFN float readPx(GLOBAL_RO(float) buf, int x, int y,
                   int w, int h, int rowBytes, int comp) {
    x = clamp(x, 0, w - 1);
    y = clamp(y, 0, h - 1);
    int floatsPerRow = rowBytes / 4;
    return buf[y * floatsPerRow + x * 4 + comp];
}

DEVFN float readPxBilinear(GLOBAL_RO(float) buf, float fx, float fy,
                           int w, int h, int rowBytes, int comp) {
    int x0 = int(floor(fx)); int x1 = x0 + 1;
    int y0 = int(floor(fy)); int y1 = y0 + 1;
    float tx = fx - float(x0);
    float ty = fy - float(y0);
    float v00 = readPx(buf, x0, y0, w, h, rowBytes, comp);
    float v10 = readPx(buf, x1, y0, w, h, rowBytes, comp);
    float v01 = readPx(buf, x0, y1, w, h, rowBytes, comp);
    float v11 = readPx(buf, x1, y1, w, h, rowBytes, comp);
    float a = v00 * (1.0f - tx) + v10 * tx;
    float b = v01 * (1.0f - tx) + v11 * tx;
    return a * (1.0f - ty) + b * ty;
}

DEVFN void writePx(GLOBAL_RW(float) buf, int x, int y, int rowBytes,
                   float r, float g, float b, float a) {
    int floatsPerRow = rowBytes / 4;
    int idx = y * floatsPerRow + x * 4;
    buf[idx + 0] = r;
    buf[idx + 1] = g;
    buf[idx + 2] = b;
    buf[idx + 3] = a;
}

// ============================================================
// PRNG (FNV-1a + xxhash finalizer) — identical to Film plugin
// for reproducibility across plugins.
// ============================================================
DEVFN uint vhs_hash(uint seed) {
    seed ^= seed >> 15u;
    seed *= 0x2C1B3C6Du;
    seed ^= seed >> 12u;
    seed *= 0x297A2D39u;
    seed ^= seed >> 15u;
    return seed;
}

DEVFN uint vhs_cell_seed(int cx, int cy, uint offset) {
    uint h = offset ^ 0x811C9DC5u;
    h = (h ^ (uint(cx) & 0xFFFFu)) * 0x01000193u;
    h = (h ^ (uint(cy) & 0xFFFFu)) * 0x01000193u;
    return vhs_hash(h);
}

// Uniform float in [0, 1) from a hash
DEVFN float vhs_uniform(uint h) {
    return float(h) * 2.3283064365e-10f;
}

// Box-Muller white-Gaussian noise from two uniforms
DEVFN float vhs_gauss(uint h1, uint h2) {
    float u1 = max(vhs_uniform(h1), 1e-7f);
    float u2 = vhs_uniform(h2);
    return sqrt(-2.0f * log(u1)) * cos(6.2831853f * u2);
}

// Clamp a gauss sample to a sane range
DEVFN float gauss_clamp(float g) { return clamp(g, -3.0f, 3.0f); }

// ============================================================
// 3D LUT — tetrahedral interpolation
//
// Written with explicit component access rather than int3/float3
// scalar-broadcast arithmetic so this compiles identically on Metal
// and on CUDA (whose built-in int3/float3 do not support the terse
// `min(lo+1, hi)` scalar-broadcast form unless helper_math patches
// them).
// ============================================================
DEVFN float3 applyLut3D(GLOBAL_RO(float) lutData, int lutSize,
                        float3 color) {
    float cx = clamp(color.x, 0.0f, 1.0f);
    float cy = clamp(color.y, 0.0f, 1.0f);
    float cz = clamp(color.z, 0.0f, 1.0f);
    float sF = float(lutSize - 1);
    float sx = cx * sF, sy = cy * sF, sz = cz * sF;

    int3 lo = MAKE_INT3(int(floor(sx)), int(floor(sy)), int(floor(sz)));
    int3 hi = MAKE_INT3(
        min(lo.x + 1, lutSize - 1),
        min(lo.y + 1, lutSize - 1),
        min(lo.z + 1, lutSize - 1));
    float3 f = MAKE_FLOAT3(sx - float(lo.x),
                           sy - float(lo.y),
                           sz - float(lo.z));

    int s = lutSize;
    int s2 = s * s;
    #define LUT_IDX(ri, gi, bi) ((bi) * s2 + (gi) * s + (ri)) * 3

    int idx000 = LUT_IDX(lo.x, lo.y, lo.z);
    int idx111 = LUT_IDX(hi.x, hi.y, hi.z);
    float3 c000 = MAKE_FLOAT3(lutData[idx000], lutData[idx000+1], lutData[idx000+2]);
    float3 c111 = MAKE_FLOAT3(lutData[idx111], lutData[idx111+1], lutData[idx111+2]);

    float3 r;
    if (f.x > f.y) {
        if (f.y > f.z) {
            int i100 = LUT_IDX(hi.x, lo.y, lo.z);
            int i110 = LUT_IDX(hi.x, hi.y, lo.z);
            float3 c100 = MAKE_FLOAT3(lutData[i100], lutData[i100+1], lutData[i100+2]);
            float3 c110 = MAKE_FLOAT3(lutData[i110], lutData[i110+1], lutData[i110+2]);
            r = (1.0f - f.x) * c000 + (f.x - f.y) * c100 + (f.y - f.z) * c110 + f.z * c111;
        } else if (f.x > f.z) {
            int i100 = LUT_IDX(hi.x, lo.y, lo.z);
            int i101 = LUT_IDX(hi.x, lo.y, hi.z);
            float3 c100 = MAKE_FLOAT3(lutData[i100], lutData[i100+1], lutData[i100+2]);
            float3 c101 = MAKE_FLOAT3(lutData[i101], lutData[i101+1], lutData[i101+2]);
            r = (1.0f - f.x) * c000 + (f.x - f.z) * c100 + (f.z - f.y) * c101 + f.y * c111;
        } else {
            int i001 = LUT_IDX(lo.x, lo.y, hi.z);
            int i101 = LUT_IDX(hi.x, lo.y, hi.z);
            float3 c001 = MAKE_FLOAT3(lutData[i001], lutData[i001+1], lutData[i001+2]);
            float3 c101 = MAKE_FLOAT3(lutData[i101], lutData[i101+1], lutData[i101+2]);
            r = (1.0f - f.z) * c000 + (f.z - f.x) * c001 + (f.x - f.y) * c101 + f.y * c111;
        }
    } else {
        if (f.z > f.y) {
            int i001 = LUT_IDX(lo.x, lo.y, hi.z);
            int i011 = LUT_IDX(lo.x, hi.y, hi.z);
            float3 c001 = MAKE_FLOAT3(lutData[i001], lutData[i001+1], lutData[i001+2]);
            float3 c011 = MAKE_FLOAT3(lutData[i011], lutData[i011+1], lutData[i011+2]);
            r = (1.0f - f.z) * c000 + (f.z - f.y) * c001 + (f.y - f.x) * c011 + f.x * c111;
        } else if (f.z > f.x) {
            int i010 = LUT_IDX(lo.x, hi.y, lo.z);
            int i011 = LUT_IDX(lo.x, hi.y, hi.z);
            float3 c010 = MAKE_FLOAT3(lutData[i010], lutData[i010+1], lutData[i010+2]);
            float3 c011 = MAKE_FLOAT3(lutData[i011], lutData[i011+1], lutData[i011+2]);
            r = (1.0f - f.y) * c000 + (f.y - f.z) * c010 + (f.z - f.x) * c011 + f.x * c111;
        } else {
            int i010 = LUT_IDX(lo.x, hi.y, lo.z);
            int i110 = LUT_IDX(hi.x, hi.y, lo.z);
            float3 c010 = MAKE_FLOAT3(lutData[i010], lutData[i010+1], lutData[i010+2]);
            float3 c110 = MAKE_FLOAT3(lutData[i110], lutData[i110+1], lutData[i110+2]);
            r = (1.0f - f.y) * c000 + (f.y - f.x) * c010 + (f.x - f.z) * c110 + f.z * c111;
        }
    }
    #undef LUT_IDX
    return r;
}

// ============================================================
// DaVinci Intermediate <-> scene linear (must match Film plugin)
// ============================================================
DEVFN float di_to_linear(float V) {
    const float A = 0.0075f, B = 7.0f, C = 0.07329248f, M = 10.44426855f;
    const float LOG_CUT = 0.02740668f;
    if (V > LOG_CUT) return EXP2(V / C - B) - A;
    else             return V / M;
}

DEVFN float linear_to_di(float L) {
    const float A = 0.0075f, B = 7.0f, C = 0.07329248f, M = 10.44426855f;
    const float LIN_CUT = 0.00262409f;
    if (L > LIN_CUT) return C * (LOG2(L + A) + B);
    else             return L * M;
}

DEVFN float3 di_to_linear3(float3 v) {
    return MAKE_FLOAT3(di_to_linear(v.x), di_to_linear(v.y), di_to_linear(v.z));
}
DEVFN float3 linear_to_di3(float3 v) {
    return MAKE_FLOAT3(linear_to_di(v.x), linear_to_di(v.y), linear_to_di(v.z));
}

// ============================================================
// DWG-linear <-> Rec.709-linear
// Resolve's DaVinci Wide Gamut -> Rec.709 matrix (D65, scene-referred).
// ============================================================
DEVFN float3 dwg_to_rec709(float3 c) {
    return MAKE_FLOAT3(
         1.5163789f * c.x - 0.2814771f * c.y - 0.2349017f * c.z,
        -0.0220062f * c.x + 1.0457783f * c.y - 0.0237721f * c.z,
        -0.0083079f * c.x + 0.0252800f * c.y + 0.9830279f * c.z);
}
DEVFN float3 rec709_to_dwg(float3 c) {
    return MAKE_FLOAT3(
         0.6638648f * c.x + 0.1828851f * c.y + 0.1532501f * c.z,
         0.0140785f * c.x + 0.9636411f * c.y + 0.0222804f * c.z,
         0.0052974f * c.x - 0.0263391f * c.y + 1.0210417f * c.z);
}

// ============================================================
// Rec.709 linear <-> Rec.709 gamma-encoded (pure gamma 2.4)
// ============================================================
DEVFN float lin_to_v(float x) {
    x = max(x, 0.0f);
    return pow(x, 1.0f / 2.4f);
}
DEVFN float v_to_lin(float x) {
    x = max(x, 0.0f);
    return pow(x, 2.4f);
}
DEVFN float3 lin_to_v3(float3 c) {
    return MAKE_FLOAT3(lin_to_v(c.x), lin_to_v(c.y), lin_to_v(c.z));
}
DEVFN float3 v_to_lin3(float3 c) {
    return MAKE_FLOAT3(v_to_lin(c.x), v_to_lin(c.y), v_to_lin(c.z));
}

// ============================================================
// Rec.709-gamma RGB <-> Y'CbCr (BT.601 matrix — period-correct)
// Cb/Cr centred around 0 (full-range).
// ============================================================
DEVFN float3 rgb_to_ycbcr601(float3 c) {
    float y  =  0.299f    * c.x + 0.587f    * c.y + 0.114f    * c.z;
    float cb = -0.168736f * c.x - 0.331264f * c.y + 0.5f      * c.z;
    float cr =  0.5f      * c.x - 0.418688f * c.y - 0.081312f * c.z;
    return MAKE_FLOAT3(y, cb, cr);
}
DEVFN float3 ycbcr601_to_rgb(float3 c) {
    float r = c.x +                  1.402f    * c.z;
    float g = c.x - 0.344136f * c.y - 0.714136f * c.z;
    float b = c.x + 1.772f    * c.y;
    return MAKE_FLOAT3(r, g, b);
}

// ============================================================
// 9-position OSD origin anchor
// ============================================================
DEVFN int2 osd_origin(int pos, int w, int h, int gw, int gh) {
    int margin = max(min(w, h) / 64, 4);
    int col = pos % 3;
    int row = pos / 3;
    int x = (col == 0) ? margin
          : (col == 1) ? (w / 2 - gw / 2)
          : (w - gw - margin);
    int y = (row == 0) ? margin
          : (row == 1) ? (h / 2 - gh / 2)
          : (h - gh - margin);
    return MAKE_INT2(x, y);
}

// ============================================================
// Pass 1 body — TAPE SIGNAL GENERATION
//
// Per-pixel signal-chain pipeline (no neighbor blurs — bandwidth
// limit happens in Pass 2 and smears all of this together):
//   RGBA -> CST -> DWG-linear -> Rec.709 linear -> gamma -> Y'CbCr
//   OSD burn-in (so OSD also picks up dot-crawl + bandwidth blur)
//   Composite encode/decode (dot crawl, cross color, composite noise)
//   Magnetic luma / chroma noise (coarse cell sizes -> visible "tape grain")
//   Dropout impulses (white spikes — Pass 2 then makes them slightly soft)
//
// Writes a float4 (Y', Cb, Cr, alpha) to outTex at (x, y).
// ============================================================

// Sample the input signal at any (xn, yn) and return the post-OSD
// Y'CbCr+alpha. Used by Pass 1 for self + horizontal neighbors so the
// composite encode/decode sees consistent OSD-burned signal everywhere.
DEVFN float4 vhs_signal_at(
    int xn, int yn,
    GLOBAL_RO(float)   srcBuf,
    CONSTANT_REF(VhsParams) p,
    GLOBAL_RO(float)   inputCstLut,
    GLOBAL_RO(uint8_t) osdMask,
    int                osdW,
    int                osdH)
{
    float3 raw = MAKE_FLOAT3(
        readPx(srcBuf, xn, yn, p.width, p.height, p.srcRowBytes, 0),
        readPx(srcBuf, xn, yn, p.width, p.height, p.srcRowBytes, 1),
        readPx(srcBuf, xn, yn, p.width, p.height, p.srcRowBytes, 2));
    float a = readPx(srcBuf, xn, yn, p.width, p.height, p.srcRowBytes, 3);

    float3 di = (p.inputCstLutSize > 0)
                  ? applyLut3D(inputCstLut, p.inputCstLutSize, raw)
                  : raw;
    float3 lin709 = dwg_to_rec709(di_to_linear3(di));
    float3 v709   = lin_to_v3(lin709);

    if (p.osdEnable && osdW > 0 && osdH > 0
        && xn >= 0 && xn < osdW && yn >= 0 && yn < osdH) {
        uint mIdx = uint(yn) * uint(osdW) + uint(xn);
        float m = float(osdMask[mIdx]) * (1.0f / 255.0f);
        if (m > 0.001f) {
            float text = (m > 0.6f) ? 1.0f : 0.0f;
            float shad = (m > 0.2f && m < 0.6f) ? 1.0f : 0.0f;
            v709 = v709 * (1.0f - text) + MAKE_FLOAT3(1.0f, 1.0f, 1.0f) * text;
            v709 = v709 * (1.0f - shad * 0.65f);
        }
    }

    float3 yuv = rgb_to_ycbcr601(v709);
    return MAKE_FLOAT4(yuv.x, yuv.y, yuv.z, a);
}

DEVFN void vhs_encode_body(
    int x, int y,
    GLOBAL_RO(float)   srcBuf,
    CONSTANT_REF(VhsParams) p,
    GLOBAL_RO(float)   inputCstLut,
    GLOBAL_RO(uint8_t) osdMask,
    int                osdW,
    int                osdH,
    WRITE_TEX          outTex)
{
    uint frameSeed = uint(p.frame_index) * 0x9E3779B1u + uint(p.seed_offset);

    // NTSC Comb — field displacement:
    // Each interlaced frame is two 240-line fields (480 lines total).
    // Field 0 reads at (x, y). Field 1 reads at (x + disp, y).
    // Integer displacement produces a sharp sawtooth on vertical edges.
    int readX = x;
    if (p.interlaceMode != 0) {
        if (((y + p.fieldOrder) & 1) == 1) {
            int disp = int(p.interlaceAmount + 0.5f);
            if (disp < 2) disp = 2;
            readX = x + disp;
        }
    }

    float4 self = vhs_signal_at(readX, y, srcBuf, p, inputCstLut, osdMask, osdW, osdH);
    float Y  = self.x;
    float Cb = self.y;
    float Cr = self.z;
    float a  = self.w;

    // ---------------- Composite encode -> decode (dot crawl / xcolor) ----
    if (p.compositeStrength > 0.001f) {
        float pxPerCycle = (p.tvStandard == 0) ? 3.77f : 3.05f;
        float twoPi = 6.2831853f;
        float lineOffset =
            (p.tvStandard == 0) ? float(y & 1) * 3.14159f
                                : (((y & 1) == 1 && p.palSwitch) ? 3.14159f : 0.0f);

        float fx = float(x) + 0.5f;
        float phase0 = twoPi * (fx       ) / pxPerCycle + lineOffset;
        float phaseM = twoPi * (fx - 1.0f) / pxPerCycle + lineOffset;
        float phaseP = twoPi * (fx + 1.0f) / pxPerCycle + lineOffset;

        // Composite neighbors also use the field-displaced position
        float4 sM = vhs_signal_at(readX - 1, y, srcBuf, p, inputCstLut, osdMask, osdW, osdH);
        float4 sP = vhs_signal_at(readX + 1, y, srcBuf, p, inputCstLut, osdMask, osdW, osdH);

        float compM = sM.x + sM.y * sin(phaseM) + sM.z * cos(phaseM);
        float comp0 = Y    + Cb   * sin(phase0) + Cr   * cos(phase0);
        float compP = sP.x + sP.y * sin(phaseP) + sP.z * cos(phaseP);

        if (p.compositeNoise > 0.001f) {
            // Horizontal-streak structure: 2 px wide cells, every line
            // independent — looks like RF-band noise that the bandwidth
            // pass will smear into the signature VHS "tape hiss".
            uint h1 = vhs_cell_seed(x >> 1, y, frameSeed ^ 0xBEEF1234u);
            uint h2 = vhs_cell_seed((x >> 1) ^ 0xA5, y, frameSeed ^ 0x12345678u);
            comp0 += gauss_clamp(vhs_gauss(h1, h2)) * p.compositeNoise * 0.15f;
        }

        float Y_dec  = (compM + comp0 + compP) / 3.0f;
        float C_res  = comp0 - Y_dec;
        float cP     = cos(phase0);
        float sP_v   = sin(phase0);
        float Cb_dec = 2.0f * C_res * sP_v;
        float Cr_dec = 2.0f * C_res * cP;

        float t = clamp(p.compositeStrength, 0.0f, 1.0f);
        float lumaHF = Y - Y_dec;
        Cb += lumaHF * p.dotCrawlGain * sP_v * t;
        Cr += lumaHF * p.dotCrawlGain * cP   * t;
        Y   = Y  * (1.0f - t) + (Y_dec + (Y - Y_dec) * (1.0f - p.crossColorGain)) * t;
        Cb  = Cb * (1.0f - t * 0.3f) + (Cb * 0.5f + Cb_dec * 0.5f) * (t * 0.3f);
        Cr  = Cr * (1.0f - t * 0.3f) + (Cr * 0.5f + Cr_dec * 0.5f) * (t * 0.3f);
    }

    // ---------------- Magnetic noise (blocky) ----------------------------
    // Coarsen cells so the noise survives the bandwidth low-pass and
    // looks like real tape grain rather than per-pixel digital noise.
    // In interlace mode 3 (NTSC Comb), each field gets independent noise
    // seeds so that Field 0 and Field 1 exhibit different noise — as they
    // would in true interlace capture where fields are recorded at
    // different times from different tape passes.
    {
        uint noiseSeedBase = frameSeed;
        if (p.interlaceMode != 0) {
            // XOR with field parity to give each field its own noise
            noiseSeedBase ^= (y & 1) ? 0xA5A5A5A5u : 0x5A5A5A5Au;
        }
        if (p.lumaNoise > 0.001f) {
            uint h1 = vhs_cell_seed(x >> 1, y >> 1, noiseSeedBase ^ 0xCAFEBABEu);
            uint h2 = vhs_cell_seed((x >> 1) + 17, (y >> 1) - 23, noiseSeedBase ^ 0xDEADBEEFu);
            Y += gauss_clamp(vhs_gauss(h1, h2)) * p.lumaNoise * 0.10f;
        }
        if (p.chromaNoise > 0.001f) {
            uint h1 = vhs_cell_seed(x >> 2, y >> 2, noiseSeedBase ^ 0x55667788u);
            uint h2 = vhs_cell_seed((x >> 2) + 91, (y >> 2) + 17, noiseSeedBase ^ 0x99AABBCCu);
            float n1 = gauss_clamp(vhs_gauss(h1, h2));
            float h_rad = p.noiseTintHue * 3.14159f / 180.0f;
            Cb += n1 * p.chromaNoise * 0.10f * cos(h_rad);
            Cr += n1 * p.chromaNoise * 0.10f * sin(h_rad);
        }
    }

    // ---------------- Dropouts (white impulses on raw signal) -----------
    // Placed BEFORE bandwidth so Pass 2 softens the edges slightly,
    // matching the look of a real dropout flash.
    if (p.dropoutDensity > 0.001f) {
        // Quantize y to block height so all rows in the same stripe share
        // the same dropout decision — produces realistic multi-line tall
        // dropouts controlled by the Dropout Height parameter.
        int heightPx = max((int)(p.dropoutHeight * p.renderScale + 0.5f), 1);
        int y_base   = (y / heightPx) * heightPx;
        uint hL = vhs_cell_seed(0, y_base * 37, frameSeed ^ 0xABCDEF01u);
        float u = vhs_uniform(hL);
        float pPerBlock = clamp(p.dropoutDensity / max(float(p.height), 1.0f), 0.0f, 1.0f);
        if (u < pPerBlock * 4.0f) {
            uint hX   = vhs_cell_seed(1, y_base, frameSeed ^ 0x13579BDFu);
            uint hLen = vhs_cell_seed(2, y_base, frameSeed ^ 0x2468ACE0u);
            float startX = vhs_uniform(hX) * float(p.width);
            float length_ = max(p.dropoutLength, 4.0f) * p.renderScale * (0.5f + vhs_uniform(hLen));
            float dx = float(x) - startX;
            if (dx > 0.0f && dx < length_) {
                float edgeDist = min(dx, length_ - dx);
                float soft = clamp(edgeDist / max(p.dropoutSoftness, 1e-3f), 0.0f, 1.0f);
                float alpha = clamp(p.dropoutOpacity, 0.0f, 1.0f) * soft;
                Y  = Y  * (1.0f - alpha) + 1.0f * alpha;
                Cb = Cb * (1.0f - alpha);
                Cr = Cr * (1.0f - alpha);
            }
        }
    }

    // ---------------- Head-switch noise band (bottom) -------------------
    // Moved here from Pass 3 so the bandwidth low-pass softens its hard
    // per-pixel structure into the signature smeared head-switch flicker.
    if (p.headSwitchEnable && p.headSwitchIntensity > 0.001f) {
        float bandTop = float(p.height) - max(p.headSwitchHeight, 1.0f) * p.renderScale;
        if (float(y) > bandTop) {
            float t = clamp((float(y) - bandTop) / max(p.headSwitchHeight * p.renderScale, 1.0f), 0.0f, 1.0f);
            uint hN  = vhs_cell_seed(x,       y, frameSeed ^ 0xF1F2F3F4u);
            uint hN2 = vhs_cell_seed(x + 31,  y, frameSeed ^ 0x0F1E2D3Cu);
            float n = vhs_gauss(hN, hN2);
            Y  += n * p.headSwitchIntensity * t * 0.4f;
            Cb += n * p.headSwitchIntensity * t * 0.2f;
            Cr += n * p.headSwitchIntensity * t * 0.2f;
        }
    }

    // ---------------- Tracking-band heavy in-band noise ------------------
    // Moved here from Pass 3 so it gets bandwidth-softened. Note the
    // geometric jitter portion of the tracking error remains in Pass 3
    // because it warps the sampling position.
    if (p.trackingErrorEnable && p.trackingErrorAmount > 0.001f) {
        float halfH = max(p.trackingErrorHeight, 1.0f);
        float bandY = fmod(float(p.frame_index) * p.trackingErrorSpeed,
                           float(p.height) + halfH * 2.0f);
        float dist = float(y) - bandY;
        if (dist > -halfH && dist < halfH) {
            float trackPhase = 1.0f - clamp(fabs(dist) / halfH, 0.0f, 1.0f);
            float opa = p.trackingErrorOpacity;
            uint h1 = vhs_cell_seed(x,     y, frameSeed ^ 0x77777777u);
            uint h2 = vhs_cell_seed(x + 5, y, frameSeed ^ 0x88888888u);
            Y  += gauss_clamp(vhs_gauss(h1, h2)) * 0.25f * trackPhase * p.trackingErrorAmount * opa;
            Cb *= (1.0f - 0.5f * trackPhase * p.trackingErrorAmount * opa);
            Cr *= (1.0f - 0.5f * trackPhase * p.trackingErrorAmount * opa);
        }
    }

    TEX_WRITE(outTex, x, y, MAKE_FLOAT4(Y, Cb, Cr, a));
}

// ============================================================
// Pass 2 body — BANDWIDTH LIMIT
//
// Separable horizontal low-pass. Luma gets a moderate Gaussian sized
// for bandwidthLines; chroma gets a much wider blur sized for
// chromaBandwidth. This is the signature VHS effect.
//
//     sigma_px = horizontalPixels / (pi * targetLines)
//
// Tap budget caps at 33 per side (67 total).
// ============================================================
DEVFN void vhs_bandwidth_body(
    int x, int y,
    CONSTANT_REF(VhsParams) p,
    READ_TEX  inTex,
    WRITE_TEX outTex)
{
    float fullW = float(p.width) / max(p.renderScale, 1e-3f);
    float pxScale = float(p.width) / max(fullW, 1.0f);

    float sigmaY = (3.14159f * max(p.bandwidthLines, 40.0f));
    sigmaY = float(p.width) / sigmaY * pxScale;
    float sigmaC = (3.14159f * max(p.chromaBandwidth, 10.0f));
    sigmaC = float(p.width) / sigmaC * pxScale;

    sigmaY = clamp(sigmaY, 0.5f, 60.0f);
    sigmaC = clamp(sigmaC, 0.5f, 80.0f);

    int radiusY = int(min(ceil(sigmaY * 3.0f), 33.0f));
    int radiusC = int(min(ceil(sigmaC * 3.0f), 33.0f));
    int radius  = max(radiusY, radiusC);

    float invY = -0.5f / (sigmaY * sigmaY);
    float invC = -0.5f / (sigmaC * sigmaC);

    float sumY = 0.0f, wY = 0.0f;
    float sumCb = 0.0f, sumCr = 0.0f, wC = 0.0f;
    float a = 0.0f;

    float invW = 1.0f / float(p.width);
    float invH = 1.0f / float(p.height);

    for (int dx = -radius; dx <= radius; ++dx) {
        float fx = float(x + dx) + 0.5f;
        float fy = float(y) + 0.5f;
        float4 s4 = TEX_SAMPLE(inTex, fx * invW, fy * invH);

        if (dx >= -radiusY && dx <= radiusY) {
            float w = exp(invY * float(dx) * float(dx));
            sumY += s4.x * w; wY += w;
        }
        if (dx >= -radiusC && dx <= radiusC) {
            float w = exp(invC * float(dx) * float(dx));
            sumCb += s4.y * w;
            sumCr += s4.z * w;
            wC += w;
        }
        if (dx == 0) a = s4.w;
    }

    float Y  = (wY > 0.0f) ? sumY / wY : 0.0f;
    float Cb = (wC > 0.0f) ? sumCb / wC : 0.0f;
    float Cr = (wC > 0.0f) ? sumCr / wC : 0.0f;

    TEX_WRITE(outTex, x, y, MAKE_FLOAT4(Y, Cb, Cr, a));
}

// ============================================================
// Pass 2b body — VERTICAL LUMA BANDWIDTH (rows)
//
// Separable vertical Gaussian low-pass on the luma channel only.
// Simulates reduced vertical resolution — fewer effective scan
// rows — complementing the horizontal bandwidth_lines pass.
//
//     sigma_px = height / (pi * targetRows)
// ============================================================
DEVFN void vhs_bandwidth_rows_body(
    int x, int y,
    CONSTANT_REF(VhsParams) p,
    READ_TEX  inTex,
    WRITE_TEX outTex)
{
    float fullH = float(p.height) / max(p.renderScale, 1e-3f);
    float pxScale = float(p.height) / max(fullH, 1.0f);

    float sigmaY = (3.14159f * max(p.bandwidthRows, 20.0f));
    sigmaY = float(p.height) / sigmaY * pxScale;

    sigmaY = clamp(sigmaY, 0.5f, 60.0f);

    int radiusY = int(min(ceil(sigmaY * 3.0f), 33.0f));

    float invY = -0.5f / (sigmaY * sigmaY);

    float sumY = 0.0f, wY = 0.0f;
    float Cb = 0.0f, Cr = 0.0f, a = 0.0f;

    float invW = 1.0f / float(p.width);
    float invH = 1.0f / float(p.height);

    for (int dy = -radiusY; dy <= radiusY; ++dy) {
        float fx = float(x) + 0.5f;
        float fy = float(y + dy) + 0.5f;
        float4 s4 = TEX_SAMPLE(inTex, fx * invW, fy * invH);

        float w = exp(invY * float(dy) * float(dy));
        sumY += s4.x * w; wY += w;
        if (dy == 0) { Cb = s4.y; Cr = s4.z; a = s4.w; }
    }

    float Y  = (wY > 0.0f) ? sumY / wY : 0.0f;

    TEX_WRITE(outTex, x, y, MAKE_FLOAT4(Y, Cb, Cr, a));
}

// ============================================================
// VHS CRT / Lens Bloom — wide atmospheric glow on video highlights.
//
// Wider and softer than the Film plugin mist: uses a pure Gaussian
// with sigma = spread (vs. 0.35×spread in Film), so the glow extends
// much further at the same spread value — appropriate for the
// diffuse CRT phosphor / dirty-lens look of VHS playback.
//
// Samples the bandwidth-limited YCbCr texture, converts each tap to
// Rec.709 gamma, and extracts highlights above `threshold`.
// Returns float4(bloom_rgb, mean_brightness) — caller uses mean_brightness
// for the optional veiling-glare shadow lift.
// ============================================================
DEVFN float3 vhs_bloom_highlight(float3 c, float thr) {
    float3 d = max(c - MAKE_FLOAT3(thr, thr, thr), MAKE_FLOAT3(0.0f, 0.0f, 0.0f));
    return d * d;   // super-linear: brights dominate
}

DEVFN float4 vhs_mist_bloom(
    READ_TEX inTex,
    float fx, float fy, float invW, float invH,
    float spread, float threshold)
{
    if (spread < 0.25f)
        return MAKE_FLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

    // Wide Gaussian: sigma = spread  (Film mist uses sigma = 0.35×spread)
    float sigma   = max(spread, 1.0f);
    float inv2s2  = -0.5f / (sigma * sigma);

    // Ring distances in pixel units: 0, 0.5, 1, 2, 4, 8 × spread
    const float ring_r2[6] = {
        0.0f,
        0.25f  * spread * spread,
        1.0f   * spread * spread,
        4.0f   * spread * spread,
        16.0f  * spread * spread,
        64.0f  * spread * spread
    };
    float w_ring[6];
    for (int i = 0; i < 6; ++i)
        w_ring[i] = exp(inv2s2 * ring_r2[i]);
    float wsum = w_ring[0] + 8.0f * (w_ring[1] + w_ring[2] + w_ring[3] + w_ring[4] + w_ring[5]);

    float3 acc  = MAKE_FLOAT3(0.0f, 0.0f, 0.0f);
    float  vacc = 0.0f;

#define VHS_BLOOM_TAP(nx, ny, RING) { \
    float4 s_ = TEX_SAMPLE(inTex, (fx + (nx) * spread) * invW, \
                                  (fy + (ny) * spread) * invH); \
    float3 rgb_ = ycbcr601_to_rgb(MAKE_FLOAT3(s_.x, s_.y, s_.z)); \
    acc  = acc + vhs_bloom_highlight(rgb_, threshold) * w_ring[RING]; \
    vacc += max(max(rgb_.x, rgb_.y), rgb_.z) * w_ring[RING]; \
}

    // Center
    VHS_BLOOM_TAP(0.0f, 0.0f, 0)
    // Ring 0.5
    VHS_BLOOM_TAP( 0.5f,  0.0f, 1)  VHS_BLOOM_TAP(-0.5f,  0.0f, 1)
    VHS_BLOOM_TAP( 0.0f,  0.5f, 1)  VHS_BLOOM_TAP( 0.0f, -0.5f, 1)
    VHS_BLOOM_TAP( 0.3535534f,  0.3535534f, 1)
    VHS_BLOOM_TAP(-0.3535534f,  0.3535534f, 1)
    VHS_BLOOM_TAP( 0.3535534f, -0.3535534f, 1)
    VHS_BLOOM_TAP(-0.3535534f, -0.3535534f, 1)
    // Ring 1
    VHS_BLOOM_TAP( 1.0f,  0.0f, 2)  VHS_BLOOM_TAP(-1.0f,  0.0f, 2)
    VHS_BLOOM_TAP( 0.0f,  1.0f, 2)  VHS_BLOOM_TAP( 0.0f, -1.0f, 2)
    VHS_BLOOM_TAP( 0.7071068f,  0.7071068f, 2)
    VHS_BLOOM_TAP(-0.7071068f,  0.7071068f, 2)
    VHS_BLOOM_TAP( 0.7071068f, -0.7071068f, 2)
    VHS_BLOOM_TAP(-0.7071068f, -0.7071068f, 2)
    // Ring 2
    VHS_BLOOM_TAP( 2.0f,  0.0f, 3)  VHS_BLOOM_TAP(-2.0f,  0.0f, 3)
    VHS_BLOOM_TAP( 0.0f,  2.0f, 3)  VHS_BLOOM_TAP( 0.0f, -2.0f, 3)
    VHS_BLOOM_TAP( 1.4142136f,  1.4142136f, 3)
    VHS_BLOOM_TAP(-1.4142136f,  1.4142136f, 3)
    VHS_BLOOM_TAP( 1.4142136f, -1.4142136f, 3)
    VHS_BLOOM_TAP(-1.4142136f, -1.4142136f, 3)
    // Ring 4
    VHS_BLOOM_TAP( 4.0f,  0.0f, 4)  VHS_BLOOM_TAP(-4.0f,  0.0f, 4)
    VHS_BLOOM_TAP( 0.0f,  4.0f, 4)  VHS_BLOOM_TAP( 0.0f, -4.0f, 4)
    VHS_BLOOM_TAP( 2.8284271f,  2.8284271f, 4)
    VHS_BLOOM_TAP(-2.8284271f,  2.8284271f, 4)
    VHS_BLOOM_TAP( 2.8284271f, -2.8284271f, 4)
    VHS_BLOOM_TAP(-2.8284271f, -2.8284271f, 4)
    // Ring 8
    VHS_BLOOM_TAP( 8.0f,  0.0f, 5)  VHS_BLOOM_TAP(-8.0f,  0.0f, 5)
    VHS_BLOOM_TAP( 0.0f,  8.0f, 5)  VHS_BLOOM_TAP( 0.0f, -8.0f, 5)
    VHS_BLOOM_TAP( 5.6568542f,  5.6568542f, 5)
    VHS_BLOOM_TAP(-5.6568542f,  5.6568542f, 5)
    VHS_BLOOM_TAP( 5.6568542f, -5.6568542f, 5)
    VHS_BLOOM_TAP(-5.6568542f, -5.6568542f, 5)

#undef VHS_BLOOM_TAP

    float inv_w = (wsum > 0.0f) ? 1.0f / wsum : 0.0f;
    return MAKE_FLOAT4(acc.x * inv_w, acc.y * inv_w, acc.z * inv_w, vacc * inv_w);
}

// ============================================================
// Pass 2 body — GEOMETRIC WARP + CHROMA REGISTRATION
//
// Takes the encoded Y'CbCr from Pass 1 and applies per-pixel
// coordinate warping (jitter, skew, pincushion, tracking error,
// interlace field misalign) and chroma registration offsets.
//
// The warped Y'CbCr is written out so that subsequent bandwidth
// passes low-pass the *warped* signal — matching the real VHS
// playback chain where analog tape path jitter is bandwidth-
// limited by the signal path.
// ============================================================
DEVFN void vhs_warp_body(
    int x, int y,
    CONSTANT_REF(VhsParams) p,
    READ_TEX  inTex,
    WRITE_TEX outTex)
{
    uint frameSeed = uint(p.frame_index) * 0x9E3779B1u + uint(p.seed_offset);

    float fx = float(x) + 0.5f;
    float fy = float(y) + 0.5f;

    if (p.lineJitterPx > 0.001f) {
        uint h = vhs_cell_seed(0, y, frameSeed);
        float n = vhs_uniform(h) * 2.0f - 1.0f;
        fx += n * p.lineJitterPx * p.renderScale;
    }

    if (p.topSkewPx > 0.001f && p.topSkewDecay > 0.001f) {
        float decay = exp(-float(y) / max(p.topSkewDecay, 1.0f));
        uint h = vhs_cell_seed(7, p.frame_index, frameSeed);
        float dir = (vhs_uniform(h) > 0.5f) ? 1.0f : -1.0f;
        fx += dir * p.topSkewPx * decay * p.renderScale;
    }

    if (p.edgePincushion > 0.001f) {
        float ny = (float(y) - float(p.height) * 0.5f) / (float(p.height) * 0.5f);
        float bow = ny * ny * ny * p.edgePincushion;
        float dx_ = (fx - float(p.width) * 0.5f) * bow * 0.05f;
        fx += dx_;
    }

    if (p.trackingErrorEnable && p.trackingErrorAmount > 0.001f) {
        float halfH = max(p.trackingErrorHeight, 1.0f);
        float bandY = fmod(float(p.frame_index) * p.trackingErrorSpeed,
                           float(p.height) + halfH * 2.0f);
        float dist = float(y) - bandY;
        if (dist > -halfH && dist < halfH) {
            float trackPhase = 1.0f - clamp(fabs(dist) / halfH, 0.0f, 1.0f);
            float opa = p.trackingErrorOpacity;
            int yBlock = y & ~3;
            uint h = vhs_cell_seed(13, yBlock, frameSeed);
            float n = vhs_uniform(h) * 2.0f - 1.0f;
            fx += n * p.trackingErrorAmount * 22.0f * trackPhase * p.renderScale * opa;
        }
    }

    float invW = 1.0f / float(p.width);
    float invH = 1.0f / float(p.height);

    float4 s4 = TEX_SAMPLE(inTex, fx * invW, fy * invH);
    float Y = s4.x;
    float a = s4.w;
    float Cb, Cr;

    // Vertical chroma delay: real VHS color-under shifts chroma ~1 scanline
    // relative to luma. Always active — authentic to the format.
    float fyChroma = fy + 1.0f * p.renderScale;

    if (p.chromaOffsetB != 0.0f || p.chromaOffsetR != 0.0f) {
        float4 s4b = TEX_SAMPLE(inTex, (fx + p.chromaOffsetB * p.renderScale) * invW,
                                fyChroma * invH);
        float4 s4r = TEX_SAMPLE(inTex, (fx + p.chromaOffsetR * p.renderScale) * invW,
                                fyChroma * invH);
        Cb = s4b.y;
        Cr = s4r.z;
    } else {
        float4 s4c = TEX_SAMPLE(inTex, fx * invW, fyChroma * invH);
        Cb = s4c.y;
        Cr = s4c.z;
    }

    TEX_WRITE(outTex, x, y, MAKE_FLOAT4(Y, Cb, Cr, a));
}

// ============================================================
// Pass 5 body — FINAL COLOR COMPOSITE
//
// Reads the warped and bandwidth-limited Y'CbCr texture and
// applies colour grading, CRT bloom (mist), colour-space
// transforms and look LUT. All geometric warp and interlace
// effects happened before bandwidth in Pass 2.
// ============================================================
DEVFN void vhs_color_body(
    int x, int y,
    GLOBAL_RW(float) dstBuf,
    CONSTANT_REF(VhsParams) p,
    GLOBAL_RO(float) outputCstLut,
    READ_TEX inTex)
{
    float fx = float(x) + 0.5f;
    float fy = float(y) + 0.5f;
    float invW = 1.0f / float(p.width);
    float invH = 1.0f / float(p.height);

    float4 s4 = TEX_SAMPLE(inTex, fx * invW, fy * invH);
    float Y = s4.x;
    float Cb = s4.y;
    float Cr = s4.z;
    float a = s4.w;

    // ---------------- Y'CbCr -> display gamma RGB -----------------------
    float3 v709 = ycbcr601_to_rgb(MAKE_FLOAT3(Y, Cb, Cr));

    // Color fade
    if (p.colorFade > 0.001f) {
        float lum = 0.299f * v709.x + 0.587f * v709.y + 0.114f * v709.z;
        v709 = v709 * (1.0f - p.colorFade) + MAKE_FLOAT3(lum, lum, lum) * p.colorFade;
    }
    // Hue tint via YCbCr rotation
    if (fabs(p.hueTint) > 0.01f) {
        float3 yc = rgb_to_ycbcr601(v709);
        float a_ = p.hueTint * 3.14159f / 180.0f;
        float ca = cos(a_), sa = sin(a_);
        float ncb = ca * yc.y - sa * yc.z;
        float ncr = sa * yc.y + ca * yc.z;
        v709 = ycbcr601_to_rgb(MAKE_FLOAT3(yc.x, ncb, ncr));
    }

    // Black lift
    v709 = v709 * (1.0f - p.blackLift) + MAKE_FLOAT3(p.blackLift, p.blackLift, p.blackLift);

    // White knee
    if (p.whiteKnee < 0.999f) {
        float k = max(p.whiteKnee, 0.5f);
        float invk = 1.0f / max(1.0f - k, 1e-3f);
        float ox = max(v709.x - k, 0.0f) * invk;
        float oy = max(v709.y - k, 0.0f) * invk;
        float oz = max(v709.z - k, 0.0f) * invk;
        float rx = k + (1.0f - k) * (1.0f - exp(-ox * 1.5f));
        float ry = k + (1.0f - k) * (1.0f - exp(-oy * 1.5f));
        float rz = k + (1.0f - k) * (1.0f - exp(-oz * 1.5f));
        v709 = MAKE_FLOAT3(
            (v709.x > k) ? rx : v709.x,
            (v709.y > k) ? ry : v709.y,
            (v709.z > k) ? rz : v709.z);
    }

    // Dynamic range compression
    if (p.dynamicRangeStops < 13.5f) {
        float scale = clamp(p.dynamicRangeStops / 14.0f, 0.3f, 1.0f);
        v709 = v709 * scale + MAKE_FLOAT3((1.0f - scale) * 0.18f,
                                          (1.0f - scale) * 0.18f,
                                          (1.0f - scale) * 0.18f);
    }

    // VHS CRT / lens bloom — applied after tape-domain color corrections,
    // before the display-to-linear encode. Wide Gaussian (sigma = spread)
    // gives a soft, dreamy glow distinct from the sharper Film mist.
    if (p.mistEnabled && p.mistAmount > 1e-6f) {
        float4 bloomVeil = vhs_mist_bloom(inTex, fx, fy, invW, invH,
                                          p.mistSpread * p.renderScale,
                                          p.mistThreshold);
        // Additive bloom — bright sources spread into neighbours
        v709 = v709 + bloomVeil.xyz * p.mistAmount;
        // Veiling glare: soft shadow lift proportional to scene brightness
        if (p.mistVeil > 1e-6f) {
            float3 headroom = max(MAKE_FLOAT3(1.0f, 1.0f, 1.0f) - v709,
                                  MAKE_FLOAT3(0.0f, 0.0f, 0.0f));
            v709 = v709 + MAKE_FLOAT3(bloomVeil.w, bloomVeil.w, bloomVeil.w)
                          * p.mistVeil * headroom;
        }
    }

    // Gamma -> linear -> DWG -> DI
    float3 v709_clamped = MAKE_FLOAT3(max(v709.x, 0.0f), max(v709.y, 0.0f), max(v709.z, 0.0f));
    float3 lin709 = v_to_lin3(v709_clamped);
    float3 linDwg = rec709_to_dwg(lin709);
    float3 di     = linear_to_di3(linDwg);

    // Output CST
    float3 outRGB = (p.cstLutSize > 0)
                      ? applyLut3D(outputCstLut, p.cstLutSize, di)
                      : di;

    writePx(dstBuf, x, y, p.dstRowBytes,
            outRGB.x, outRGB.y, outRGB.z, a);
}
