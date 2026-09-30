#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;

class ClippingAreaActorInfo {
public:
    void setExpandedClippingMode(bool isExpanded);

    bool isExpandedClippingMode() const { return mIsExpandedClippingMode; }

private:
    u8 _0[0x94];
    bool mIsEnableExpandedClippingMode;
    bool mIsExpandedClippingMode;
};

struct ClippingAreaActorInfoNode {
    u8 _0[0x10];
    LiveActor* mActor;
    ClippingAreaActorInfo* mInfo;
    u8 _20[0x48 - 0x20];
};
}  // namespace al
