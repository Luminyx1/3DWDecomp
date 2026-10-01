#include "Project/Clipping/ClippingFarAreaObserver.hpp"

#include "Library/Player/PlayerUtil.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {
/**
 * Constructs the far clip area observer.
 * @param pAreaObjDirector area director
 * @param pPlayerHolder player holder
 */
ClippingFarAreaObserver::ClippingFarAreaObserver(const AreaObjDirector* pAreaObjDirector,
                                                 const PlayerHolder* pPlayerHolder)
    : mAreaObjDirector(pAreaObjDirector), mPlayerHolder(pPlayerHolder) {}

/**
 * Sets the default far clip distance.
 * @param distance far clip distance
 */
void ClippingFarAreaObserver::setDefaultFarClipDistance(f32 distance) {
    mFarClipDistance = distance;
    mDefaultFarClipDistance = distance;
}

/**
 * Sets the default far clip distance of the sub view.
 * @param distance far clip distance
 */
void ClippingFarAreaObserver::setDefaultFarClipDistanceSub(f32 distance) {
    mFarClipDistanceSub = distance;
    mDefaultFarClipDistanceSub = distance;
}

/**
 * Finds the far clip areas.
 */
void ClippingFarAreaObserver::endInit() {
    mAreaObjGroup = mAreaObjDirector->getAreaObjGroup("ClippingFarArea");
}

/**
 * Updates the far clip distances from the far clip area the players are in.
 */
void ClippingFarAreaObserver::update() {
    if (mAreaObjGroup == nullptr) {
        return;
    }

    mCurrentArea = nullptr;
    s32 num = getPlayerNumMax(mPlayerHolder);

    for (s32 i = 0; i < num; i++) {
        if (isPlayerDead(mPlayerHolder, i)) {
            continue;
        }

        AreaObj* areaObj = mAreaObjGroup->getInVolumeAreaObj(getPlayerPos(mPlayerHolder, i));

        if (areaObj == nullptr) {
            continue;
        }

        if (mCurrentArea == nullptr || areaObj->getPriority() > mCurrentArea->getPriority()) {
            mCurrentArea = areaObj;
        }
    }

    mFarClipDistance = mDefaultFarClipDistance;
    mFarClipDistanceSub = mDefaultFarClipDistanceSub;

    if (mCurrentArea != nullptr) {
        tryGetAreaObjArg(&mFarClipDistance, mCurrentArea, "FarClipDistance");
        tryGetAreaObjArg(&mFarClipDistanceSub, mCurrentArea, "FarClipDistanceSub");
    }
}
}  // namespace al
