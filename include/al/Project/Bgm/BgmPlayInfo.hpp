#pragma once

#include <basis/seadTypes.h>

namespace al {
class ByamlIter;

struct BgmPlayInfo {
    static BgmPlayInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmPlayInfo* pInfoA, const BgmPlayInfo* pInfoB);

    const char* mName;                      // _0
    const char* mLineName;                  // _8
    const char* mDefaultResourceName;       // _10
    bool mIsPlayingByUpperLayerAudioUser;   // _18
};
}  // namespace al
