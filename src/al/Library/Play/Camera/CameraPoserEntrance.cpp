#include "Library/Play/Camera/CameraPoserEntrance.hpp"

#include <gfx/seadCamera.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/Obj/PlayerWatcher.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Camera/Holder/CameraSwitcher.hpp"

namespace al {

/**
 * Constructs a camera that shows the players entering the stage.
 * @param rParam Camera parameters.
 * @param pPlayerWatcher Watcher of the players to look at.
 * @param pCameraSwitcher Switcher used to end the camera once a player moves.
 * @param pPlacementId Placement id of the camera, copied.
 */
CameraPoserEntrance::CameraPoserEntrance(const CameraPoserEntranceParam& rParam,
                                         const PlayerWatcher* pPlayerWatcher,
                                         CameraSwitcher* pCameraSwitcher,
                                         const PlacementId* pPlacementId)
    : mParam(rParam), mPlayerWatcher(pPlayerWatcher), mCameraSwitcher(pCameraSwitcher) {
    mName = "Entrance";
    mPlacementId = new PlacementId(*pPlacementId);
    mStartPlayerPos = new sead::Vector3f[pPlayerWatcher->getPlayerNum()];

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        mStartPlayerPos[i] = sead::Vector3f::zero;
    }
}

/**
 * Remembers where the players start.
 */
void CameraPoserEntrance::start() {
    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        mStartPlayerPos[i] = mPlayerWatcher->getPlayerPos(i);
    }
}

/**
 * Looks at the center of the alive players and ends the camera once a player moved away from
 * their start position.
 */
void CameraPoserEntrance::update() {
    sead::Vector3f sum = sead::Vector3f::zero;
    s32 aliveNum = 0;

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if (mPlayerWatcher->isPlayerAlive(i)) {
            sum += mPlayerWatcher->getPlayerPos(i) + sead::Vector3f(0.0f, 150.0f, 0.0f);
            aliveNum++;
        }
    }

    if (aliveNum == 0) {
        return;
    }

    mLookAtPos = sum * (1.0f / aliveNum);
    const sead::Vector3f& rOffset = mParam.mLookAtOffset;
    mLookAtPos.x += rOffset.x * sead::Mathf::cos(sead::Mathf::deg2rad(mParam.mAngleH)) +
                    rOffset.z * sead::Mathf::sin(sead::Mathf::deg2rad(mParam.mAngleH));
    mLookAtPos.y += rOffset.y;
    mLookAtPos.z += rOffset.z * sead::Mathf::cos(sead::Mathf::deg2rad(mParam.mAngleH)) -
                    rOffset.x * sead::Mathf::sin(sead::Mathf::deg2rad(mParam.mAngleH));

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if ((mStartPlayerPos[i] - mPlayerWatcher->getPlayerPos(i)).length() > 50.0f) {
            mCameraSwitcher->end(*mPlacementId, -1);
            return;
        }
    }
}

/**
 * Writes the pose of the camera, looking at the look at position from the parameter angles.
 * @param pCamera The camera to write to.
 */
void CameraPoserEntrance::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    sead::Vector3f front = sead::Vector3f::ez;
    sead::Vector3f lookAtPos = mLookAtPos;
    f32 angleV = mParam.mAngleV;
    f32 angleH = mParam.mAngleH;

    sead::Quatf quatV;
    quatV.setAxisAngle(-sead::Vector3f::ex, angleV);
    sead::Matrix34f mtxV;
    mtxV.fromQuat(quatV);

    sead::Quatf quatH;
    quatH.setAxisAngle(sead::Vector3f::ey, angleH);
    sead::Matrix34f mtxH;
    mtxH.fromQuat(quatH);

    sead::Matrix34f mtx;
    mtx.setMul(mtxH, mtxV);
    front.mul(mtx);

    sead::Vector3f pos = mParam.mDistance * front + lookAtPos;
    pCamera->setAt(lookAtPos);
    pCamera->setPos(pos);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * Reads the camera parameters.
 * @param pIter The camera parameters.
 */
void CameraPoserEntrance::loadParam(const ByamlIter* pIter) {
    ByamlIter iter;
    pIter->tryGetIterByIndex(&iter, 0);
    iter.tryGetFloatByKey(&mParam.mAngleV, "AngleV");
    iter.tryGetFloatByKey(&mParam.mAngleH, "AngleH");
    iter.tryGetFloatByKey(&mParam.mDistance, "Distance");
    iter.tryGetFloatByKey(&mParam.mLookAtOffset.x, "LookAtOffsetX");
    iter.tryGetFloatByKey(&mParam.mLookAtOffset.y, "LookAtOffsetY");
    iter.tryGetFloatByKey(&mParam.mLookAtOffset.z, "LookAtOffsetZ");
}

/**
 * Constructs the default entrance camera parameters.
 */
CameraPoserEntranceParam::CameraPoserEntranceParam() = default;

}  // namespace al
