#include "Project/Camera/Area/CameraAngleVerticalRequester.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {
/** @brief Creates a requester without any area. */
CameraAngleVerticalRequester::CameraAngleVerticalRequester() = default;

/**
 * @brief Sets the area director the request areas are looked up in.
 * @param pAreaObjDirector The scene's area director.
 */
void CameraAngleVerticalRequester::init(AreaObjDirector* pAreaObjDirector) {
    mAreaObjDirector = pAreaObjDirector;
}

/** @brief Looks up the group of request areas once all areas are placed. */
void CameraAngleVerticalRequester::initAfterPlacement() {
    mRequestAreaGroup = tryFindAreaObjGroup(this, "CameraAngleVerticalRequestArea");
}

/**
 * @brief Finds the request area the target is in and reads its angle when it changed.
 * @param rPos The position of the camera target.
 */
void CameraAngleVerticalRequester::update(const sead::Vector3f& rPos) {
    if (mRequestAreaGroup == nullptr) {
        return;
    }

    AreaObj* area = tryGetAreaObj(mRequestAreaGroup, rPos);
    if (area != mRequestArea) {
        mRequestArea = area;
        mFramesUnchanged = -1;
        if (area != nullptr) {
            getArg(&mAngleVertical, *area->mPlacementInfo, "AngleVertical");
        }
    }
    mFramesUnchanged++;
}
}  // namespace al
