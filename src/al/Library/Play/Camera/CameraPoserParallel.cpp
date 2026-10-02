#include "Library/Play/Camera/CameraPoserParallel.hpp"

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/Camera/PlayerWatcher.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Camera/ControlAngleParam.hpp"
#include "Project/Camera/Param/CameraFunction.hpp"
#include "Project/Collision/CollisionUtil.hpp"
#include "Project/Collision/HitDb.hpp"

namespace al {

/**
 * Creates the parallel camera.
 * @param rParam The initial camera parameters.
 * @param pProjection The projection of the scene camera.
 * @param pPlayerWatcher The watcher of the players to follow.
 * @param pLookAtCamera The scene camera, used to project players onto the screen.
 * @param pCollisionDirector The collision director used for collision checks.
 * @param pPlacementId The placement id of the camera.
 */
CameraPoserParallel::CameraPoserParallel(const CameraPoserParallelParam& rParam,
                                         const sead::PerspectiveProjection* pProjection,
                                         PlayerWatcher* pPlayerWatcher,
                                         const sead::LookAtCamera* pLookAtCamera,
                                         CollisionDirector* pCollisionDirector,
                                         const PlacementId* pPlacementId)
    : mCollisionDirector(pCollisionDirector), mSceneLookAtCamera(pLookAtCamera),
      mProjection(pProjection), mParam(rParam), mPlayerWatcher(pPlayerWatcher) {
    mLookAtCamera = new sead::LookAtCamera;
    mName = "Parallel";
    mPlacementId = new PlacementId(*pPlacementId);
    _88 = true;
    mControlAngleParam = new ControlAngleParam();
}

/**
 * Allocates the per-player buffers and takes over the common settings.
 * @param pParam The common camera settings.
 */
void CameraPoserParallel::init(const SettingParam* pParam) {
    mSettingParam = pParam;
    mPrevPlayerPos = new sead::Vector3f[mPlayerWatcher->getPlayerNum()];
    mIsPlayerApproachChecked = new bool[mPlayerWatcher->getPlayerNum()];

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        mPrevPlayerPos[i] = sead::Vector3f::zero;
        mIsPlayerApproachChecked[i] = true;
    }

    if (!mIsUseAdvancedSetting) {
        mParam.mSettingParam = *pParam;
    }
}

/**
 * Resets the camera state and runs a first update.
 */
void CameraPoserParallel::start() {
    if (mPlayerWatcher->getAlivePlayerNum() == 0) {
        return;
    }

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if (mPlayerWatcher->isPlayerAlive(i)) {
            mPrevPlayerPos[i] = mPlayerWatcher->getPlayerPos(i);
        }
    }

    mInterpStartLookAtPos = mLookAtPos;
    mBaseLookAtPos = mLookAtPos;
    mPrevLookAtPos = mLookAtPos;
    mParam.mDistance = mParam.mDistanceMin;
    mMoveLimitInterpFrame = 0;
    mMoveLimitReleaseFrame = 60;
    mSpeedCompensationV = 0.0f;
    mIsKeepCameraHeight = false;
    mIsActivateGyroMode = mIsForceGyroMode;
    update();
}

/**
 * Updates the camera with the mode that fits the current players.
 */
void CameraPoserParallel::update() {
    if (mPlayerWatcher->getAlivePlayerNum() == 0) {
        return;
    }

    mAngleHFromZoneToRoot = CameraFunction::calcAngleHFromZoneToRoot(mParam.mAngleH, mZoneMtx);

    if (_9C != -1) {
        updateCourseSelectCamera();
    } else if (mPlayerWatcher->getCameraTargetNum() +
                       mPlayerWatcher->isExistAdditionalCameraLookAtPos() ==
                   1 ||
               mParam.mSettingParam._34 ||
               (mParam.mAngleV < 15.0f && !mParam.mSettingParam._36)) {
        updateSingleCamera();
        mMoveLimitInterpFrame = 0;
        mMoveLimitReleaseFrame = 0;
    } else {
        updateMultiCamera();
    }

    if (mPlayerWatcher->isInCameraRestrictedArea(mPlayerWatcher->getTopPlayerIndex())) {
        _88 = false;
    } else {
        _88 = !mControlAngleParam->mIsInvalidControl;
    }

    mPrevLookAtPos = mLookAtPos;
}

/**
 * Follows the player of the course select screen that owns the camera.
 */
void CameraPoserParallel::updateCourseSelectCamera() {
    sead::Vector3f playerPos = mPlayerWatcher->getPlayerPos(_9C);
    mLookAtPos = playerPos;
    applyCameraOffset();
    mBaseLookAtPos = mLookAtPos;

    if (!mIsFirstCalc) {
        sead::Vector3f move = playerPos - mPrevPlayerPos[_9C];
        sead::Vector3f moveH = {move.x, 0.0f, move.z};
        f32 moveLengthH = moveH.length();

        if (moveLengthH < 100.0f) {
            f32 speedH = moveLengthH * mParam.mSettingParam._18;
            moveH *= mParam.mSettingParam._18;
            f32 prevSpeedH = mSpeedCompensationH;

            if (sead::Mathf::abs(speedH - prevSpeedH) > 30.0f) {
                speedH = prevSpeedH + sgn(speedH - prevSpeedH) * 30.0f;
            }

            normalizeOrZero(&moveH);
            mLookAtPos += moveH * speedH;
            mSpeedCompensationH = speedH;
        }

        if (sead::Mathf::abs(move.y) < 100.0f) {
            sead::Viewport viewport(0.0f, 0.0f, static_cast<u32>(getDisplayWidth()),
                                    static_cast<u32>(getDisplayHeight()));
            sead::Vector2f layoutPos;
            mSceneLookAtCamera->projectByMatrix(&layoutPos, mPlayerWatcher->getTopPlayerPos(),
                                                *mProjection, viewport);
            f32 rate = 0.0f;

            if (move.y < 0.0f && layoutPos.y < -mParam.mSettingParam._24) {
                rate = (-layoutPos.y - mParam.mSettingParam._24) /
                       (static_cast<u32>(getDisplayHeight()) * 0.5f - mParam.mSettingParam._24);
                rate = sead::Mathf::clamp(rate, 0.0f, 1.0f);
            } else if (move.y > 0.0f && layoutPos.y > mParam.mSettingParam._20) {
                rate = (layoutPos.y - mParam.mSettingParam._20) /
                       (static_cast<u32>(getDisplayHeight()) * 0.5f - mParam.mSettingParam._20);
                rate = sead::Mathf::clamp(rate, 0.0f, 1.0f);
            }

            f32 speedV = rate * (move.y * mParam.mSettingParam._1C);
            f32 prevSpeedV = mSpeedCompensationV;

            if (sead::Mathf::abs(speedV - prevSpeedV) > 30.0f) {
                speedV = prevSpeedV + sgn(speedV - prevSpeedV) * 30.0f;
            }

            mLookAtPos.y += speedV;
            mSpeedCompensationV = speedV;
        }
    }

    mPrevPlayerPos[_9C] = playerPos;
    mParam.mDistance = mParam.mDistanceMin;
    mIsDistanceMax = false;
    updateLookAtCamera();
    applyMoveLimit(sead::Vector3f(mParam.mMoveLimitMin), sead::Vector3f(mParam.mMoveLimitMax));
}

/**
 * Follows the top player.
 */
void CameraPoserParallel::updateSingleCamera() {
    mLookAtPos = mPlayerWatcher->getTopPlayerPos();

    if (!mPlayerWatcher->isSetLookAtPosPtr(mPlayerWatcher->getTopPlayerIndex())) {
        mLookAtPos += sead::Vector3f(0.0f, 150.0f, 0.0f);
    }

    applyCameraOffset();
    mBaseLookAtPos = mLookAtPos;

    if (!mIsFirstCalc && !mIsSnapshotMode) {
        const sead::Vector3f& rTopPlayerPos = mPlayerWatcher->getTopPlayerPos();
        sead::Vector3f move = rTopPlayerPos - mPrevPlayerPos[mPlayerWatcher->getTopPlayerIndex()];
        sead::Vector3f moveH = {move.x, 0.0f, move.z};
        f32 moveLengthH = moveH.length();

        if (moveLengthH < 100.0f) {
            f32 speedH = moveLengthH * mParam.mSettingParam._18;
            moveH *= mParam.mSettingParam._18;
            f32 prevSpeedH = mSpeedCompensationH;

            if (sead::Mathf::abs(speedH - prevSpeedH) > 30.0f) {
                speedH = prevSpeedH + sgn(speedH - prevSpeedH) * 30.0f;
            }

            normalizeOrZero(&moveH);
            mLookAtPos += moveH * speedH;
            mSpeedCompensationH = speedH;
        }

        if (sead::Mathf::abs(move.y) < 100.0f) {
            sead::Viewport viewport(0.0f, 0.0f, static_cast<u32>(getDisplayWidth()),
                                    static_cast<u32>(getDisplayHeight()));
            sead::Vector2f layoutPos;
            mSceneLookAtCamera->projectByMatrix(&layoutPos, mPlayerWatcher->getTopPlayerPos(),
                                                *mProjection, viewport);
            f32 rate = 0.0f;

            if (move.y < 0.0f && layoutPos.y < -mParam.mSettingParam._24) {
                rate = (-layoutPos.y - mParam.mSettingParam._24) /
                       (static_cast<u32>(getDisplayHeight()) * 0.5f - mParam.mSettingParam._24);
                rate = sead::Mathf::clamp(rate, 0.0f, 1.0f);
            } else if (move.y > 0.0f && layoutPos.y > mParam.mSettingParam._20) {
                rate = (layoutPos.y - mParam.mSettingParam._20) /
                       (static_cast<u32>(getDisplayHeight()) * 0.5f - mParam.mSettingParam._20);
                rate = sead::Mathf::clamp(rate, 0.0f, 1.0f);
            }

            f32 speedV = rate * (move.y * mParam.mSettingParam._1C);
            f32 prevSpeedV = mSpeedCompensationV;

            if (sead::Mathf::abs(speedV - prevSpeedV) > 30.0f) {
                speedV = prevSpeedV + sgn(speedV - prevSpeedV) * 30.0f;
            }

            mLookAtPos.y += speedV;
            mSpeedCompensationV = speedV;
        }
    }

    if (!mIsSnapshotMode) {
        applyCameraRailOffset();
    }

    const sead::Vector3f& rTopPlayerPos = mPlayerWatcher->getTopPlayerPos();
    mPrevPlayerPos[mPlayerWatcher->getTopPlayerIndex()] = rTopPlayerPos;
    mParam.mDistance = mParam.mDistanceMin;
    mIsDistanceMax = false;
    updateLookAtCamera();
    applyMoveLimit(sead::Vector3f(mParam.mMoveLimitMin), sead::Vector3f(mParam.mMoveLimitMax));

    sead::Vector3f front = sead::Vector3f::ez;

    sead::Quatf quatV;
    quatV.setAxisAngle(-sead::Vector3f::ex, mParam.mAngleV);
    sead::Quatf quatH;
    quatH.setAxisAngle(sead::Vector3f::ey, mAngleHFromZoneToRoot);

    sead::Vector3f cameraPos;

    if (mIsSnapshotMode) {
        cameraPos = mLookAtPos;
        return;
    }

    sead::Matrix34f mtxV;
    mtxV.fromQuat(quatV);
    sead::Matrix34f mtxH;
    mtxH.fromQuat(quatH);
    sead::Matrix34f mtx;
    mtx.setMul(mtxH, mtxV);
    front.mul(mtx);
    cameraPos = mLookAtPos + front * mParam.mDistance;

    if (mIsKeepCameraHeight) {
        sead::Vector3f checkPos = cameraPos;
        checkPos.y = mKeepCameraPos.y;

        if (!checkStrikeCollision(checkPos) &&
            !checkStrikeCollision(checkPos - sead::Vector3f::ey * 2.0f * 100.0f)) {
            mIsKeepCameraHeight = false;
        }

        if (cameraPos.y > mKeepCameraPos.y) {
            mIsKeepCameraHeight = false;
        }
    } else {
        if (checkStrikeCollision(cameraPos) ||
            checkStrikeCollision(cameraPos - sead::Vector3f::ey * 2.0f * 100.0f)) {
            mIsKeepCameraHeight = true;
        }

        mKeepCameraPos = cameraPos;
    }

    mLookAtPos.y += mKeepCameraPos.y - cameraPos.y;
}

/**
 * Follows all players, keeping them on the screen and choosing a base player that the camera
 * prefers to keep inside the base screen.
 */
void CameraPoserParallel::updateMultiCamera() {
    s32 targetNum = mPlayerWatcher->getCameraTargetNum();
    f32 sumY = 0.0f;

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        sead::Vector3f lookAtPos;

        if (mPlayerWatcher->tryGetPlayerLookAtPos(&lookAtPos, i)) {
            sumY += lookAtPos.y;
        }
    }

    if (mPlayerWatcher->isExistAdditionalCameraLookAtPos()) {
        sumY += mPlayerWatcher->getAdditionalCameraLookAtPos().y;
        targetNum++;
    }

    mLookAtPos.y = sumY / targetNum;
    calcLookAtPosToScreenCenterX();
    calcLookAtPosToScreenCenterY();
    calcMultiCameraDistance();

    if (mIsUseOffsetAtMultiMode) {
        applyCameraOffset();
    }

    s32 fastestIndex = -1;

    if (mParam.mSettingParam._35) {
        mIsTopPlayerBase = true;
    } else if (mPlayerWatcher->isExistPriorPlayer()) {
        if (mPlayerWatcher->isPlayerPrior(mPlayerWatcher->getTopPlayerIndex())) {
            mIsTopPlayerBase = true;
        } else {
            mIsTopPlayerBase = false;
        }
    } else {
        sead::Vector2f topPlayerVec = sead::Vector2f::zero;

        if (mPlayerWatcher->getTopPlayerVelocity().length() > 0.5f) {
            calcPlayerVecOutsideBaseScreen(&topPlayerVec, mPlayerWatcher->getTopPlayerIndex());
            f32 scale = calcPlayerScaleInScreen(mPlayerWatcher->getTopPlayerIndex());

            if (scale < 1.0f) {
                topPlayerVec = sead::Vector2f::zero;
            } else {
                topPlayerVec *= 1.0f / scale;
            }

            sead::Vector2f topPlayerDir = topPlayerVec;

            if (!isNearZero(topPlayerDir.length(), 0.001f)) {
                normalize(&topPlayerDir);
                mTopPlayerMoveDir = topPlayerDir;
            }
        } else {
            mTopPlayerMoveDir = sead::Vector2f::zero;
        }

        sead::Vector2f otherPlayerVec = sead::Vector2f::zero;
        f32 maxMoveLength = 0.0f;

        for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
            if (!mPlayerWatcher->isPlayerAlive(i)) {
                continue;
            }

            if (i == mPlayerWatcher->getTopPlayerIndex()) {
                continue;
            }

            if (mPlayerWatcher->getPlayerVelocity(i).length() < 0.5f) {
                continue;
            }

            f32 scale = calcPlayerScaleInScreen(i);

            if (scale < 1.0f) {
                continue;
            }

            sead::Vector2f moveVec = sead::Vector2f::zero;
            calcPlayerVecOnScreen(&moveVec, i);
            sead::Vector2f parallelVec = sead::Vector2f::zero;

            if (mTopPlayerMoveDir.x != 0.0f || mTopPlayerMoveDir.y != 0.0f) {
                f32 dot = mTopPlayerMoveDir.x * moveVec.x + mTopPlayerMoveDir.y * moveVec.y;

                if (dot > 0.0f) {
                    f32 invScale = 1.0f / scale;
                    parallelVec.x = mTopPlayerMoveDir.x * dot;
                    parallelVec.y = mTopPlayerMoveDir.y * dot;
                    topPlayerVec += parallelVec * invScale;
                }
            }

            sead::Vector2f verticalVec = moveVec - parallelVec;
            sead::Vector2f layoutPos;
            calcPlayerLayoutPos(&layoutPos, i);
            calcVecOutsideBaseScreen(&verticalVec, layoutPos);
            otherPlayerVec += verticalVec * (1.0f / scale);

            if (maxMoveLength < moveVec.length()) {
                fastestIndex = i;
                maxMoveLength = moveVec.length();
            }
        }

        if (mBasePlayerIndex == -1) {
            mBasePlayerIndex = fastestIndex;
        }

        if (mBasePlayerIndex != -1 && !mPlayerWatcher->isPlayerAlive(mBasePlayerIndex)) {
            mBasePlayerIndex = -1;
        }

        f32 diff = topPlayerVec.length() * mParam.mSettingParam._C - otherPlayerVec.length();

        if (mBaseChangeFrame >= 66) {
            if (mIsTopPlayerBase && (diff < -0.01f ||
                         !mPlayerWatcher->isPlayerAlive(mPlayerWatcher->getTopPlayerIndex()))) {
                if (mBaseCandidateIndex == fastestIndex || mBaseCandidateIndex == -1) {
                    mIsTopPlayerBase = false;
                    mBaseChangeFrame = 0;
                    mMoveLimitInterpFrame = 0;
                }
            }

            if (!mIsTopPlayerBase && diff > 0.005f) {
                if (mBaseCandidateIndex == mPlayerWatcher->getTopPlayerIndex() ||
                    mBaseCandidateIndex == -1) {
                    mIsTopPlayerBase = true;
                    mBaseChangeFrame = 0;
                    mMoveLimitInterpFrame = 0;
                }
            }
        }

        if (diff > 0.05f) {
            mBaseCandidateIndex = mPlayerWatcher->getTopPlayerIndex();
        } else if (diff < -0.01f) {
            mBaseCandidateIndex = fastestIndex;
        } else {
            mBaseCandidateIndex = -1;
        }
    }

    if (mIsDistanceMax && !mIsFirstCalc) {
        if (mIsTopPlayerBase) {
            mBasePlayerInterpFrame = 0;

            if (mTopPlayerInterpFrame == 0) {
                mInterpStartLookAtPos = mBaseLookAtPos;
            }

            calcLookAtPosToPutPlayerInBaseScreenX(mPlayerWatcher->getTopPlayerIndex());
            calcLookAtPosToPutPlayerInBaseScreenY(mPlayerWatcher->getTopPlayerIndex());
            f32 rate = sead::Mathf::clamp(
                static_cast<f32>(mTopPlayerInterpFrame) / mParam.mSettingParam._10, 0.0f, 1.0f);
            rate = easeInOut(rate);
            mLookAtPos = mInterpStartLookAtPos * (1.0f - rate) + mLookAtPos * rate;
            mTopPlayerInterpFrame++;
        } else {
            mTopPlayerInterpFrame = 0;

            if (mPlayerWatcher->isExistPriorPlayer()) {
                f32 maxSpeed = -1.0f;

                for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
                    if (!mPlayerWatcher->isPlayerPrior(i) ||
                        i == mPlayerWatcher->getTopPlayerIndex()) {
                        continue;
                    }

                    f32 speed = mPlayerWatcher->getPlayerVelocity(i).length();

                    if (maxSpeed < speed) {
                        maxSpeed = speed;
                        mBasePlayerIndex = i;
                    }
                }
            } else if (mBasePlayerIndex == -1) {
                mBasePlayerIndex = fastestIndex;
            } else if (fastestIndex != -1) {
                sead::Vector2f baseVec = sead::Vector2f::zero;
                calcPlayerVecOutsideBaseScreen(&baseVec, mBasePlayerIndex);
                sead::Vector2f fastestVec = sead::Vector2f::zero;
                calcPlayerVecOutsideBaseScreen(&fastestVec, fastestIndex);

                if (mBaseChangeFrame >= 66 && baseVec.length() + 2.0f < fastestVec.length()) {
                    mBasePlayerIndex = fastestIndex;
                    mBasePlayerInterpFrame = 0;
                    mBaseChangeFrame = 0;
                }
            }

            if (mBasePlayerIndex == -1) {
                mBasePlayerInterpFrame = 0;
            } else {
                if (mBasePlayerInterpFrame == 0) {
                    mInterpStartLookAtPos = mBaseLookAtPos;
                }

                calcLookAtPosToPutPlayerInBaseScreenX(mBasePlayerIndex);
                calcLookAtPosToPutPlayerInBaseScreenY(mBasePlayerIndex);
                f32 rate = sead::Mathf::clamp(
                    static_cast<f32>(mBasePlayerInterpFrame) / mParam.mSettingParam._14, 0.0f,
                    1.0f);
                rate = easeInOut(rate);
                mLookAtPos = mInterpStartLookAtPos * (1.0f - rate) + mLookAtPos * rate;
                mBasePlayerInterpFrame++;
            }
        }
    } else {
        mTopPlayerInterpFrame = 0;
        mBasePlayerInterpFrame = 0;
    }

    mBaseLookAtPos = mLookAtPos;
    mBaseChangeFrame++;
    calcSpeedCompensationMulti();

    f32 scale = calcPlayerScaleInScreen(mPlayerWatcher->getTopPlayerIndex());

    if (mIsTopPlayerBase) {
        if (scale > 0.0f && scale < mParam.mSettingParam._30) {
            sead::Vector3f savedLookAtPos = mLookAtPos;
            sead::Vector3f cameraDir = mLookAtCamera->getAt() - mLookAtCamera->getPos();
            cameraDir.y = 0.0f;
            normalize(&cameraDir);
            updateLookAtCamera();

            sead::Vector3f topLookAtPos;
            mPlayerWatcher->getTopPlayerLookAtPos(&topLookAtPos);
            sead::Vector2f layoutPos;
            calcWorldPosToLayoutPos(&layoutPos, topLookAtPos);
            f32 step = 100.0f;

            while (scale < mParam.mSettingParam._30 && layoutPos.y > 0.0f) {
                sead::Vector3f prevLookAtPos = mLookAtPos;
                mLookAtPos += step * cameraDir;
                updateLookAtCamera();

                if (step < 1.0f) {
                    break;
                }

                f32 newScale = calcPlayerScaleInScreen(mPlayerWatcher->getTopPlayerIndex());

                if (newScale > mParam.mSettingParam._30 + 0.1f) {
                    mLookAtPos = prevLookAtPos;
                    step *= 0.5f;
                } else {
                    calcWorldPosToLayoutPos(&layoutPos, topLookAtPos);
                    scale = newScale;
                }
            }

            f32 rate = easeInOut(static_cast<f32>(180 - mHeightAdjustFrame) / 180.0f);
            mLookAtPos = savedLookAtPos * rate + mLookAtPos * (1.0f - rate);
            mHeightAdjustFrame = sead::Mathi::clamp(mHeightAdjustFrame + 1, 0, 180);
        } else {
            mHeightAdjustFrame = 0;
        }
    } else {
        mHeightAdjustFrame = 0;
    }

    sead::Vector3f limitMin;
    sead::Vector3f limitMax;
    calcMoveLimitValueForMulti(&limitMin, &limitMax);
    updateLookAtCamera();
    f32 prevLookAtZ = mLookAtPos.z;
    applyMoveLimit(limitMin, limitMax);

    if (!mIsFirstCalc) {
        if (prevLookAtZ - mLookAtPos.z > 1.0f) {
            mMoveLimitReleaseFrame = 0;

            if (mMoveLimitInterpFrame == 0) {
                mMoveLimitInterpStartPos = mPrevLookAtPos;
            }

            includeBaseAfterMoveLimit();
            f32 rate = sead::Mathf::clamp(mMoveLimitInterpFrame / 60.0f, 0.0f, 1.0f);
            mLookAtPos.x = sead::lerp(mMoveLimitInterpStartPos.x, mLookAtPos.x, rate);
            mLookAtPos.y = sead::lerp(mMoveLimitInterpStartPos.y, mLookAtPos.y, rate);
            mLookAtPos.z = sead::lerp(mMoveLimitInterpStartPos.z, mLookAtPos.z, rate);
            applyMoveLimit(limitMin, limitMax);
            mMoveLimitInterpFrame++;
        } else {
            mMoveLimitInterpFrame = 0;

            if (mMoveLimitReleaseFrame == 0) {
                mMoveLimitInterpStartPos = mPrevLookAtPos;
            }

            f32 rate = sead::Mathf::clamp(mMoveLimitReleaseFrame / 60.0f, 0.0f, 1.0f);
            mLookAtPos.x = sead::lerp(mMoveLimitInterpStartPos.x, mLookAtPos.x, rate);
            mLookAtPos.y = sead::lerp(mMoveLimitInterpStartPos.y, mLookAtPos.y, rate);
            mLookAtPos.z = sead::lerp(mMoveLimitInterpStartPos.z, mLookAtPos.z, rate);
            mMoveLimitReleaseFrame++;
        }
    }

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        mPlayerWatcher->getPlayerLookAtPos(&mPrevPlayerPos[i], i);
    }
}

/**
 * Enters the snapshot mode.
 * @param fovy The field of view of the snapshot camera.
 */
void CameraPoserParallel::startSnapshotMode(f32 fovy) {
    mIsSnapshotMode = true;
    mIsNoNormalInterpoleBeforeSnapshot = mIsNoNormalInterpole;
    mIsNoNormalInterpole = true;
}

/**
 * Leaves the snapshot mode.
 */
void CameraPoserParallel::endSnapshotMode() {
    CameraPoser::endSnapshotMode();
    mIsNoNormalInterpole = mIsNoNormalInterpoleBeforeSnapshot;
}

/**
 * Replaces the camera parameters.
 * @param rParam The new parameters.
 */
void CameraPoserParallel::setParam(const CameraPoserParallelParam& rParam) {
    copyMemory(&mParam, &rParam, sizeof(CameraPoserParallelParam));
}

/**
 * Forces the camera to always follow a single player.
 */
void CameraPoserParallel::changeSingleCameraMode() {
    mIsUseAdvancedSetting = true;
    mParam.mSettingParam._34 = true;
}

/**
 * Writes the current pose to the internal look at camera and updates its view matrix.
 */
void CameraPoserParallel::updateLookAtCamera() {
    makeLookAtCamera(mLookAtCamera);
    mLookAtCamera->updateViewMatrix();
}

/**
 * Shifts the look at position by the look at offset in camera space.
 */
void CameraPoserParallel::applyCameraOffset() {
    sead::Matrix34f viewMtx = mLookAtCamera->getMatrix();
    sead::Matrix34f invViewMtx;
    invViewMtx.setInverse(viewMtx);

    sead::Vector3f viewPos;
    viewPos.setMul(viewMtx, mLookAtPos);
    viewPos.x += mParam.mLookAtOffsetX;
    viewPos.y += mParam.mLookAtOffsetY;
    mLookAtPos.setMul(invViewMtx, viewPos);
}

/**
 * Moves the look at position ahead along the camera rail by the players' movement.
 */
void CameraPoserParallel::applyCameraRailOffset() {
    sead::Vector3f railDir = mPlayerWatcher->getCameraRailDir();
    railDir.y = 0.0f;
    normalizeOrDirZ(&railDir);
    sead::Vector3f move = sead::Vector3f::zero;

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if (!mPlayerWatcher->isPlayerAlive(i)) {
            continue;
        }

        sead::Vector3f lookAtPos;
        mPlayerWatcher->getPlayerLookAtPos(&lookAtPos, i);
        sead::Vector3f playerMove = lookAtPos - mPrevPlayerPos[i];
        sead::Vector3f railMove;
        parallelizeVec(&railMove, railDir, playerMove);

        if (railMove.length() > 100.0f) {
            return;
        }

        move += railMove;
    }

    mCameraRailOffset += move * 0.35f;
    f32 length = mCameraRailOffset.length();

    if (length > 200.0f) {
        mCameraRailOffset *= 200.0f;
        mCameraRailOffset *= 1.0f / length;
    }

    mLookAtPos += mCameraRailOffset;
}

/**
 * Moves the look at position so that the camera stays inside the move limit box of the zone.
 * @param rMin The minimum of the move limit box.
 * @param rMax The maximum of the move limit box.
 */
void CameraPoserParallel::applyMoveLimit(const sead::Vector3f& rMin, const sead::Vector3f& rMax) {
    sead::Vector3f front = sead::Vector3f::ez;
    sead::Vector3f lookAtPos = mLookAtPos;
    f32 angleV = mParam.mAngleV;
    f32 angleH = mAngleHFromZoneToRoot;

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

    sead::Vector3f cameraPos = lookAtPos + mParam.mDistance * front;
    sead::Matrix34f invZoneMtx;
    invZoneMtx.setInverse(mZoneMtx);
    sead::Vector3f localPos = invZoneMtx * cameraPos;
    rotateVectorDegreeY(&localPos, -mParam.mAngleH);
    rotateVectorDegreeY(&localPos, -mParam.mMoveLimitRotateDegreeY);
    clampV3f(&localPos, rMin, rMax);
    rotateVectorDegreeY(&localPos, mParam.mMoveLimitRotateDegreeY);
    rotateVectorDegreeY(&localPos, mParam.mAngleH);

    sead::Vector3f limitedPos;
    limitedPos.setMul(mZoneMtx, localPos);
    mLookAtPos = lookAtPos + (limitedPos - cameraPos);
}

/**
 * Checks whether a valid floor collision is near the position.
 * @param rPos The position to check.
 * @return Whether a floor facing upwards was hit.
 */
bool CameraPoserParallel::checkStrikeCollision(const sead::Vector3f& rPos) {
    s32 hitNum = alCollisionUtil::checkStrikeSphere(this, rPos, 100.0f, nullptr, nullptr);

    if (hitNum != 0 && CameraFunction::checkValidCollisionBySphereHitInfo(this, hitNum)) {
        const SphereHitInfo* pHitInfo = alCollisionUtil::getStrikeSphereInfo(this, 0);
        sead::Vector3f normal = *pHitInfo->mTriangle.getNormal(0);

        if (normal.dot(sead::Vector3f::ey) > 0.0f) {
            return true;
        }
    }

    return false;
}

/**
 * Moves the look at position sideways until the players are horizontally centered on the screen.
 */
void CameraPoserParallel::calcLookAtPosToScreenCenterX() {
    updateLookAtCamera();
    sead::BoundBox2f layoutBox;
    calcLayoutBoxIncludeAllPlayer(&layoutBox, nullptr);
    f32 center = layoutBox.getCenter().x;

    if (center > 0.1f) {
        f32 step = 100.0f;

        s32 i = 0;

        while (center > 0.1f) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos.x += step * sead::Mathf::cos(sead::Mathf::deg2rad(mAngleHFromZoneToRoot));
            mLookAtPos.z -= step * sead::Mathf::sin(sead::Mathf::deg2rad(mAngleHFromZoneToRoot));
            updateLookAtCamera();

            if (i++ >= 60) {
                calcPlayerWorldCenterPos(&mLookAtPos);
                break;
            }

            sead::BoundBox2f box;
            calcLayoutBoxIncludeAllPlayer(&box, nullptr);
            f32 newCenter = box.getCenter().x;

            if (newCenter < -0.1f) {
                step *= 0.5f;
                mLookAtPos = prevLookAtPos;
            } else {
                center = newCenter;
            }
        }
    }

    if (center < -0.1f) {
        f32 step = 100.0f;

        s32 i = 0;

        while (center < -0.1f) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos.x -= step * sead::Mathf::cos(sead::Mathf::deg2rad(mAngleHFromZoneToRoot));
            mLookAtPos.z += step * sead::Mathf::sin(sead::Mathf::deg2rad(mAngleHFromZoneToRoot));
            updateLookAtCamera();

            if (i++ >= 60) {
                calcPlayerWorldCenterPos(&mLookAtPos);
                break;
            }

            sead::BoundBox2f box;
            calcLayoutBoxIncludeAllPlayer(&box, nullptr);
            f32 newCenter = box.getCenter().x;

            if (newCenter > 0.1f) {
                step *= 0.5f;
                mLookAtPos = prevLookAtPos;
            } else {
                center = newCenter;
            }
        }
    }
}

/**
 * Moves the look at position along the view until the players are vertically centered on the
 * screen.
 */
void CameraPoserParallel::calcLookAtPosToScreenCenterY() {
    updateLookAtCamera();
    sead::BoundBox2f layoutBox;
    calcLayoutBoxIncludeAllPlayer(&layoutBox, nullptr);
    f32 center = layoutBox.getCenter().y;

    if (center > 0.1f) {
        f32 step = 100.0f;

        s32 i = 0;

        while (center > 0.1f) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos.z -= step * sead::Mathf::cos(sead::Mathf::deg2rad(mAngleHFromZoneToRoot));
            mLookAtPos.x -= step * sead::Mathf::sin(sead::Mathf::deg2rad(mAngleHFromZoneToRoot));
            updateLookAtCamera();

            if (i++ >= 60) {
                calcPlayerWorldCenterPos(&mLookAtPos);
                break;
            }

            sead::BoundBox2f box;
            calcLayoutBoxIncludeAllPlayer(&box, nullptr);
            f32 newCenter = box.getCenter().y;

            if (newCenter < -0.1f) {
                step *= 0.5f;
                mLookAtPos = prevLookAtPos;
            } else {
                center = newCenter;
            }
        }
    }

    if (center < -0.1f) {
        f32 step = 100.0f;

        s32 i = 0;

        while (center < -0.1f) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos.z += step * sead::Mathf::cos(sead::Mathf::deg2rad(mAngleHFromZoneToRoot));
            mLookAtPos.x += step * sead::Mathf::sin(sead::Mathf::deg2rad(mAngleHFromZoneToRoot));
            updateLookAtCamera();

            if (i++ >= 60) {
                calcPlayerWorldCenterPos(&mLookAtPos);
                break;
            }

            sead::BoundBox2f box;
            calcLayoutBoxIncludeAllPlayer(&box, nullptr);
            f32 newCenter = box.getCenter().y;

            if (newCenter > 0.1f) {
                step *= 0.5f;
                mLookAtPos = prevLookAtPos;
            } else {
                center = newCenter;
            }
        }
    }
}

/**
 * Pulls the camera back until all players fit into the layout box, and keeps the distance for a
 * while before letting the camera approach again.
 */
void CameraPoserParallel::calcMultiCameraDistance() {
    f32 prevDistance = mParam.mDistance;
    f32 distanceMax = mParam.mDistanceMax;

    if (checkEnableCameraApproach()) {
        mParam.mDistance = mParam.mDistanceMin;
    }

    updateLookAtCamera();

    for (s32 i = 0; mParam.mDistance < mParam.mDistanceMax; i++) {
        if (isAllPlayerInLayoutBox()) {
            break;
        }

        f32 step = i == 0 ? 300.0f : 700.0f;
        mParam.mDistance += step;
        updateLookAtCamera();

        if (!isExistCameraPosWithinMoveLimitForMulti()) {
            mParam.mDistance -= step;
            distanceMax = mParam.mDistance;
            break;
        }

        if (mParam.mDistance >= mParam.mDistanceMax) {
            mParam.mDistance = mParam.mDistanceMax;
            break;
        }
    }

    mIsDistanceMax = mParam.mDistance == distanceMax;

    if (prevDistance < mParam.mDistance) {
        mDistanceKeepFrame = 0;
        return;
    }

    if (mDistanceKeepFrame < 180) {
        mDistanceKeepFrame++;
        mParam.mDistance = prevDistance;
        return;
    }

    if (!mIsTopPlayerBase) {
        return;
    }

    sead::Vector3f lookAtPos;
    mPlayerWatcher->getPlayerLookAtPos(&lookAtPos, mPlayerWatcher->getTopPlayerIndex());
    sead::Vector3f headPos = lookAtPos + sead::Vector3f(0.0f, 150.0f, 0.0f);
    sead::Viewport viewport(0.0f, 0.0f, static_cast<u32>(getDisplayWidth()),
                            static_cast<u32>(getDisplayHeight()));
    sead::Vector2f lookAtLayoutPos;
    sead::Vector2f headLayoutPos;
    mLookAtCamera->projectByMatrix(&lookAtLayoutPos, lookAtPos, *mProjection, viewport);
    mLookAtCamera->projectByMatrix(&headLayoutPos, headPos, *mProjection, viewport);

    if (headLayoutPos.y - lookAtLayoutPos.y < mParam.mSettingParam._30 + 10.0f) {
        mParam.mDistance = prevDistance;
    }
}

/**
 * Calculates the screen movement of a player, keeping only the components that move it further
 * away from the screen center.
 * @param pOut The screen movement.
 * @param index The player index.
 */
void CameraPoserParallel::calcPlayerVecOutsideBaseScreen(sead::Vector2f* pOut, s32 index) const {
    calcPlayerVecOnScreen(pOut, index);
    sead::Vector2f layoutPos;
    calcPlayerLayoutPos(&layoutPos, index);
    calcVecOutsideBaseScreen(pOut, layoutPos);
}

/**
 * Calculates the height of a player on the screen.
 * @param index The player index.
 * @return The screen height of the player.
 */
f32 CameraPoserParallel::calcPlayerScaleInScreen(s32 index) const {
    sead::Vector3f lookAtPos;
    mPlayerWatcher->getPlayerLookAtPos(&lookAtPos, index);
    sead::Vector2f lookAtLayoutPos;
    sead::Vector3f headPos = lookAtPos + sead::Vector3f(0.0f, 150.0f, 0.0f);
    sead::Vector2f headLayoutPos;
    sead::Viewport viewport(0.0f, 0.0f, static_cast<u32>(getDisplayWidth()),
                            static_cast<u32>(getDisplayHeight()));
    mLookAtCamera->projectByMatrix(&lookAtLayoutPos, lookAtPos, *mProjection, viewport);
    mLookAtCamera->projectByMatrix(&headLayoutPos, headPos, *mProjection, viewport);
    return headLayoutPos.y - lookAtLayoutPos.y;
}

/**
 * Calculates how far a player moved on the screen since the last frame.
 * @param pOut The screen movement.
 * @param index The player index.
 */
void CameraPoserParallel::calcPlayerVecOnScreen(sead::Vector2f* pOut, s32 index) const {
    sead::Vector2f layoutPos;
    sead::Vector2f prevLayoutPos;
    calcWorldPosToLayoutPos(&prevLayoutPos, mPrevPlayerPos[index]);
    calcPlayerLayoutPos(&layoutPos, index);
    *pOut = layoutPos - prevLayoutPos;
}

/**
 * Calculates the screen position of a player.
 * @param pOut The screen position.
 * @param index The player index.
 */
void CameraPoserParallel::calcPlayerLayoutPos(sead::Vector2f* pOut, s32 index) const {
    sead::Vector3f lookAtPos;
    mPlayerWatcher->getPlayerLookAtPos(&lookAtPos, index);
    calcWorldPosToLayoutPos(pOut, lookAtPos);
}

/**
 * Removes the components of a screen movement that move towards the screen center.
 * @param pOut The screen movement to modify.
 * @param rLayoutPos The screen position.
 */
void CameraPoserParallel::calcVecOutsideBaseScreen(sead::Vector2f* pOut,
                                                   const sead::Vector2f& rLayoutPos) const {
    if (rLayoutPos.x > 0.0f && pOut->x < 0.0f) {
        pOut->x = 0.0f;
    }

    if (rLayoutPos.x <= 0.0f && pOut->x > 0.0f) {
        pOut->x = 0.0f;
    }

    if (rLayoutPos.y > 0.0f && pOut->y < 0.0f) {
        pOut->y = 0.0f;
    }

    if (rLayoutPos.y <= 0.0f && pOut->y > 0.0f) {
        pOut->y = 0.0f;
    }
}

/**
 * Moves the look at position towards a player until it is horizontally inside the base screen.
 * @param index The player index.
 */
void CameraPoserParallel::calcLookAtPosToPutPlayerInBaseScreenX(s32 index) {
    updateLookAtCamera();
    sead::Vector2f layoutPos;
    calcPlayerLayoutPos(&layoutPos, index);

    if (layoutPos.x > mParam.mSettingParam._0) {
        f32 rate = 0.5f;

        s32 i = 0;

        while (layoutPos.x > mParam.mSettingParam._0) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            sead::Vector3f playerLookAtPos;
            mPlayerWatcher->getPlayerLookAtPos(&playerLookAtPos, index);
            mLookAtPos = (1.0f - rate) * mLookAtPos + rate * playerLookAtPos;
            updateLookAtCamera();

            if (i++ >= 60) {
                break;
            }

            sead::Vector2f newLayoutPos;
            calcPlayerLayoutPos(&newLayoutPos, index);

            if (newLayoutPos.x < mParam.mSettingParam._0 - 0.1f) {
                mLookAtPos = prevLookAtPos;
                rate *= 0.5f;
            } else {
                layoutPos.x = newLayoutPos.x;
            }
        }
    }

    if (layoutPos.x < -mParam.mSettingParam._0) {
        f32 rate = 0.5f;

        s32 i = 0;

        while (layoutPos.x < -mParam.mSettingParam._0) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            sead::Vector3f playerLookAtPos;
            mPlayerWatcher->getPlayerLookAtPos(&playerLookAtPos, index);
            mLookAtPos = (1.0f - rate) * mLookAtPos + rate * playerLookAtPos;
            updateLookAtCamera();

            if (i++ >= 60) {
                break;
            }

            sead::Vector2f newLayoutPos;
            calcPlayerLayoutPos(&newLayoutPos, index);

            if (newLayoutPos.x > 0.1f - mParam.mSettingParam._0) {
                mLookAtPos = prevLookAtPos;
                rate *= 0.5f;
            } else {
                layoutPos.x = newLayoutPos.x;
            }
        }
    }
}

/**
 * Moves the look at position towards a player until it is vertically inside the base screen.
 * @param index The player index.
 */
void CameraPoserParallel::calcLookAtPosToPutPlayerInBaseScreenY(s32 index) {
    updateLookAtCamera();
    sead::Vector2f layoutPos;
    calcPlayerLayoutPos(&layoutPos, index);

    if (layoutPos.y > mParam.mSettingParam._4) {
        f32 rate = 0.5f;

        s32 i = 0;

        while (layoutPos.y > mParam.mSettingParam._4) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            sead::Vector3f playerLookAtPos;
            mPlayerWatcher->getPlayerLookAtPos(&playerLookAtPos, index);
            mLookAtPos = (1.0f - rate) * mLookAtPos + rate * playerLookAtPos;
            updateLookAtCamera();

            if (i++ >= 60) {
                break;
            }

            sead::Vector2f newLayoutPos;
            calcPlayerLayoutPos(&newLayoutPos, index);

            if (newLayoutPos.y < mParam.mSettingParam._4 - 0.1f) {
                mLookAtPos = prevLookAtPos;
                rate *= 0.5f;
            } else {
                layoutPos.y = newLayoutPos.y;
            }
        }
    }

    if (layoutPos.y < -mParam.mSettingParam._8) {
        f32 rate = 0.5f;

        s32 i = 0;

        while (layoutPos.y < -mParam.mSettingParam._8) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            sead::Vector3f playerLookAtPos;
            mPlayerWatcher->getPlayerLookAtPos(&playerLookAtPos, index);
            mLookAtPos = (1.0f - rate) * mLookAtPos + rate * playerLookAtPos;
            updateLookAtCamera();

            if (i++ >= 60) {
                break;
            }

            sead::Vector2f newLayoutPos;
            calcPlayerLayoutPos(&newLayoutPos, index);

            if (newLayoutPos.y > 0.1f - mParam.mSettingParam._8) {
                mLookAtPos = prevLookAtPos;
                rate *= 0.5f;
            } else {
                layoutPos.y = newLayoutPos.y;
            }
        }
    }
}

/**
 * Moves the look at position ahead of the players' movement in multi player mode.
 */
void CameraPoserParallel::calcSpeedCompensationMulti() {
    if (mParam.mSettingParam._35) {
        sead::Vector3f lookAtPos;
        mPlayerWatcher->getTopPlayerLookAtPos(&lookAtPos);
        sead::Vector3f move = lookAtPos - mPrevPlayerPos[mPlayerWatcher->getTopPlayerIndex()];
        sead::Vector3f moveH = {move.x, 0.0f, move.z};
        f32 moveLengthH = moveH.length();

        if (moveLengthH < 100.0f) {
            f32 speedH = moveLengthH * mParam.mSettingParam._28;
            f32 prevSpeedH = mSpeedCompensationH;

            if (sead::Mathf::abs(speedH - prevSpeedH) > 30.0f) {
                speedH = prevSpeedH + sgn(speedH - prevSpeedH) * 30.0f;
            }

            normalizeOrZero(&moveH);
            mLookAtPos += moveH * speedH;
            mSpeedCompensationH = speedH;
        }
    } else {
        sead::Vector3f moveSum = sead::Vector3f::zero;
        s32 moveNum = 0;

        for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
            if (!mPlayerWatcher->isPlayerAlive(i)) {
                continue;
            }

            sead::Vector3f lookAtPos;
            mPlayerWatcher->getPlayerLookAtPos(&lookAtPos, i);
            sead::Vector3f move = lookAtPos - mPrevPlayerPos[i];

            if (move.length() > 100.0f) {
                continue;
            }

            moveSum += move;
            moveNum++;
        }

        if (moveNum > 0) {
            moveSum *= 1.0f / moveNum;
            moveSum.y = 0.0f;
            f32 speedH = moveSum.length() * mParam.mSettingParam._28;
            f32 prevSpeedH = mSpeedCompensationH;

            if (sead::Mathf::abs(speedH - prevSpeedH) > 30.0f) {
                speedH = prevSpeedH + sgn(speedH - prevSpeedH) * 30.0f;
            }

            normalizeOrZero(&moveSum);
            mLookAtPos += moveSum * speedH;
            mSpeedCompensationH = speedH;
        }
    }

    sead::Viewport viewport(0.0f, 0.0f, static_cast<u32>(getDisplayWidth()),
                            static_cast<u32>(getDisplayHeight()));
    f32 speedV = 0.0f;
    s32 speedNum = 0;

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if (!mPlayerWatcher->isPlayerAlive(i)) {
            continue;
        }

        sead::Vector3f playerPos;
        sead::Vector2f playerLayoutPos;
        sead::Vector2f lookAtLayoutPos;
        sead::Vector3f lookAtPos;
        mPlayerWatcher->getPlayerLookAtPos(&lookAtPos, i);
        playerPos = mPlayerWatcher->getPlayerPos(i);
        mSceneLookAtCamera->projectByMatrix(&playerLayoutPos, playerPos, *mProjection, viewport);
        mSceneLookAtCamera->projectByMatrix(&lookAtLayoutPos, lookAtPos, *mProjection, viewport);

        f32 moveV;

        if (mPlayerWatcher->isFallBothPlayerAndLookAt(i)) {
            moveV = mPlayerWatcher->getPlayerVelocity(i).y;
        } else {
            moveV = lookAtPos.y - mPrevPlayerPos[i].y;
        }

        if (sead::Mathf::abs(moveV) > 100.0f) {
            continue;
        }

        f32 playerSpeedV = moveV * mParam.mSettingParam._2C;
        f32 rate = 0.0f;
        f32 layoutY = lookAtLayoutPos.y;
        f32 maxBottomY = mParam.mSettingParam._24;

        if (playerSpeedV < 0.0f && layoutY < -maxBottomY) {
            rate = (-playerLayoutPos.y - maxBottomY) /
                   (static_cast<u32>(getDisplayHeight()) * 0.5f - mParam.mSettingParam._24);
            rate = sead::Mathf::clamp(rate, 0.0f, 1.0f);
        } else if (playerSpeedV > 0.0f && layoutY > mParam.mSettingParam._20) {
            rate = (playerLayoutPos.y - mParam.mSettingParam._20) /
                   (static_cast<u32>(getDisplayHeight()) * 0.5f - mParam.mSettingParam._20);
            rate = sead::Mathf::clamp(rate, 0.0f, 1.0f);
        }

        speedV += playerSpeedV * rate;
        speedNum++;
    }

    if (speedNum > 0) {
        speedV /= speedNum;
    }

    f32 prevSpeedV = mSpeedCompensationV;

    if (sead::Mathf::abs(speedV - prevSpeedV) > 30.0f) {
        speedV = prevSpeedV + sgn(speedV - prevSpeedV) * 30.0f;
    }

    mLookAtPos.y += speedV;
    mSpeedCompensationV = speedV;
}

/**
 * Projects a world position onto the screen.
 * @param pOut The screen position.
 * @param rWorldPos The world position.
 */
void CameraPoserParallel::calcWorldPosToLayoutPos(sead::Vector2f* pOut,
                                                  const sead::Vector3f& rWorldPos) const {
    sead::Viewport viewport(0.0f, 0.0f, static_cast<u32>(getDisplayWidth()),
                            static_cast<u32>(getDisplayHeight()));
    mLookAtCamera->projectByMatrix(pOut, rWorldPos, *mProjection, viewport);
}

/**
 * Calculates the move limit box of the look at position in multi player mode, shrunk by the area
 * that is visible around it.
 * @param pMin The minimum of the move limit box.
 * @param pMax The maximum of the move limit box.
 */
void CameraPoserParallel::calcMoveLimitValueForMulti(sead::Vector3f* pMin,
                                                     sead::Vector3f* pMax) const {
    sead::Vector3f limitMin = mParam.mMoveLimitMinForMulti;
    sead::Vector3f limitMax = mParam.mMoveLimitMaxForMulti;
    f32 tanFovyX = sead::Mathf::tan(sead::Mathf::deg2rad(mFovyDegree * 0.5f)) *
                   static_cast<u32>(getDisplayWidth()) / static_cast<u32>(getDisplayHeight());
    f32 fovyX = sead::Mathf::rad2deg(sead::Mathf::atan2(tanFovyX, 1.0f));

    limitMax.x -= (mParam.mDistance - mParam.mDistanceMin) *
                  sead::Mathf::tan(sead::Mathf::deg2rad(fovyX + mParam.mMoveLimitRotateDegreeY));
    limitMin.x += (mParam.mDistance - mParam.mDistanceMin) *
                  sead::Mathf::tan(sead::Mathf::deg2rad(fovyX - mParam.mMoveLimitRotateDegreeY));
    limitMax.y += (mParam.mDistance - mParam.mDistanceMin) *
                  sead::Mathf::cos(sead::Mathf::deg2rad(mParam.mAngleV)) *
                  sead::Mathf::tan(sead::Mathf::deg2rad(mParam.mAngleV - mFovyDegree * 0.5f));
    limitMin.y += (mParam.mDistance - mParam.mDistanceMin) *
                  sead::Mathf::cos(sead::Mathf::deg2rad(mParam.mAngleV)) *
                  sead::Mathf::tan(sead::Mathf::deg2rad(mParam.mAngleV + mFovyDegree * 0.5f));
    limitMin.z += mParam.mDistance - mParam.mDistanceMin;
    limitMax.z += mParam.mDistance - mParam.mDistanceMin;
    *pMin = limitMin;
    *pMax = limitMax;
}

/**
 * Moves the look at position so that the players are centered on the screen and the base player
 * is inside the base screen, keeping the previous position when it does not converge.
 */
void CameraPoserParallel::includeBaseAfterMoveLimit() {
    if (mIsFirstCalc) {
        return;
    }

    sead::Vector3f side;
    side.setCross(mLookAtCamera->getAt() - mLookAtCamera->getPos(), sead::Vector3f::ey);
    normalizeOrZero(&side);

    updateLookAtCamera();
    sead::BoundBox2f layoutBox;
    calcLayoutBoxIncludeAllPlayer(&layoutBox, nullptr);
    f32 center = layoutBox.getCenter().x;
    sead::Vector3f savedLookAtPos = mLookAtPos;

    if (center > 0.1f) {
        f32 step = 100.0f;
        s32 i = 0;

        while (center > 0.1f) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos += step * side;
            updateLookAtCamera();

            if (i++ >= 60) {
                mLookAtPos = savedLookAtPos;
                break;
            }

            sead::BoundBox2f box;
            calcLayoutBoxIncludeAllPlayer(&box, nullptr);
            f32 newCenter = box.getCenter().x;

            if (newCenter < -0.1f) {
                step *= 0.5f;
                mLookAtPos = prevLookAtPos;
            } else {
                center = newCenter;
            }
        }
    }

    if (center < -0.1f) {
        f32 step = 100.0f;
        s32 i = 0;

        while (center < -0.1f) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos -= step * side;
            updateLookAtCamera();

            if (i++ >= 60) {
                mLookAtPos = savedLookAtPos;
                break;
            }

            sead::BoundBox2f box;
            calcLayoutBoxIncludeAllPlayer(&box, nullptr);
            f32 newCenter = box.getCenter().x;

            if (newCenter > 0.1f) {
                step *= 0.5f;
                mLookAtPos = prevLookAtPos;
            } else {
                center = newCenter;
            }
        }
    }

    updateLookAtCamera();
    calcLayoutBoxIncludeAllPlayer(&layoutBox, nullptr);
    center = layoutBox.getCenter().y;
    savedLookAtPos = mLookAtPos;

    if (center > 0.1f) {
        f32 step = 100.0f;
        s32 i = 0;

        while (center > 0.1f) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos += step * sead::Vector3f::ey;
            updateLookAtCamera();

            if (i++ >= 60) {
                mLookAtPos = savedLookAtPos;
                break;
            }

            sead::BoundBox2f box;
            calcLayoutBoxIncludeAllPlayer(&box, nullptr);
            f32 newCenter = box.getCenter().y;

            if (newCenter < -0.1f) {
                step *= 0.5f;
                mLookAtPos = prevLookAtPos;
            } else {
                center = newCenter;
            }
        }
    }

    if (center < -0.1f) {
        f32 step = 100.0f;
        s32 i = 0;

        while (center < -0.1f) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos -= step * sead::Vector3f::ey;
            updateLookAtCamera();

            if (i++ >= 60) {
                mLookAtPos = savedLookAtPos;
                break;
            }

            sead::BoundBox2f box;
            calcLayoutBoxIncludeAllPlayer(&box, nullptr);
            f32 newCenter = box.getCenter().y;

            if (newCenter > 0.1f) {
                step *= 0.5f;
                mLookAtPos = prevLookAtPos;
            } else {
                center = newCenter;
            }
        }
    }

    s32 playerIndex;

    if (mIsTopPlayerBase) {
        playerIndex = mPlayerWatcher->getTopPlayerIndex();
    } else {
        playerIndex = mBasePlayerIndex;
    }

    if (playerIndex == -1) {
        return;
    }

    savedLookAtPos = mLookAtPos;
    updateLookAtCamera();
    sead::Vector2f layoutPos;
    calcPlayerLayoutPos(&layoutPos, playerIndex);

    f32 step = 100.0f;
    s32 i = 0;

    if (layoutPos.x > mParam.mSettingParam._0) {
        while (layoutPos.x > mParam.mSettingParam._0) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos += step * side;
            updateLookAtCamera();

            if (i++ >= 60) {
                mLookAtPos = savedLookAtPos;
                break;
            }

            sead::Vector2f newLayoutPos;
            calcPlayerLayoutPos(&newLayoutPos, playerIndex);

            if (newLayoutPos.x < mParam.mSettingParam._0 - 0.1f) {
                mLookAtPos = prevLookAtPos;
                step *= 0.5f;
            } else {
                layoutPos.x = newLayoutPos.x;
            }
        }
    }

    if (layoutPos.x < -mParam.mSettingParam._0) {
        while (layoutPos.x < -mParam.mSettingParam._0) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos -= step * side;
            updateLookAtCamera();

            if (i++ >= 60) {
                mLookAtPos = savedLookAtPos;
                break;
            }

            sead::Vector2f newLayoutPos;
            calcPlayerLayoutPos(&newLayoutPos, playerIndex);

            if (newLayoutPos.x > 0.1f - mParam.mSettingParam._0) {
                mLookAtPos = prevLookAtPos;
                step *= 0.5f;
            } else {
                layoutPos.x = newLayoutPos.x;
            }
        }
    }

    savedLookAtPos = mLookAtPos;
    updateLookAtCamera();
    calcPlayerLayoutPos(&layoutPos, playerIndex);

    step = 100.0f;
    i = 0;

    if (layoutPos.y > mParam.mSettingParam._4) {
        while (layoutPos.y > mParam.mSettingParam._4) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos += step * sead::Vector3f::ey;
            updateLookAtCamera();

            if (i++ >= 60) {
                mLookAtPos = savedLookAtPos;
                break;
            }

            sead::Vector2f newLayoutPos;
            calcPlayerLayoutPos(&newLayoutPos, playerIndex);

            if (newLayoutPos.y < mParam.mSettingParam._4 - 0.1f) {
                mLookAtPos = prevLookAtPos;
                step *= 0.5f;
            } else {
                layoutPos.y = newLayoutPos.y;
            }
        }
    }

    if (layoutPos.y < -mParam.mSettingParam._8) {
        while (layoutPos.y < -mParam.mSettingParam._8) {
            sead::Vector3f prevLookAtPos = mLookAtPos;
            mLookAtPos -= step * sead::Vector3f::ey;
            updateLookAtCamera();

            if (i++ >= 60) {
                mLookAtPos = savedLookAtPos;
                break;
            }

            sead::Vector2f newLayoutPos;
            calcPlayerLayoutPos(&newLayoutPos, playerIndex);

            if (newLayoutPos.y > 0.1f - mParam.mSettingParam._8) {
                mLookAtPos = prevLookAtPos;
                step *= 0.5f;
            } else {
                layoutPos.y = newLayoutPos.y;
            }
        }
    }
}

/**
 * Checks whether the camera may approach the players again. Once a player is flagged, the camera
 * waits until every alive player was flagged and some time passed.
 * @return Whether the camera may approach.
 */
bool CameraPoserParallel::checkEnableCameraApproach() {
    if (mIsWaitApproach) {
        for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
            if (mPlayerWatcher->isPlayerAlive(i) && mPlayerWatcher->isPlayerApproach(i)) {
                mIsPlayerApproachChecked[i] = true;
            }
        }

        for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
            if (mPlayerWatcher->isPlayerAlive(i) && !mIsPlayerApproachChecked[i]) {
                return false;
            }
        }

        if (++mApproachWaitFrame >= 60) {
            mIsWaitApproach = false;
            return true;
        }

        return false;
    }

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if (mPlayerWatcher->isPlayerAlive(i) && mPlayerWatcher->isPlayerApproach(i)) {
            return true;
        }
    }

    mIsWaitApproach = true;
    mApproachWaitFrame = 0;

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if (mPlayerWatcher->isPlayerAlive(i)) {
            mIsPlayerApproachChecked[i] = false;
        }
    }

    return false;
}

/**
 * Checks whether all players are inside the layout box of the screen.
 * @return Whether all players are inside.
 */
bool CameraPoserParallel::isAllPlayerInLayoutBox() const {
    sead::BoundBox2f layoutBox;
    bool isOutside = false;
    calcLayoutBoxIncludeAllPlayer(&layoutBox, &isOutside);

    if (isOutside) {
        return false;
    }

    if (!(layoutBox.getSizeX() <= mParam.mSettingParam._0 * 2)) {
        return false;
    }

    return layoutBox.getMax().y < mParam.mSettingParam._4 &&
           layoutBox.getMin().y > -mParam.mSettingParam._8;
}

/**
 * Checks whether the move limit box for multi player mode is not empty.
 * @return Whether a valid camera position exists.
 */
bool CameraPoserParallel::isExistCameraPosWithinMoveLimitForMulti() const {
    sead::Vector3f limitMin;
    sead::Vector3f limitMax;
    calcMoveLimitValueForMulti(&limitMin, &limitMax);

    if (limitMin.x > limitMax.x) {
        return false;
    }

    return !(limitMin.y > limitMax.y);
}

/**
 * Writes the pose of the camera, looking at the look at position from the parameter angles.
 * @param pCamera The camera to write to.
 */
void CameraPoserParallel::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    sead::Vector3f front = sead::Vector3f::ez;
    sead::Vector3f lookAtPos = mLookAtPos;
    f32 angleV = mParam.mAngleV;
    f32 angleH = mAngleHFromZoneToRoot;

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

    pCamera->setPos(mParam.mDistance * front + lookAtPos);
    pCamera->setAt(lookAtPos);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * Reads the camera parameters.
 * @param pIter The camera parameters.
 */
void CameraPoserParallel::loadParam(const ByamlIter* pIter) {
    CameraPoser::loadParam(pIter);
    pIter->tryGetFloatByKey(&mParam.mAngleV, "AngleV");
    pIter->tryGetFloatByKey(&mParam.mAngleH, "AngleH");
    pIter->tryGetFloatByKey(&mParam.mDistanceMin, "DistanceMin");
    pIter->tryGetFloatByKey(&mParam.mDistanceMax, "DistanceMax");
    pIter->tryGetFloatByKey(&mParam.mLookAtOffsetX, "LookAtOffsetX");
    pIter->tryGetFloatByKey(&mParam.mLookAtOffsetY, "LookAtOffsetY");
    pIter->tryGetIntByKey(&mInterpolationFrame, "InterpolationFrame");
    pIter->tryGetBoolByKey(&mIsUseRotateDegreeYMoveLimit, "IsUseRotateDegreeYMoveLimit");

    if (mIsUseRotateDegreeYMoveLimit) {
        pIter->tryGetFloatByKey(&mParam.mMoveLimitRotateDegreeY, "MoveLimitRotateDegreeY");
    }

    ByamlIter limitIter;

    if (pIter->tryGetIterByKey(&limitIter, "MoveLimitMin")) {
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMin.x, "X");
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMin.y, "Y");
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMin.z, "Z");
    }

    if (pIter->tryGetIterByKey(&limitIter, "MoveLimitMax")) {
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMax.x, "X");
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMax.y, "Y");
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMax.z, "Z");
    }

    if (pIter->tryGetIterByKey(&limitIter, "MoveLimitMinForMulti")) {
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMinForMulti.x, "X");
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMinForMulti.y, "Y");
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMinForMulti.z, "Z");
    }

    if (pIter->tryGetIterByKey(&limitIter, "MoveLimitMaxForMulti")) {
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMaxForMulti.x, "X");
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMaxForMulti.y, "Y");
        limitIter.tryGetFloatByKey(&mParam.mMoveLimitMaxForMulti.z, "Z");
    }

    pIter->tryGetBoolByKey(&mIsUseAdvancedSetting, "IsUseAdvancedSetting");

    if (mIsUseAdvancedSetting) {
        SettingParam& rSetting = mParam.mSettingParam;
        pIter->tryGetFloatByKey(&rSetting._0, "LayoutPosMaxX");
        pIter->tryGetFloatByKey(&rSetting._4, "LayoutPosMaxTopY");
        pIter->tryGetFloatByKey(&rSetting._8, "LayoutPosMaxBottomY");
        pIter->tryGetIntByKey(&rSetting._10, "TopPlayerIncludingBaseScreenTime");
        pIter->tryGetIntByKey(&rSetting._14, "BehindPlayerIncludingBaseScreenTime");
        pIter->tryGetFloatByKey(&rSetting._18, "SpeedCompensationH");
        pIter->tryGetFloatByKey(&rSetting._1C, "SpeedCompensationV");
        pIter->tryGetFloatByKey(&rSetting._20, "LayoutPosMaxTopYForSpeedCompensationV");
        pIter->tryGetFloatByKey(&rSetting._24, "LayoutPosMaxBottomYForSpeedCompensationV");
        pIter->tryGetFloatByKey(&rSetting._28, "SpeedCompensationMultiH");
        pIter->tryGetFloatByKey(&rSetting._2C, "SpeedCompensationMultiV");
        pIter->tryGetFloatByKey(&rSetting._30, "TargetPlayerPlayerHeightMin");
        pIter->tryGetBoolByKey(&rSetting._34, "IsUseSingleMode");
        pIter->tryGetBoolByKey(&rSetting._35, "IsAlwaysTopPlayerPrior");
        pIter->tryGetBoolByKey(&rSetting._36, "IsActivateMultiModeAnyAngleV");
        pIter->tryGetBoolByKey(&mIsForceGyroMode, "IsForceGyroMode");
        pIter->tryGetBoolByKey(&mIsUseOffsetAtMultiMode, "IsUseOffsetAtMultiMode");
    }

    pIter->tryGetBoolByKey(&mIsNoNormalInterpole, "IsNoNormalInterpole");
}

/**
 * Creates the parameters with their default values.
 */
CameraPoserParallelParam::CameraPoserParallelParam() = default;

/**
 * Calculates the center of all alive players.
 * @param pOut The center position.
 */
void CameraPoserParallel::calcPlayerWorldCenterPos(sead::Vector3f* pOut) const {
    sead::Vector3f sum = sead::Vector3f::zero;
    s32 aliveNum = 0;

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if (mPlayerWatcher->isPlayerAlive(i)) {
            sum += mPlayerWatcher->getPlayerPos(i);
            aliveNum++;
        }
    }

    *pOut = sum * (1.0f / aliveNum);
}

/**
 * Checks whether a player is inside the layout box of the screen.
 * @param index The player index.
 * @return Whether the player is inside.
 */
bool CameraPoserParallel::isPlayerInLayoutBox(s32 index) const {
    sead::Vector2f layoutPos;
    calcPlayerLayoutPos(&layoutPos, index);
    return -mParam.mSettingParam._0 < layoutPos.x && mParam.mSettingParam._0 > layoutPos.x &&
           mParam.mSettingParam._4 > layoutPos.y && layoutPos.y > -mParam.mSettingParam._8;
}

/**
 * Calculates the screen box that includes all players in front of the camera.
 * @param pOut The box to extend.
 * @param pIsBehindCamera Set when a player is behind the camera. Can be null.
 */
void CameraPoserParallel::calcLayoutBoxIncludeAllPlayer(sead::BoundBox2f* pOut,
                                                        bool* pIsBehindCamera) const {
    sead::Vector3f cameraDir = mLookAtCamera->getAt() - mLookAtCamera->getPos();
    bool isIncluded = false;

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        sead::Vector3f lookAtPos;

        if (!mPlayerWatcher->tryGetPlayerLookAtPos(&lookAtPos, i)) {
            continue;
        }

        if (cameraDir.dot(lookAtPos - mLookAtCamera->getPos()) < 0.0f) {
            if (!mPlayerWatcher->isPlayerAlive(i)) {
                mPlayerWatcher->quitPlayerLookAtStop(i);
            }

            if (pIsBehindCamera != nullptr) {
                *pIsBehindCamera = true;
            }

            continue;
        }

        sead::Vector2f layoutPos;
        calcWorldPosToLayoutPos(&layoutPos, lookAtPos);
        pOut->addPoint(layoutPos);
        isIncluded = true;
    }

    if (mPlayerWatcher->isExistAdditionalCameraLookAtPos()) {
        sead::Vector2f layoutPos;
        calcWorldPosToLayoutPos(&layoutPos, mPlayerWatcher->getAdditionalCameraLookAtPos());
        pOut->addPoint(layoutPos);
    } else if (!isIncluded) {
        sead::Vector2f layoutPos;
        calcWorldPosToLayoutPos(&layoutPos, mPlayerWatcher->getTopPlayerPos());
        pOut->addPoint(layoutPos);
    }
}

}  // namespace al
