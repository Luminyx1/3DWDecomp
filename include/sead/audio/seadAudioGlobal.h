#pragma once

#include "basis/seadTypes.h"

namespace sead {
class AudioGlobal {
public:
    enum OutputMode {
        cOutputMode_Stereo = 0,
        cOutputMode_Monaural = 1,
        cOutputMode_Surround = 2,
        cOutputMode_Invalid = 4
    };

    enum AuxBus {
        cAuxBus_A = 0,
        cAuxBus_B = 1,
        cAuxBus_C = 2
    };
};

enum AudioStartResult {
    cAudioStartResult_Success = 0,
    cAudioStartResult_Unknown = 21
};
}  // namespace sead
