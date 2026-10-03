#pragma once

#include <basis/seadTypes.h>

namespace al {
class AreaObjDirector;
class AreaObjGroup;
class ClippingActorInfo;
class PlacementId;
class PlayerHolder;

class ClippingViewInfo {
public:
    ClippingViewInfo();

    const PlacementId* mParentId;
    bool mIsInViewCtrlArea;
    bool _9;
};

class ViewInfoCtrl {
public:
    ViewInfoCtrl(const AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder);

    void initActorInfo(ClippingActorInfo* pInfo, PlacementId* pPlacementId);
    void endInit();
    void update();
    ClippingViewInfo* tryFindViewInfo(PlacementId* pPlacementId) const;

private:
    const AreaObjDirector* mAreaObjDirector = nullptr;
    AreaObjGroup* mViewCtrlAreaGroup = nullptr;
    ClippingViewInfo* mDefaultViewInfo = nullptr;
    s32 mViewInfoNum = 0;
    ClippingViewInfo** mViewInfos = nullptr;
    bool mIsInvalid = false;
    const PlayerHolder* mPlayerHolder = nullptr;
};
}  // namespace al
