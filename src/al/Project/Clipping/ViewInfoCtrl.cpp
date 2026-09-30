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
 * Creates the view info controller.
 * @param pAreaObjDirector area director
 * @param pPlayerHolder player holder
 */
ViewInfoCtrl::ViewInfoCtrl(const AreaObjDirector* pAreaObjDirector,
                           const PlayerHolder* pPlayerHolder)
    : mAreaObjDirector(pAreaObjDirector), mPlayerHolder(pPlayerHolder) {
    mClippingPlacementIds = new ClippingPlacementId*[0x80];
    for (s32 i = 0; i < 0x80; i++) {
        mClippingPlacementIds[i] = nullptr;
    }

    mDefaultPlacementId = new ClippingPlacementId;
    mClippingPlacementIds[0] = new ClippingPlacementId;
    mClippingPlacementIdsSize = 1;
    mClippingPlacementIds[0]->mParentId = nullptr;
    mClippingPlacementIds[0]->mIsInViewCtrlArea = true;
}

/**
 * Links the view group far clip flag of an actor to its view group.
 * @param pInfo clipping info of the actor
 * @param pPlacementId view id of the actor, or nullptr
 */
void ViewInfoCtrl::initActorInfo(ClippingActorInfo* pInfo, PlacementId* pPlacementId) {
    if (!pPlacementId || !pPlacementId->mPlacementID) {
        pInfo->mViewGroupFarClipFlag = &mDefaultPlacementId->mIsInViewCtrlArea;
        return;
    }

    for (s32 i = 0; i < mClippingPlacementIdsSize; i++) {
        ClippingPlacementId* clippingId = mClippingPlacementIds[i];
        if (clippingId->mParentId && clippingId->mParentId->isEqual(*pPlacementId)) {
            pInfo->mViewGroupFarClipFlag = &clippingId->mIsInViewCtrlArea;
            return;
        }
    }

    ClippingPlacementId* newId = new ClippingPlacementId;
    newId->mParentId = pPlacementId;
    pInfo->mViewGroupFarClipFlag = &newId->mIsInViewCtrlArea;
    mClippingPlacementIds[mClippingPlacementIdsSize] = newId;
    mClippingPlacementIdsSize++;
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
    if (mIsInvalid || !mViewCtrlAreaGroup) {
        return;
    }

    for (s32 i = 0; i < mClippingPlacementIdsSize; i++) {
        ClippingPlacementId* clippingId = mClippingPlacementIds[i];
        clippingId->mIsInViewCtrlArea = false;
        clippingId->_9 = false;
    }

    for (s32 i = 0; i < mViewCtrlAreaGroup->mNumAreas; i++) {
        AreaObj* areaObj = mViewCtrlAreaGroup->getAreaObj(i);
        s32 playerNum = getPlayerNumMax(mPlayerHolder);
        for (s32 j = 0; j < playerNum; j++) {
            if (isPlayerDead(mPlayerHolder, j)) {
                continue;
            }

            if (tryIsInAreaPos(areaObj, getPlayerPos(mPlayerHolder, j))) {
                PlacementId viewId;
                alPlacementFunction::getClippingViewId(&viewId, *areaObj->mPlacementInfo);
                ClippingPlacementId* clippingId = tryFindViewInfo(&viewId);
                if (clippingId) {
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
ViewInfoCtrl::ClippingPlacementId* ViewInfoCtrl::tryFindViewInfo(PlacementId* pPlacementId) const {
    if (!pPlacementId) {
        return nullptr;
    }

    for (s32 i = 0; i < mClippingPlacementIdsSize; i++) {
        ClippingPlacementId* clippingId = mClippingPlacementIds[i];
        if (clippingId->mParentId && clippingId->mParentId->isEqual(*pPlacementId)) {
            return clippingId;
        }
    }

    return nullptr;
}
}  // namespace al
