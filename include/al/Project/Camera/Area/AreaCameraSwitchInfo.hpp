#pragma once

#include <basis/seadTypes.h>

namespace al {
class AreaObj;

/// The camera switch settings of a camera area.
class AreaCameraSwitchInfo {
public:
    AreaCameraSwitchInfo();

    void initArea(const AreaObj* pArea, s32 priority, bool isInterpoleIn, bool isInterpoleOut);

    void* _0 = nullptr;                    // _0
    const AreaObj* mArea = nullptr;        // _8
    s32 mPriority = 0;                     // _10
    bool mIsInterpoleIn = false;           // _14
    bool mIsInterpoleOut = false;          // _15
    bool _16 = false;                      // _16
    bool mIsEndNoInterpole = false;        // _17
    bool mIsStartWhenCollideGround = false;  // _18
    bool mIsStartWhenInWater = false;      // _19
};
}  // namespace al
