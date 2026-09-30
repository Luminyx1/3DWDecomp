#pragma once

#include <basis/seadTypes.h>

namespace al {
class AreaObjDirector;
class AreaObjGroup;
class ClippingActorInfo;
class PlacementId;
class PlayerHolder;

class ViewInfoCtrl {
public:
    struct ClippingPlacementId {
        const PlacementId* mParentId = nullptr;
        bool mIsInViewCtrlArea = false;
        bool _9 = false;
    };

    ViewInfoCtrl(const AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder);

    void initActorInfo(ClippingActorInfo* pInfo, PlacementId* pPlacementId);
    void endInit();
    void update();
    ClippingPlacementId* tryFindViewInfo(PlacementId* pPlacementId) const;

private:
    const AreaObjDirector* mAreaObjDirector = nullptr;
    AreaObjGroup* mViewCtrlAreaGroup = nullptr;
    ClippingPlacementId* mDefaultPlacementId = nullptr;
    s32 mClippingPlacementIdsSize = 0;
    ClippingPlacementId** mClippingPlacementIds = nullptr;
    bool mIsInvalid = false;
    const PlayerHolder* mPlayerHolder = nullptr;
};
}  // namespace al
