#pragma once

#include <nn/types.h>

namespace nn::atk {
class Sound3DListener;
class Sound3DManager;
struct Sound3DParam;

class Sound3DCalculator {
public:
    struct CalculatePanParam {
        CalculatePanParam();

        f32 speakerAngleStereo;
        f32 frontSpeakerAngleSurround;
        f32 rearSpeakerAngleSurround;
        f32 initPan;
    };

    static void CalculateVolumeAndPriority(f32* pVolume, s32* pPriority,
                                           const Sound3DManager& rManager,
                                           const Sound3DListener& rListener,
                                           const Sound3DParam& rParam);
    static void CalculatePan(f32* pPan, f32* pSurroundPan, const Sound3DManager& rManager,
                             const Sound3DListener& rListener, const Sound3DParam& rParam,
                             const CalculatePanParam& rPanParam);
    static void CalculatePitch(f32* pPitch, const Sound3DManager& rManager,
                               const Sound3DListener& rListener, const Sound3DParam& rParam);
    static void CalculateBiquadFilterValue(f32* pValue, const Sound3DManager& rManager,
                                           const Sound3DListener& rListener,
                                           const Sound3DParam& rParam);
};
}  // namespace nn::atk
