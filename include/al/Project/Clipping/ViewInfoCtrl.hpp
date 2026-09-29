#pragma once

#include <basis/seadTypes.h>

namespace al {
    class AreaObjDirector;
    class AreaObjGroup;
    class ClippingActorInfo;
    class PlacementId;
    class PlayerHolder;

    /// Tracks which clipping view groups contain a player, using the scene's ViewCtrlAreas.
    class ViewInfoCtrl {
    public:
        struct ViewInfo {
            ViewInfo() : mPlacementId(nullptr), mIsInViewCtrlArea(false), _9(false) {}

            PlacementId* mPlacementId;  // _0
            bool mIsInViewCtrlArea;     // _8
            bool _9;
        };

        ViewInfoCtrl(const AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder);

        void initActorInfo(ClippingActorInfo* pActorInfo, PlacementId* pViewId);
        void endInit();
        void update();
        ViewInfo* tryFindViewInfo(PlacementId* pViewId) const;

        const AreaObjDirector* mAreaObjDirector;    // _0
        AreaObjGroup* mViewCtrlAreaGroup;           // _8
        ViewInfo* mDefaultViewInfo;                 // _10
        s32 mViewInfoNum;                           // _18
        ViewInfo** mViewInfos;                      // _20
        bool mIsInvalid;                            // _28
        const PlayerHolder* mPlayerHolder;          // _30
    };
};
