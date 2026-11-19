#include "../src/engines/shared/vhs90_region_effects.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

static bool same(const std::vector<float>& a, const std::vector<float>& b) {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

int main() {
    constexpr int W = 320, H = 240, STRIDE = W * 4;
    std::vector<float> src(W * H * 4), a, b;
    for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) {
        const size_t i = (static_cast<size_t>(y) * W + x) * 4;
        src[i] = float(x) / float(W - 1);
        src[i + 1] = float(y) / float(H - 1);
        src[i + 2] = 0.25f;
        src[i + 3] = 0.37f;
    }

    a = src; b = src;
    vhs90::FloatRgbaView va{a.data(), W, H, STRIDE};
    vhs90::FloatRgbaView vb{b.data(), W, H, STRIDE};
    vhs90::TrackingNoiseParams tr;
    tr.enabled = true;
    tr.region.centerX = 0.5f;
    tr.region.centerY = 0.35f;
    tr.region.width = 0.9f;
    tr.region.height = 0.18f;
    tr.displacementPixels = 15.0f;
    tr.noiseAmount = 0.08f;
    vhs90::applyTrackingNoise(va, tr, 1234, 12);
    vhs90::applyTrackingNoise(vb, tr, 1234, 12);
    if (!same(a, b)) return 1;

    bool changed = false;
    for (size_t i = 0; i < a.size(); i += 4) {
        if (std::abs(a[i] - src[i]) > 1e-6f) changed = true;
        if (a[i + 3] != src[i + 3]) return 2;
    }
    if (!changed) return 3;

    std::vector<float> full = src;
    vhs90::FloatRgbaView vf{full.data(), W, H, STRIDE};
    vhs90::HeadSwitchParams hs;
    hs.enabled = true;
    hs.anchorY = 1.0f;
    hs.heightFraction = 1.0f;
    hs.horizontalShiftPixels = 40.0f;
    hs.intensity = 1.0f;
    vhs90::applyHeadSwitching(vf, hs, 55, 8);

    int changedRows = 0;
    for (int y = 0; y < H; ++y) {
        bool rowChanged = false;
        for (int x = 0; x < W; ++x) {
            const size_t i = (static_cast<size_t>(y) * W + x) * 4;
            if (std::abs(full[i] - src[i]) > 1e-5f) rowChanged = true;
            if (full[i + 3] != src[i + 3]) return 4;
        }
        if (rowChanged) ++changedRows;
    }
    if (changedRows < H * 0.9) return 5;

    vhs90::DropoutParams dp;
    dp.enabled = true;
    dp.densityPerFrame = 12.0f;
    dp.intensity = 1.0f;
    vhs90::applyDropouts(vf, dp, 55, 8);

    std::cout << "PASS tracking deterministic; alpha preserved; head-switch rows="
              << changedRows << "; dropouts applied\n";
    return 0;
}
