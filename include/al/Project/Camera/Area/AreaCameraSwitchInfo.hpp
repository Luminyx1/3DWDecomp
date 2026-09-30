#pragma once

#include <basis/seadTypes.h>

namespace al {
class AreaObj;

class AreaCameraSwitchInfo {
public:
    AreaCameraSwitchInfo();

    void initArea(const AreaObj* pArea, s32 priority, bool isInterpoleIn, bool isInterpoleOut);

    void* _0 = nullptr;
    const AreaObj* mArea = nullptr;
    s32 mPriority = 0;
    bool mIsInterpoleIn = false;
    bool mIsInterpoleOut = false;
    bool _16 = false;
    bool mIsEndNoInterpole = false;
    bool mIsStartWhenCollideGround = false;
    bool mIsStartWhenInWater = false;
};

static_assert(sizeof(AreaCameraSwitchInfo) == 0x20);
}  // namespace al
