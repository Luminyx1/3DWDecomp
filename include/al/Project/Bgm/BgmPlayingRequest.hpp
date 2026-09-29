#pragma once

#include <basis/seadTypes.h>

namespace al {
enum BgmPlayingType : s32 {
    BgmPlayingType_Start = 0,
    BgmPlayingType_Stop = 1,
    BgmPlayingType_Pause = 2,
    BgmPlayingType_Resume = 3,
};

struct BgmPlayingRequest {
    const char* name = nullptr;  // _0
    s32 _8 = -1;
    s32 _c = 0;
    s32 _10 = -1;
    bool _14 = false;
    bool _15 = false;
    s32 _18 = -1;
    s32 _1c = -1;
};
}  // namespace al
