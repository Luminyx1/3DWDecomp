#include "Library/Play/Camera/CameraPoserKinopioBrigade.hpp"

#include <cmath>
#include <gfx/seadCamera.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/Controller/InputFunction.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Obj/PlayerWatcher.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"

namespace {

/**
 * Calculates the pitch of the pad from its pose.
 * @param rAxisY Y axis of the pad pose.
 * @param rAxisZ Z axis of the pad pose.
 * @return Pitch in degrees.
 */
f32 calcPadAngleV(const sead::Vector3f& rAxisY, const sead::Vector3f& rAxisZ) {
    f32 lengthH = sead::Mathf::sqrt(rAxisY.x * rAxisY.x + rAxisY.z * rAxisY.z);

    if (rAxisZ.y <= 0.0f) {
        lengthH = -lengthH;
    }

    return sead::Mathf::rad2deg(std::atan2(rAxisY.y, lengthH));
}

/**
 * Calculates the yaw of the pad from its pose.
 * @param rAxisX X axis of the pad pose.
 * @return Yaw in degrees.
 */
f32 calcPadAngleH(const sead::Vector3f& rAxisX) {
    return sead::Mathf::rad2deg(std::atan2(-rAxisX.z, rAxisX.x));
}

/**
 * Wraps an angle into the range [-180, 180).
 * @param angle Angle in degrees.
 * @return The wrapped angle.
 */
f32 wrapAngle180(f32 angle) {
    return al::modf(angle + 180.0f + 360.0f, 360.0f) + -180.0f;
}

/**
 * Snaps an angle to the nearest multiple of 45 degrees.
 * @param angle Angle in degrees.
 * @return The snapped angle.
 */
f32 snapAngle45(f32 angle) {
    return static_cast<s32>((angle + 22.5f) / 45.0f) * 45.0f;
}

/**
 * Reads the stick of a player that rotates the camera.
 * @param pStick Output for the stick, which keeps its value if the player cannot rotate the camera.
 * @param pHolder Player holder.
 * @param port Controller port of the player.
 * @return Whether the player can rotate the camera.
 */
bool tryGetCameraStick(sead::Vector2f* pStick, const al::PlayerHolder* pHolder, s32 port) {
    if (port < 1 || al::tryFindAlivePlayerActorFromPort(pHolder, port) == nullptr) {
        return false;
    }

    if (al::isPadTypeJoySingle(port)) {
        if (!al::isPadHoldA(port)) {
            return false;
        }

        *pStick = al::getLeftStick(port);
    } else {
        *pStick = al::getRightStick(port);
    }

    return true;
}

/**
 * Checks whether the stick is tilted enough to rotate the camera.
 * @param rStick Stick.
 * @return Whether the stick is tilted.
 */
bool isStickTilted(const sead::Vector2f& rStick) {
    return sead::Mathf::abs(rStick.x) > 0.3f || sead::Mathf::abs(rStick.y) > 0.3f;
}

/**
 * Reads the stick of the player that rotated the camera last.
 * @param pStick Output for the stick.
 * @param pPoser Camera poser.
 * @param pHolder Player holder.
 * @return Whether the player is rotating the camera.
 */
bool tryGetOwnerStick(sead::Vector2f* pStick, const al::CameraPoserKinopioBrigade* pPoser,
                      const al::PlayerHolder* pHolder) {
    if (pPoser->mStickPort < 1 ||
        al::tryFindAlivePlayerActorFromPort(pHolder, pPoser->mStickPort) == nullptr) {
        return false;
    }

    if (al::isPadTypeJoySingle(pPoser->mStickPort)) {
        if (!al::isPadHoldA(pPoser->mStickPort)) {
            return false;
        }

        *pStick = al::getLeftStick(pPoser->mStickPort);
    } else {
        *pStick = al::getRightStick(pPoser->mStickPort);
    }

    return isStickTilted(*pStick);
}

}  // namespace

namespace al {

/**
 * Creates the camera of the Captain Toad levels, controlled by the stick of any player.
 * @param pPlacementId Placement id of the camera.
 * @param pIsReverseH Whether the horizontal stick axis is reversed.
 * @param pIsReverseV Whether the vertical stick axis is reversed.
 * @param pAudioKeeper Audio keeper the camera sounds are played with.
 * @param pPlayerWatcher Watcher of the players.
 */
CameraPoserKinopioBrigade::CameraPoserKinopioBrigade(const PlacementId* pPlacementId,
                                                     const bool* pIsReverseH,
                                                     const bool* pIsReverseV,
                                                     IUseAudioKeeper* pAudioKeeper,
                                                     PlayerWatcher* pPlayerWatcher)
    : mIsReverseH(pIsReverseH), mIsReverseV(pIsReverseV), mAudioKeeper(pAudioKeeper),
      mPlayerWatcher(pPlayerWatcher) {
    mName = "KinopioBrigade";
    mPlacementId = new PlacementId(*pPlacementId);
    _A2 = true;
}

/**
 * Snaps the horizontal angle to 45 degrees and resets the vertical angle.
 */
void CameraPoserKinopioBrigade::start() {
    getMainControllerPort();
    sead::Vector3f axisX = sead::Vector3f::ex;
    sead::Vector3f axisY = sead::Vector3f::ey;
    sead::Vector3f axisZ = sead::Vector3f::ez;
    f32 padAngleV = calcPadAngleV(axisY, axisZ);
    f32 padAngleH = calcPadAngleH(axisX);
    f32 drcAngleV = wrapAngle180(padAngleV);
    f32 angleH = snapAngle45(mAngleH);
    f32 baseAngleV = 30.0f - drcAngleV;
    mDrcAngleV = drcAngleV;
    mDrcAngleH = padAngleH;
    f32 baseAngleH = angleH - padAngleH;
    mBaseAngleV = baseAngleV;
    mBaseAngleH = baseAngleH;
    mCurrentBaseAngleV = baseAngleV;
    mCurrentBaseAngleH = baseAngleH;
}

/**
 * Gets the index of the player controlling the camera.
 * @return Index of the player.
 */
s32 CameraPoserKinopioBrigade::getCameraOwnerIdx() const {
    return mOwnerIdx;
}

/**
 * Updates the angles from the pad and the sticks of the players.
 */
void CameraPoserKinopioBrigade::update() {
    if (mPlayerWatcher->getAlivePlayerNum() == 0) {
        return;
    }

    if (mWaitFrame >= 1) {
        mWaitFrame--;
        return;
    }

    s32 port = getMainControllerPort();
    sead::Vector3f axisX = mPadAxisX;
    sead::Vector3f axisY = mPadAxisY;
    sead::Vector3f axisZ = mPadAxisZ;
    f32 padAngleV = calcPadAngleV(axisY, axisZ);

    if (padAngleV * mPadAngleV < -10000.0f) {
        padAngleV = mPadAngleV;
    } else {
        mPadAngleV = padAngleV;
    }

    f32 padAngleH = calcPadAngleH(axisX);
    f32 drcAngleV = wrapAngle180(convergeDegree(mDrcAngleV, padAngleV, 0.1f));
    f32 drcAngleH = convergeDegree(mDrcAngleH, padAngleH, 0.1f);
    mDrcAngleV = drcAngleV;
    mDrcAngleH = drcAngleH;

    // The player watcher starts with the player holder.
    const PlayerHolder* holder = *reinterpret_cast<const PlayerHolder* const*>(mPlayerWatcher);
    sead::Vector2f stick = sead::Vector2f(0.0f, 0.0f);

    if (!tryGetOwnerStick(&stick, this, holder)) {
        for (s32 i = 0; i < getMaxControllerPorts(); i++) {
            s32 playerPort = getPlayerControllerPort(i);

            if (tryGetCameraStick(&stick, holder, playerPort) && isStickTilted(stick)) {
                mOwnerIdx = i;
                mStickPort = playerPort;
                break;
            }
        }
    }

    if (*mIsReverseV) {
        stick.y = -stick.y;
    }

    if (*mIsReverseH) {
        stick.x = -stick.x;
    }

    if (isStickTilted(stick)) {
        if (sead::Mathf::abs(stick.x) < sead::Mathf::abs(stick.y)) {
            f32 currentAngleV = drcAngleV + mBaseAngleV;
            f32 diff = stick.y * -1.5f;

            if (isInRange(diff + currentAngleV, 0.0f, 89.0f)) {
                mBaseAngleV += diff;
            } else if (diff > 0.0f && currentAngleV < 0.0f) {
                mCurrentBaseAngleV = 0.0f - drcAngleV;
                mBaseAngleV = mCurrentBaseAngleV + diff;
            } else if (diff < 0.0f && currentAngleV > 89.0f) {
                mCurrentBaseAngleV = 89.0f - drcAngleV;
                mBaseAngleV = mCurrentBaseAngleV + diff;
            }
        } else {
            mBaseAngleH += stick.x * -1.5f;
        }
    }

    if (isPadTriggerPressRightStick(port)) {
        mBaseAngleV = 30.0f - drcAngleV;
        mBaseAngleH = snapAngle45(mAngleH) - drcAngleH;
        startSe(mAudioKeeper, "ResetGyro");
    }

    if (mCurrentBaseAngleV < mBaseAngleV) {
        mCurrentBaseAngleV += 3.0f;

        if (mCurrentBaseAngleV > mBaseAngleV) {
            mCurrentBaseAngleV = mBaseAngleV;
        }
    } else {
        mCurrentBaseAngleV += -3.0f;

        if (mCurrentBaseAngleV < mBaseAngleV) {
            mCurrentBaseAngleV = mBaseAngleV;
        }
    }

    mBaseAngleH = wrapValue(mBaseAngleH, 360.0f);
    mCurrentBaseAngleH = convergeDegree(mCurrentBaseAngleH, mBaseAngleH, 3.0f);
    mAngleV = sead::Mathf::clamp(drcAngleV + mCurrentBaseAngleV, 0.0f, 89.0f);
    mAngleH = drcAngleH + mCurrentBaseAngleH;
    mCameraUp = sead::Vector3f::ey;

    f32 moving =
        sead::Mathf::abs(mAngleV - mPrevAngleV) + sead::Mathf::abs(mAngleH - mPrevAngleH);

    if (moving >= 0.01f && moving < 10.0f) {
        holdSeWithParam(mAudioKeeper, "PgGyroMoving", moving);
    }

    mPrevAngleV = mAngleV;
    mPrevAngleH = mAngleH;
}

/**
 * Places the camera around the look at position at the current angles.
 * @param pCamera Camera to write the pose to.
 */
void CameraPoserKinopioBrigade::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    sead::Vector3f front = sead::Vector3f::ez;

    sead::Quatf quatV;
    quatV.setAxisAngle(-sead::Vector3f::ex, mAngleV);
    sead::Matrix34f mtxV;
    mtxV.fromQuat(quatV);

    sead::Quatf quatH;
    quatH.setAxisAngle(sead::Vector3f::ey, mAngleH);
    sead::Matrix34f mtxH;
    mtxH.fromQuat(quatH);

    sead::Matrix34f mtx;
    mtx.setMul(mtxH, mtxV);
    front.mul(mtx);

    pCamera->setAt(mLookAtPos);
    pCamera->setPos(front * 5500.0f + mLookAtPos);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * Loads the look at position.
 * @param pIter Camera parameter iterator.
 */
void CameraPoserKinopioBrigade::loadParam(const ByamlIter* pIter) {
    CameraPoser::loadParam(pIter);

    ByamlIter lookAtPosIter;

    if (pIter->tryGetIterByKey(&lookAtPosIter, "LookAtPos")) {
        lookAtPosIter.tryGetFloatByKey(&mLookAtPos.x, "X");
        lookAtPosIter.tryGetFloatByKey(&mLookAtPos.y, "Y");
        lookAtPosIter.tryGetFloatByKey(&mLookAtPos.z, "Z");
    }

    mFovyDegree = 30.0f;
}

}  // namespace al
