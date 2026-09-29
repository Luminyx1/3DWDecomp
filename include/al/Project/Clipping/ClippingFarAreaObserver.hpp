#pragma once

#include <basis/seadTypes.h>

namespace al {
    class AreaObj;
    class AreaObjDirector;
    class AreaObjGroup;
    class PlayerHolder;

    /// Updates the far clip distance from the ClippingFarArea the players are in.
    class ClippingFarAreaObserver {
    public:
        ClippingFarAreaObserver(const AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder);

        void setDefaultFarClipDistance(f32 distance);
        void setDefaultFarClipDistanceSub(f32 distance);
        void endInit();
        void update();

        f32 getFarClipDistance() const { return mFarClipDistance * mFarClipDistanceRate; }

        const AreaObjDirector* mAreaObjDirector;    // _0
        const PlayerHolder* mPlayerHolder;          // _8
        AreaObjGroup* mAreaObjGroup;                // _10
        AreaObj* mAreaObj;                          // _18
        f32 mFarClipDistance;                       // _20
        f32 mDefaultFarClipDistance;                // _24
        f32 mFarClipDistanceSub;                    // _28
        f32 mDefaultFarClipDistanceSub;             // _2C
        f32 mFarClipDistanceRate;                   // _30
    };
};
