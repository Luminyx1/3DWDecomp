#include "Project/Clipping/ViewInfoCtrl.hpp"

#include "Library/Clipping/ClippingActorInfo.hpp"
#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"

namespace al {
/**
 * Creates an empty view group info.
 */
ClippingViewInfo::ClippingViewInfo() : mParentId(nullptr), mIsInViewCtrlArea(false), _9(false) {}

/**
 * Creates the view info controller.
 * @param pAreaObjDirector area director
 * @param pPlayerHolder player holder
 */
ViewInfoCtrl::ViewInfoCtrl(const AreaObjDirector* pAreaObjDirector,
                           const PlayerHolder* pPlayerHolder)
    : mAreaObjDirector(pAreaObjDirector), mPlayerHolder(pPlayerHolder) {
    mViewInfos = new ClippingViewInfo*[0x80];

    for (s32 i = 0; i < 0x80; i++) {
        mViewInfos[i] = nullptr;
    }

    mDefaultViewInfo = new ClippingViewInfo;
    mViewInfos[0] = new ClippingViewInfo;
    mViewInfoNum = 1;
    mViewInfos[0]->mParentId = nullptr;
    mViewInfos[0]->mIsInViewCtrlArea = true;
}

/**
 * Links the view group far clip flag of an actor to its view group.
 * @param pInfo clipping info of the actor
 * @param pPlacementId view id of the actor, or nullptr
 */
void ViewInfoCtrl::initActorInfo(ClippingActorInfo* pInfo, PlacementId* pPlacementId) {
    if (pPlacementId == nullptr || pPlacementId->mPlacementID == nullptr) {
        pInfo->mViewGroupFarClipFlag = &mDefaultViewInfo->mIsInViewCtrlArea;
        return;
    }

    for (s32 i = 0; i < mViewInfoNum; i++) {
        ClippingViewInfo* clippingId = mViewInfos[i];

        if (clippingId->mParentId != nullptr && clippingId->mParentId->isEqual(*pPlacementId)) {
            pInfo->mViewGroupFarClipFlag = &clippingId->mIsInViewCtrlArea;
            return;
        }
    }

    ClippingViewInfo* newId = new ClippingViewInfo;
    newId->mParentId = pPlacementId;
    pInfo->mViewGroupFarClipFlag = &newId->mIsInViewCtrlArea;
    mViewInfos[mViewInfoNum] = newId;
    mViewInfoNum++;
}

/**
 * Finishes initialization.
 */
void ViewInfoCtrl::endInit() {
    mViewCtrlAreaGroup = mAreaObjDirector->getAreaObjGroup("ViewCtrlArea");
}

/**
 * Updates which view groups contain a player.
 */
void ViewInfoCtrl::update() {
    if (mIsInvalid || mViewCtrlAreaGroup == nullptr) {
        return;
    }

    for (s32 i = 0; i < mViewInfoNum; i++) {
        ClippingViewInfo* clippingId = mViewInfos[i];
        clippingId->mIsInViewCtrlArea = false;
        clippingId->_9 = false;
    }

    for (s32 i = 0; i < mViewCtrlAreaGroup->getSize(); i++) {
        AreaObj* areaObj = mViewCtrlAreaGroup->getAreaObj(i);
        s32 playerNum = getPlayerNumMax(mPlayerHolder);

        for (s32 j = 0; j < playerNum; j++) {
            if (isPlayerDead(mPlayerHolder, j)) {
                continue;
            }

            if (tryIsInAreaPos(areaObj, getPlayerPos(mPlayerHolder, j))) {
                PlacementId viewId;
                alPlacementFunction::getClippingViewId(&viewId, areaObj->getPlacementInfo());
                ClippingViewInfo* clippingId = tryFindViewInfo(&viewId);

                if (clippingId != nullptr) {
                    clippingId->mIsInViewCtrlArea = true;
                }

                break;
            }
        }
    }
}

/**
 * Finds the view group of a view id.
 * @param pPlacementId view id
 * @return the view group, or nullptr if it doesn't exist
 */
ClippingViewInfo* ViewInfoCtrl::tryFindViewInfo(PlacementId* pPlacementId) const {
    if (pPlacementId == nullptr) {
        return nullptr;
    }

    for (s32 i = 0; i < mViewInfoNum; i++) {
        ClippingViewInfo* clippingId = mViewInfos[i];

        if (clippingId->mParentId != nullptr && clippingId->mParentId->isEqual(*pPlacementId)) {
            return clippingId;
        }
    }

    return nullptr;
}
}  // namespace al
