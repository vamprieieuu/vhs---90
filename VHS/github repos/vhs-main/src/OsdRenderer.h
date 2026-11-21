#ifndef VHS_OSD_RENDERER_H
#define VHS_OSD_RENDERER_H

#include "VhsParams.h"
#include <cstdint>
#include <vector>

// Build a full-frame R8 mask containing the rasterized camcorder OSD
// at the requested positions. Returns an empty vector if no element is
// enabled.
//
// Mask values:
//   0   = transparent (image passes through)
//   255 = white text fill
//   128 = black 1-px shadow
//
// `frameInTimeline` is the timeline frame index (used to advance the
// free-running clock and tape counter).
std::vector<uint8_t> renderOsdMask(const VhsParams& p, int frameInTimeline,
                                   double fps);

#endif // VHS_OSD_RENDERER_H
