#pragma once

#include <basis/seadTypes.h>

namespace al {
    class LiveActor;
    class ClippingAreaActorInfo;

    /// Links an actor to the clipping area info it belongs to (LiveActor::mClippingAreaActorInfoNode).
    class ClippingAreaActorInfoNode {
    public:
        u8 _0[0x10];
        LiveActor* mActor;                          // _10
        ClippingAreaActorInfo* mAreaActorInfo;      // _18
        u8 _20[0x28];
    };

    static_assert(sizeof(ClippingAreaActorInfoNode) == 0x48, "ClippingAreaActorInfoNode size");

    /// Clipping settings shared by the actors placed in one clipping view area.
    class ClippingAreaActorInfo {
    public:
        void setExpandedClippingMode(bool isExpanded);
        void disableFarLod();

        u8 _0[0x95];
        bool mIsExpandedClippingMode;               // _95
    };
};
