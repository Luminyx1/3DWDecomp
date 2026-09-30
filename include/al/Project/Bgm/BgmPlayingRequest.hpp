#pragma once

#include <basis/seadTypes.h>

namespace al {
struct BgmPlayingRequest {
    BgmPlayingRequest(const char* pName, s32 fadeIn = -1, s32 startDelay = 0, s32 fadeOut = -1,
                      bool isRestartBgm = false, s32 unk = -1)
        : name(pName), fadeInFrames(fadeIn), startDelayFrames(startDelay), fadeOutFrames(fadeOut),
          isRestart(isRestartBgm), _1c(unk) {}

    const char* name;
    s32 fadeInFrames;
    s32 startDelayFrames;
    s32 fadeOutFrames;
    bool isRestart;
    bool _15 = false;
    s32 _18 = -1;
    s32 _1c;
};
static_assert(sizeof(BgmPlayingRequest) == 0x20);
}  // namespace al
