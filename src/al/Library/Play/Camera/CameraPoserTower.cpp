#include "Library/Play/Camera/CameraPoserTower.hpp"

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrix.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Obj/PlayerWatcher.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Camera/Param/CameraFunction.hpp"

namespace al {
/**
 * Creates the camera, keeping its own copy of the placement id.
 * @param rParam The default camera parameters.
 * @param pProjection The projection used to place the players on the screen.
 * @param pPlayerWatcher The watcher of the players.
 * @param pPlacementId The placement id of the camera.
 */
CameraPoserTower::CameraPoserTower(const CameraPoserTowerParam& rParam,
                                   const sead::PerspectiveProjection* pProjection,
                                   const PlayerWatcher* pPlayerWatcher,
                                   const PlacementId* pPlacementId)
    : mParam(rParam), mProjection(pProjection), mPlayerWatcher(pPlayerWatcher) {
    mLookAtCamera = new sead::LookAtCamera;
    mName = "Tower";
    mPlacementId = new PlacementId(*pPlacementId);
}

/**
 * Places the look at position around the tower, following one or several players.
 */
void CameraPoserTower::update() {
    if (mPlayerWatcher->getAlivePlayerNum() == 0) {
        return;
    }

    CameraFunction::calcPosFromZoneToRoot(&mAxisPosRoot, mParam.axisPos, mZoneMtx);

    if (mPlayerWatcher->getAlivePlayerNum() == 1) {
        updateSingleCamera();
    } else {
        updateMultiCamera();
    }
}

/**
 * Looks at the top player from the closest distance.
 */
void CameraPoserTower::updateSingleCamera() {
    mParam.distance = mParam.distanceMin;
    mLookAtPos = mPlayerWatcher->getTopPlayerPos() + mParam.upOffset * sead::Vector3f::ey;
    updateLookAtCamera();
    limitMoveDegree();
    applyMoveLimit(mParam.moveLimitMinY, mParam.moveLimitMaxY);
    mPrevLookAtPos = mLookAtPos;
}

/**
 * Places the look at position and distance so that all players stay on the screen, preferring
 * the top player or the player that is the farthest outside of the base screen.
 */
void CameraPoserTower::updateMultiCamera() {
    f32 sumY = 0.0f;
    s32 aliveNum = 0;

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if (mPlayerWatcher->isPlayerAlive(i)) {
            sead::Vector3f lookAtPos;
            mPlayerWatcher->getPlayerLookAtPos(&lookAtPos, i);
            sumY += lookAtPos.y;
            aliveNum++;
        }
    }

    mLookAtPos.y = sumY / aliveNum;
    calcLookAtPosToScreenCenterX();
    calcLookAtPosToScreenCenterY();
    calcCameraDistance();

    s32 farthestIndex = -1;

    if (mParam.isAlwaysTopPlayerPrior) {
        mIsTopPlayerPrior = true;
    } else {
        sead::Vector2f topVec = sead::Vector2f::zero;

        if (mPlayerWatcher->getTopPlayerVelocity().length() > 0.5f) {
            calcPlayerVecOutsideBaseScreen(&topVec, mPlayerWatcher->getTopPlayerIndex());
        }

        sead::Vector2f sumVec = sead::Vector2f::zero;
        f32 maxLength = 0.0f;

        for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
            if (!mPlayerWatcher->isPlayerAlive(i) || i == mPlayerWatcher->getTopPlayerIndex()) {
                continue;
            }

            if (mPlayerWatcher->getPlayerVelocity(i).length() < 0.5f) {
                continue;
            }

            sead::Vector2f vec = sead::Vector2f::zero;
            calcPlayerVecOutsideBaseScreen(&vec, i);
            sumVec += vec;

            if (maxLength < vec.length()) {
                maxLength = vec.length();
                farthestIndex = i;
            }
        }

        if (mPriorPlayerIndex != -1 && !mPlayerWatcher->isPlayerAlive(mPriorPlayerIndex)) {
            mPriorPlayerIndex = -1;
        }

        f32 diff = topVec.length() * mParam.topPlayerPriorRate - sumVec.length();

        if (mIsTopPlayerPrior) {
            if (diff < 0.0f ||
                !mPlayerWatcher->isPlayerAlive(mPlayerWatcher->getTopPlayerIndex())) {
                mIsTopPlayerPrior = false;
            }
        }

        if (!mIsTopPlayerPrior && diff > 0.0f) {
            mIsTopPlayerPrior = true;
        }
    }

    if (!mIsDistanceMax) {
        mTopPlayerFrame = 0;
        mPriorPlayerFrame = 0;
    } else if (mIsTopPlayerPrior) {
        mPriorPlayerFrame = 0;
        sead::Vector3f prevLookAtPos = mLookAtPos;
        calcLookAtPosToPutPlayerInBaseScreenX(mPlayerWatcher->getTopPlayerIndex());
        calcLookAtPosToPutPlayerInBaseScreenY(mPlayerWatcher->getTopPlayerIndex());
        f32 rate = mTopPlayerFrame / mParam.topPlayerIncludingBaseScreenTime;
        sead::Vector3f newLookAtPos = mLookAtPos;
        rate = easeInOut(sead::Mathf::clamp(rate, 0.0f, 1.0f));
        mLookAtPos = newLookAtPos * rate + prevLookAtPos * (1.0f - rate);
        mTopPlayerFrame++;
    } else {
        mTopPlayerFrame = 0;
        sead::Vector3f prevLookAtPos = mLookAtPos;

        if (mPriorPlayerIndex == -1) {
            mPriorPlayerIndex = farthestIndex;
        } else if (farthestIndex != -1) {
            sead::Vector2f priorVec = sead::Vector2f::zero;
            calcPlayerVecOutsideBaseScreen(&priorVec, mPriorPlayerIndex);
            sead::Vector2f farthestVec = sead::Vector2f::zero;
            calcPlayerVecOutsideBaseScreen(&farthestVec, farthestIndex);

            if (priorVec.length() + 0.5f < farthestVec.length()) {
                mPriorPlayerIndex = farthestIndex;
            }
        }

        if (mPriorPlayerIndex == -1) {
            mPriorPlayerFrame = 0;
        } else {
            calcLookAtPosToPutPlayerInBaseScreenX(mPriorPlayerIndex);
            calcLookAtPosToPutPlayerInBaseScreenY(mPriorPlayerIndex);
            f32 rate = mPriorPlayerFrame / mParam.behindPlayerIncludingBaseScreenTime;
            sead::Vector3f newLookAtPos = mLookAtPos;
            rate = easeInOut(sead::Mathf::clamp(rate, 0.0f, 1.0f));
            mLookAtPos = newLookAtPos * rate + prevLookAtPos * (1.0f - rate);
            mPriorPlayerFrame++;
        }
    }

    limitMoveDegree();
    f32 minY;
    f32 maxY;
    calcMoveLimitValueForMulti(&minY, &maxY);
    applyMoveLimit(minY, maxY);
    mPrevLookAtPos = mLookAtPos;
}

/**
 * Looks at the center of all players.
 */
void CameraPoserTower::start() {
    if (mPlayerWatcher->getAlivePlayerNum() == 0) {
        return;
    }

    calcPlayerWorldCenterPos(&mLookAtPos);
    mPrevLookAtPos = mLookAtPos;
    updateLookAtCamera();
}

/**
 * Calculates the average position of all alive players.
 * @param pOut The center position.
 */
void CameraPoserTower::calcPlayerWorldCenterPos(sead::Vector3f* pOut) {
    sead::Vector3f sum = sead::Vector3f::zero;
    s32 aliveNum = 0;

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if (mPlayerWatcher->isPlayerAlive(i)) {
            sum += mPlayerWatcher->getPlayerPos(i);
            aliveNum++;
        }
    }

    pOut->set(sum * (1.0f / aliveNum));
}

/**
 * Writes the current pose to the internal camera and updates its view matrix.
 */
void CameraPoserTower::updateLookAtCamera() {
    makeLookAtCamera(mLookAtCamera);
    mLookAtCamera->updateViewMatrix();
}

/**
 * Writes the pose of the camera, looking at the look at position from outside the tower axis.
 * @param pCamera The camera to write to.
 */
void CameraPoserTower::makeLookAtCamera(sead::LookAtCamera* pCamera) const {
    sead::Vector3f dir = mLookAtPos - mAxisPosRoot;
    verticalizeVec(&dir, sead::Vector3f::ey, dir);
    normalizeOrZero(&dir);

    f32 dirX = dir.x;
    f32 dirZ = dir.z;
    f32 horizontal = sead::Mathf::cos(sead::Mathf::deg2rad(mParam.angleV)) * mParam.distance;
    f32 offsetX = dirX * horizontal;
    f32 offsetZ = dirZ * horizontal;
    f32 offsetY = sead::Mathf::sin(sead::Mathf::deg2rad(mParam.angleV)) * mParam.distance;
    sead::Vector3f offset = {offsetX, offsetY, offsetZ};

    pCamera->setPos(offset + mLookAtPos);
    pCamera->setAt(mLookAtPos);
    pCamera->setUp(mCameraUp);
    pCamera->normalizeUp();
}

/**
 * Reads the tower axis, distances, angles, screen layout limits and move limits.
 * @param pIter The camera parameters.
 */
void CameraPoserTower::loadParam(const ByamlIter* pIter) {
    CameraPoser::loadParam(pIter);

    ByamlIter axisIter;

    if (pIter->tryGetIterByKey(&axisIter, "AxisPos")) {
        axisIter.tryGetFloatByKey(&mParam.axisPos.x, "X");
        axisIter.tryGetFloatByKey(&mParam.axisPos.y, "Y");
        axisIter.tryGetFloatByKey(&mParam.axisPos.z, "Z");
    }

    pIter->tryGetFloatByKey(&mParam.distanceMin, "DistanceMin");
    pIter->tryGetFloatByKey(&mParam.distanceMax, "DistanceMax");
    pIter->tryGetFloatByKey(&mParam.angleV, "AngleV");
    pIter->tryGetFloatByKey(&mParam.upOffset, "UpOffset");
    pIter->tryGetIntByKey(&mInterpolationFrame, "InterpolationFrame");
    pIter->tryGetFloatByKey(&mParam.layoutPosMaxX, "LayoutPosMaxX");
    pIter->tryGetFloatByKey(&mParam.layoutPosMaxTopY, "LayoutPosMaxTopY");
    pIter->tryGetFloatByKey(&mParam.layoutPosMaxBottomY, "LayoutPosMaxBottomY");
    pIter->tryGetFloatByKey(&mParam.topPlayerIncludingBaseScreenTime,
                            "TopPlayerIncludingBaseScreenTime");
    pIter->tryGetFloatByKey(&mParam.behindPlayerIncludingBaseScreenTime,
                            "BehindPlayerIncludingBaseScreenTime");
    pIter->tryGetBoolByKey(&mParam.isAlwaysTopPlayerPrior, "IsAlwaysTopPlayerPrior");
    pIter->tryGetFloatByKey(&mParam.moveLimitMinY, "MoveLimitMinY");
    pIter->tryGetFloatByKey(&mParam.moveLimitMaxY, "MoveLimitMaxY");
    pIter->tryGetFloatByKey(&mParam.moveLimitMultiMinY, "MoveLimitMultiMinY");
    pIter->tryGetFloatByKey(&mParam.moveLimitMultiMaxY, "MoveLimitMultiMaxY");
}

/**
 * Limits the horizontal rotation of the look at position around the tower axis to one degree
 * per frame.
 */
void CameraPoserTower::limitMoveDegree() {
    f32 lookAtY = mLookAtPos.y;
    sead::Vector3f dir = mLookAtPos - mAxisPosRoot;
    verticalizeVec(&dir, sead::Vector3f::ey, dir);
    f32 distance = dir.length();
    normalizeOrDirZ(&dir);

    sead::Vector3f prevDir = mPrevLookAtPos - mAxisPosRoot;
    verticalizeVec(&prevDir, sead::Vector3f::ey, prevDir);
    normalizeOrDirZ(&prevDir);

    sead::Vector3f newDir;
    turnVecToVecDegree(&newDir, prevDir, dir, 1.0f);
    mLookAtPos.set(distance * newDir.x + mAxisPosRoot.x, lookAtY,
                   distance * newDir.z + mAxisPosRoot.z);
}

/**
 * Moves the look at position vertically so that the camera height in the zone stays in a range.
 * @param minY The lowest camera height in the zone.
 * @param maxY The highest camera height in the zone.
 */
void CameraPoserTower::applyMoveLimit(f32 minY, f32 maxY) {
    sead::Vector3f dir = mLookAtPos - mAxisPosRoot;
    verticalizeVec(&dir, sead::Vector3f::ey, dir);
    normalizeOrZero(&dir);

    f32 dirX = dir.x;
    f32 dirZ = dir.z;
    f32 horizontal = sead::Mathf::cos(sead::Mathf::deg2rad(mParam.angleV)) * mParam.distance;
    f32 offsetX = dirX * horizontal;
    f32 offsetZ = dirZ * horizontal;
    f32 offsetY = sead::Mathf::sin(sead::Mathf::deg2rad(mParam.angleV)) * mParam.distance;
    sead::Vector3f offset = {offsetX, offsetY, offsetZ};
    sead::Vector3f cameraPos = offset + mLookAtPos;

    sead::Matrix34f invZoneMtx;
    invZoneMtx.setInverse(mZoneMtx);
    sead::Vector3f localPos;
    localPos.setMul(invZoneMtx, cameraPos);
    localPos.y = sead::Mathf::clamp(localPos.y, minY, maxY);
    sead::Vector3f limitedPos;
    limitedPos.setMul(mZoneMtx, localPos);
    mLookAtPos.y += limitedPos.y - cameraPos.y;
}

/**
 * Rotates the look at position around the tower axis until the players are centered
 * horizontally on the screen.
 */
void CameraPoserTower::calcLookAtPosToScreenCenterX() {
    updateLookAtCamera();
    sead::BoundBox2f box;
    calcLayoutBoxIncludeAllPlayer(&box);
    f32 centerX = (box.getMin().x + box.getMax().x) * 0.5f;

    f32 degree = 0.05f;
    s32 count = 0;

    while (centerX > 0.1f) {
        if (count >= 60) {
            calcPlayerWorldCenterPos(&mLookAtPos);
            break;
        }

        count++;
        sead::Vector3f prevLookAtPos = mLookAtPos;
        sead::Vector3f dir = mLookAtPos - mAxisPosRoot;
        rotateVectorDegreeY(&dir, degree);
        mLookAtPos = mAxisPosRoot + dir;
        updateLookAtCamera();
        sead::BoundBox2f newBox;
        calcLayoutBoxIncludeAllPlayer(&newBox);
        f32 newCenterX = (newBox.getMin().x + newBox.getMax().x) * 0.5f;

        if (newCenterX < -0.1f) {
            degree *= 0.5f;
            mLookAtPos = prevLookAtPos;
        } else {
            centerX = newCenterX;
        }
    }

    degree = 0.05f;
    count = 0;

    while (centerX < -0.1f) {
        if (count >= 60) {
            calcPlayerWorldCenterPos(&mLookAtPos);
            break;
        }

        count++;
        sead::Vector3f prevLookAtPos = mLookAtPos;
        sead::Vector3f dir = mLookAtPos - mAxisPosRoot;
        rotateVectorDegreeY(&dir, -degree);
        mLookAtPos = mAxisPosRoot + dir;
        updateLookAtCamera();
        sead::BoundBox2f newBox;
        calcLayoutBoxIncludeAllPlayer(&newBox);
        f32 newCenterX = (newBox.getMin().x + newBox.getMax().x) * 0.5f;

        if (newCenterX > 0.1f) {
            degree *= 0.5f;
            mLookAtPos = prevLookAtPos;
        } else {
            centerX = newCenterX;
        }
    }
}

/**
 * Moves the look at position towards or away from the tower axis until the players are centered
 * vertically on the screen.
 */
void CameraPoserTower::calcLookAtPosToScreenCenterY() {
    updateLookAtCamera();
    sead::BoundBox2f box;
    calcLayoutBoxIncludeAllPlayer(&box);
    f32 centerY = (box.getMin().y + box.getMax().y) * 0.5f;

    f32 step = 5.0f;
    s32 count = 0;

    while (centerY > 0.1f) {
        if (count >= 60) {
            calcPlayerWorldCenterPos(&mLookAtPos);
            break;
        }

        count++;
        sead::Vector3f prevLookAtPos = mLookAtPos;
        sead::Vector3f dir = {mAxisPosRoot.x - mLookAtPos.x, 0.0f, mAxisPosRoot.z - mLookAtPos.z};
        normalizeOrZero(&dir);
        mLookAtPos += step * dir;
        updateLookAtCamera();
        sead::BoundBox2f newBox;
        calcLayoutBoxIncludeAllPlayer(&newBox);
        f32 newCenterY = (newBox.getMin().y + newBox.getMax().y) * 0.5f;

        if (newCenterY < -0.1f) {
            step *= 0.5f;
            mLookAtPos = prevLookAtPos;
        } else {
            centerY = newCenterY;
        }
    }

    step = 5.0f;
    count = 0;

    while (centerY < -0.1f) {
        if (count >= 60) {
            calcPlayerWorldCenterPos(&mLookAtPos);
            break;
        }

        count++;
        sead::Vector3f prevLookAtPos = mLookAtPos;
        sead::Vector3f dir = {mLookAtPos.x - mAxisPosRoot.x, 0.0f, mLookAtPos.z - mAxisPosRoot.z};
        normalizeOrZero(&dir);
        mLookAtPos += step * dir;
        updateLookAtCamera();
        sead::BoundBox2f newBox;
        calcLayoutBoxIncludeAllPlayer(&newBox);
        f32 newCenterY = (newBox.getMin().y + newBox.getMax().y) * 0.5f;

        if (newCenterY > 0.1f) {
            step *= 0.5f;
            mLookAtPos = prevLookAtPos;
        } else {
            centerY = newCenterY;
        }
    }
}

/**
 * Moves the camera away from the closest distance until all players fit in the layout box,
 * keeping a farther previous distance for a while to avoid jitter.
 */
void CameraPoserTower::calcCameraDistance() {
    f32 prevDistance = mParam.distance;
    mParam.distance = mParam.distanceMin;
    f32 distanceMax = mParam.distanceMax;
    updateLookAtCamera();

    while (mParam.distance < mParam.distanceMax) {
        if (isAllPlayerInLayoutBox()) {
            break;
        }

        mParam.distance += 600.0f;
        updateLookAtCamera();

        if (!isExistCameraPosWithinMoveLimitForMulti()) {
            mParam.distance -= 600.0f;
            break;
        }

        if (mParam.distance >= mParam.distanceMax) {
            mParam.distance = mParam.distanceMax;
            break;
        }
    }

    if (prevDistance < mParam.distance) {
        mDistanceKeepFrame = 0;
    } else if (mDistanceKeepFrame < 240) {
        mParam.distance = prevDistance;
        mDistanceKeepFrame++;
    }

    mIsDistanceMax = mParam.distance == distanceMax;
}

/**
 * Calculates the vector on the screen from a player to its look at position, cut to the part
 * that points outside of the base screen.
 * @param pOut The vector on the screen.
 * @param index The player index.
 */
void CameraPoserTower::calcPlayerVecOutsideBaseScreen(sead::Vector2f* pOut, s32 index) {
    sead::Vector2f playerLayoutPos;
    sead::Vector2f lookAtLayoutPos;
    calcWorldPosToLayoutPos(&playerLayoutPos, mPlayerWatcher->getPlayerPos(index));
    sead::Vector3f lookAtPos;
    mPlayerWatcher->getPlayerLookAtPos(&lookAtPos, index);
    calcWorldPosToLayoutPos(&lookAtLayoutPos, lookAtPos);
    *pOut = lookAtLayoutPos - playerLayoutPos;
    calcVecOutsideBaseScreen(lookAtLayoutPos, pOut);
}

/**
 * Moves the look at position towards a player until it is horizontally inside the base screen.
 * @param index The player index.
 */
void CameraPoserTower::calcLookAtPosToPutPlayerInBaseScreenX(s32 index) {
    updateLookAtCamera();
    sead::Vector2f layoutPos;
    calcPlayerLayoutPos(&layoutPos, index);

    f32 rate = 0.5f;
    s32 count = 0;

    while (count <= 60 && layoutPos.x > mParam.layoutPosMaxX) {
        sead::Vector3f prevLookAtPos = mLookAtPos;
        mLookAtPos = rate * mPlayerWatcher->getPlayerPos(index) + (1.0f - rate) * mLookAtPos;
        updateLookAtCamera();
        sead::Vector2f newLayoutPos;
        calcPlayerLayoutPos(&newLayoutPos, index);

        if (newLayoutPos.x < mParam.layoutPosMaxX - 0.1f) {
            mLookAtPos = prevLookAtPos;
            rate *= 0.5f;
        } else {
            layoutPos.x = newLayoutPos.x;
            count++;
        }
    }

    rate = 0.5f;
    count = 0;

    while (count <= 60 && layoutPos.x < -mParam.layoutPosMaxX) {
        sead::Vector3f prevLookAtPos = mLookAtPos;
        mLookAtPos = rate * mPlayerWatcher->getPlayerPos(index) + (1.0f - rate) * mLookAtPos;
        updateLookAtCamera();
        sead::Vector2f newLayoutPos;
        calcPlayerLayoutPos(&newLayoutPos, index);

        if (newLayoutPos.x > 0.1f - mParam.layoutPosMaxX) {
            mLookAtPos = prevLookAtPos;
            rate *= 0.5f;
        } else {
            layoutPos.x = newLayoutPos.x;
            count++;
        }
    }
}

/**
 * Moves the look at position towards a player until it is vertically inside the base screen.
 * @param index The player index.
 */
void CameraPoserTower::calcLookAtPosToPutPlayerInBaseScreenY(s32 index) {
    updateLookAtCamera();
    sead::Vector2f layoutPos;
    calcPlayerLayoutPos(&layoutPos, index);

    f32 rate = 0.5f;
    s32 count = 0;

    while (count <= 60 && layoutPos.y > mParam.layoutPosMaxTopY) {
        sead::Vector3f prevLookAtPos = mLookAtPos;
        mLookAtPos = rate * mPlayerWatcher->getPlayerPos(index) + (1.0f - rate) * mLookAtPos;
        updateLookAtCamera();
        sead::Vector2f newLayoutPos;
        calcPlayerLayoutPos(&newLayoutPos, index);

        if (newLayoutPos.y < mParam.layoutPosMaxTopY - 0.1f) {
            mLookAtPos = prevLookAtPos;
            rate *= 0.5f;
        } else {
            layoutPos.y = newLayoutPos.y;
            count++;
        }
    }

    rate = 0.5f;
    count = 0;

    while (count <= 60 && layoutPos.y < -mParam.layoutPosMaxBottomY) {
        sead::Vector3f prevLookAtPos = mLookAtPos;
        mLookAtPos = rate * mPlayerWatcher->getPlayerPos(index) + (1.0f - rate) * mLookAtPos;
        updateLookAtCamera();
        sead::Vector2f newLayoutPos;
        calcPlayerLayoutPos(&newLayoutPos, index);

        if (newLayoutPos.y > 0.1f - mParam.layoutPosMaxBottomY) {
            mLookAtPos = prevLookAtPos;
            rate *= 0.5f;
        } else {
            layoutPos.y = newLayoutPos.y;
            count++;
        }
    }
}

/**
 * Calculates the range of the look at height that keeps the screen inside the multiplayer move
 * limits.
 * @param pMinY The lowest look at height.
 * @param pMaxY The highest look at height.
 */
void CameraPoserTower::calcMoveLimitValueForMulti(f32* pMinY, f32* pMaxY) const {
    f32 minY = mParam.moveLimitMinY;
    f32 maxY = mParam.moveLimitMaxY;
    *pMinY = minY +
             (mParam.distance - mParam.distanceMin) *
                 sead::Mathf::cos(sead::Mathf::deg2rad(mParam.angleV)) *
                 sead::Mathf::tan(sead::Mathf::deg2rad(mParam.angleV + mFovyDegree * 0.5f));
    *pMaxY = maxY +
             (mParam.distance - mParam.distanceMin) *
                 sead::Mathf::cos(sead::Mathf::deg2rad(mParam.angleV)) *
                 sead::Mathf::tan(sead::Mathf::deg2rad(mParam.angleV - mFovyDegree * 0.5f));
}

/**
 * Calculates the box on the screen that includes all alive players.
 * @param pBox The box on the screen.
 */
void CameraPoserTower::calcLayoutBoxIncludeAllPlayer(sead::BoundBox2f* pBox) {
    sead::Vector2f max = {sead::Mathf::minNumber(), sead::Mathf::minNumber()};
    sead::Vector2f min = {sead::Mathf::maxNumber(), sead::Mathf::maxNumber()};

    for (s32 i = 0; i < mPlayerWatcher->getPlayerNum(); i++) {
        if (!mPlayerWatcher->isPlayerAlive(i)) {
            continue;
        }

        sead::Vector2f layoutPos;
        calcPlayerLayoutPos(&layoutPos, i);

        if (layoutPos.x < min.x) {
            min.x = layoutPos.x;
        }

        if (layoutPos.y < min.y) {
            min.y = layoutPos.y;
        }

        if (layoutPos.x > max.x) {
            max.x = layoutPos.x;
        }

        if (layoutPos.y > max.y) {
            max.y = layoutPos.y;
        }
    }

    pBox->set(min, max);
}

/**
 * Calculates the position of a player on the screen, at the height of its look at position.
 * @param pOut The position on the screen.
 * @param index The player index.
 */
void CameraPoserTower::calcPlayerLayoutPos(sead::Vector2f* pOut, s32 index) const {
    sead::Viewport viewport(0.0f, 0.0f, static_cast<u32>(getDisplayWidth()),
                            static_cast<u32>(getDisplayHeight()));
    sead::Vector3f pos = mPlayerWatcher->getPlayerPos(index);
    sead::Vector3f lookAtPos;
    mPlayerWatcher->getPlayerLookAtPos(&lookAtPos, index);
    pos.y = lookAtPos.y;
    mLookAtCamera->projectByMatrix(pOut, pos, *mProjection, viewport);
}

/**
 * @return Whether all players are inside the layout box on the screen.
 */
bool CameraPoserTower::isAllPlayerInLayoutBox() {
    sead::BoundBox2f box;
    calcLayoutBoxIncludeAllPlayer(&box);

    if (box.getSizeX() <= mParam.layoutPosMaxX * 2) {
        return box.getMax().y < mParam.layoutPosMaxTopY &&
               box.getMin().y > -mParam.layoutPosMaxBottomY;
    }

    return false;
}

/**
 * @return Whether there is a look at height that keeps the screen inside the multiplayer move
 * limits.
 */
bool CameraPoserTower::isExistCameraPosWithinMoveLimitForMulti() const {
    f32 minY;
    f32 maxY;
    calcMoveLimitValueForMulti(&minY, &maxY);
    return !(minY > maxY);
}

/**
 * Projects a world position to the screen.
 * @param pOut The position on the screen.
 * @param pos The world position.
 */
void CameraPoserTower::calcWorldPosToLayoutPos(sead::Vector2f* pOut, sead::Vector3f pos) {
    sead::Viewport viewport(0.0f, 0.0f, static_cast<u32>(getDisplayWidth()),
                            static_cast<u32>(getDisplayHeight()));
    mLookAtCamera->projectByMatrix(pOut, pos, *mProjection, viewport);
}

/**
 * Cuts the components of a vector on the screen that point back towards the screen center.
 * @param rBasePos The position on the screen the vector starts from.
 * @param pVec The vector to cut.
 */
void CameraPoserTower::calcVecOutsideBaseScreen(const sead::Vector2f& rBasePos,
                                                sead::Vector2f* pVec) {
    if (rBasePos.x > 0.0f && pVec->x < 0.0f) {
        pVec->x = 0.0f;
    }

    if (rBasePos.x <= 0.0f && pVec->x > 0.0f) {
        pVec->x = 0.0f;
    }

    if (rBasePos.y > 0.0f && pVec->y < 0.0f) {
        pVec->y = 0.0f;
    }

    if (rBasePos.y <= 0.0f && pVec->y > 0.0f) {
        pVec->y = 0.0f;
    }
}

/**
 * Creates the default tower camera parameters.
 */
CameraPoserTowerParam::CameraPoserTowerParam() = default;
}  // namespace al
