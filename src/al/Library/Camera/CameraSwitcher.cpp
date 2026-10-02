#include "Library/Camera/CameraSwitcher.hpp"

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>

#include "Library/Camera/CameraUtil.hpp"
#include "Library/Camera/PlayerWatcher.hpp"
#include "Library/Play/Camera/CameraPoserRail.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Camera/CameraPoser.hpp"
#include "Project/Camera/Info/SceneCameraInfo.hpp"
#include "Project/Camera/Param/CameraHolder.hpp"
#include "Project/Camera/Param/CameraInfo.hpp"

namespace al {

/**
 * Creates the switcher with an empty camera pool.
 * @param ppActivePoser Where the active camera poser is stored.
 * @param pHolder The holder of all cameras by placement id.
 * @param pPlayerWatcher The player watcher.
 * @param pIsResetPoser Whether a newly started poser starts from scratch instead of taking over
 * the state of the previous one.
 */
CameraSwitcher::CameraSwitcher(CameraPoser** ppActivePoser, const CameraHolder* pHolder,
                               PlayerWatcher* pPlayerWatcher, bool* pIsResetPoser)
    : mHolder(pHolder), mPlayerWatcher(pPlayerWatcher), mActivePoser(ppActivePoser),
      mCurrentPoser(ppActivePoser), mIsResetPoser(pIsResetPoser) {
    mPoolCameraIds = new const PlacementId*[cPoolCameraNum];

    for (s32 i = 0; i < cPoolCameraNum; i++) {
        mPoolCameraIds[i] = nullptr;
    }
}

/**
 * Sets the interpolation frame for the camera change.
 * @param interpoleFrame The interpolation frame, or -1 to use the one of the active poser.
 */
inline void CameraSwitcher::setInterpoleFrame(s32 interpoleFrame) {
    mInterpoleFrame =
        interpoleFrame == -1 ? (*mActivePoser)->mInterpolationFrame : interpoleFrame;
}

/**
 * Starts a camera, or puts it into the pool if a camera with higher priority is active.
 * @param rId Placement id of the camera to start.
 * @param interpoleFrame The interpolation frame, or -1 to use the one of the camera.
 */
void CameraSwitcher::start(const PlacementId& rId, s32 interpoleFrame) {
    if ((*mCurrentPoser)->mPlacementId != nullptr &&
        mHolder->getCameraInfoById((*mCurrentPoser)->mPlacementId) != nullptr) {
        if (mHolder->getCameraInfoById((*mCurrentPoser)->mPlacementId)->mPriority >
            mHolder->getCameraInfoById(&rId)->mPriority) {
            addPoolCameraList(rId);
            return;
        }

        if (mHolder->getCameraInfoById((*mCurrentPoser)->mPlacementId)->mPriority <
            mHolder->getCameraInfoById(&rId)->mPriority) {
            addPoolCameraList(*(*mCurrentPoser)->mPlacementId);
        }
    }

    CameraPoser* poser = mHolder->getCameraById(&rId);
    mIsEndFollowCamera = false;
    mIsStartFollowCamera = false;

    if (isEqualString("Follow", poser->mName)) {
        mIsStartFollowCamera = true;
    } else if (isEqualString("Follow", (*mCurrentPoser)->mName)) {
        mIsEndFollowCamera = true;
    }

    if (poser == nullptr || poser == *mCurrentPoser) {
        return;
    }

    sead::Vector3f lookAtPos = (*mActivePoser)->mLookAtPos;
    f32 distance = (*mActivePoser)->getDistance();
    sead::Vector3f railOffset = (*mActivePoser)->getCameraRailOffset();
    f32 speedCompensationV = (*mActivePoser)->getSpeedCompensationV();
    s32 prevValue9C = (*mActivePoser)->_9C;
    f32 railCoord = 0.0f;

    if (isEqualString("Rail", (*mActivePoser)->mName)) {
        CameraPoserRail* railPoser = static_cast<CameraPoserRail*>(*mActivePoser);

        if (!railPoser->isReverseCoord()) {
            railCoord = railPoser->getRailCoord();
        }
    }

    *mActivePoser = poser;
    *mCurrentPoser = poser;

    if (*mIsResetPoser) {
        (*mActivePoser)->setFirstCalcFlag(true);
    } else {
        if (isEqualString("Parallel", (*mActivePoser)->mName)) {
            (*mActivePoser)->setLookAtPos(lookAtPos);
            (*mActivePoser)->setCameraRailOffset(railOffset);
            (*mActivePoser)->setSpeedCompensationV(speedCompensationV);
        }

        (*mActivePoser)->setDistance(distance);
        (*mActivePoser)->setFirstCalcFlag(false);
    }

    if (isEqualString("Rail", (*mActivePoser)->mName) && railCoord != 0.0f) {
        CameraPoserRail* railPoser = static_cast<CameraPoserRail*>(*mActivePoser);

        if (!railPoser->isReverseCoord()) {
            railPoser->setRailCoord(railCoord);
        }
    }

    (*mActivePoser)->_9C = prevValue9C;
    setInterpoleFrame(interpoleFrame);
    (*mActivePoser)->start();
    mIsChanged = true;
}

/**
 * Puts a camera into the pool, replacing a pooled camera with the same priority.
 * @param rId Placement id of the camera.
 */
void CameraSwitcher::addPoolCameraList(const PlacementId& rId) {
    s32 priority = mHolder->getCameraInfoById(&rId)->mPriority;

    for (s32 i = 0; i < cPoolCameraNum; i++) {
        if (mPoolCameraIds[i] != nullptr &&
            priority == mHolder->getCameraInfoById(mPoolCameraIds[i])->mPriority) {
            removePoolCameraList(*mPoolCameraIds[i]);
        }
    }

    for (s32 i = 0; i < cPoolCameraNum; i++) {
        if (mPoolCameraIds[i] == nullptr) {
            mPoolCameraIds[i] = &rId;
            return;
        }
    }
}

/**
 * Ends a camera and starts the pooled camera with the highest priority.
 * @param rId Placement id of the camera to end.
 * @param interpoleFrame The interpolation frame, or -1 to use the one of the next camera.
 */
void CameraSwitcher::end(const PlacementId& rId, s32 interpoleFrame) {
    removePoolCameraList(rId);

    if (tryStartCameraFromPoolCameraList()) {
        setInterpoleFrame(interpoleFrame);
    }
}

/**
 * Removes a camera from the pool.
 * @param rId Placement id of the camera.
 */
void CameraSwitcher::removePoolCameraList(const PlacementId& rId) {
    if (rId.mPlacementID == nullptr) {
        return;
    }

    for (s32 i = 0; i < cPoolCameraNum; i++) {
        if (mPoolCameraIds[i] != nullptr && isEqualPlacementID(*mPoolCameraIds[i], rId)) {
            mPoolCameraIds[i] = nullptr;
            return;
        }
    }
}

/**
 * Tries to start the pooled camera with the highest priority.
 * @return Whether a pooled camera was started.
 */
bool CameraSwitcher::tryStartCameraFromPoolCameraList() {
    CameraPoser* nextPoser = nullptr;
    s32 maxPriority = -1;

    for (s32 i = 0; i < cPoolCameraNum; i++) {
        if (mPoolCameraIds[i] != nullptr &&
            maxPriority < mHolder->getCameraInfoById(mPoolCameraIds[i])->mPriority) {
            maxPriority = mHolder->getCameraInfoById(mPoolCameraIds[i])->mPriority;
            nextPoser = mHolder->getCameraById(mPoolCameraIds[i]);
        }
    }

    if (nextPoser == nullptr) {
        return false;
    }

    sead::Vector3f lookAtPos = (*mActivePoser)->mLookAtPos;
    sead::Vector3f railOffset = (*mActivePoser)->getCameraRailOffset();
    s32 prevValue9C = (*mActivePoser)->_9C;
    *mActivePoser = nextPoser;
    *mCurrentPoser = nextPoser;

    if (!*mIsResetPoser && isEqualString("Parallel", (*mActivePoser)->mName)) {
        (*mActivePoser)->setLookAtPos(lookAtPos);
        (*mActivePoser)->setCameraRailOffset(railOffset);
    }

    (*mActivePoser)->_9C = prevValue9C;
    mInterpoleFrame = (*mActivePoser)->mInterpolationFrame;
    (*mActivePoser)->start();
    (*mActivePoser)->update();
    mIsChanged = true;
    return true;
}

/**
 * Checks whether a camera poser is the current one.
 * @param pPoser The camera poser.
 * @return Whether the poser has the placement id of the current poser.
 */
bool CameraSwitcher::isCameraCurrent(const CameraPoser* pPoser) const {
    return isEqualPlacementID(*(*mCurrentPoser)->mPlacementId, *pPoser->mPlacementId);
}

/**
 * Stops keeping the look-at positions of all players.
 */
void CameraSwitcher::offLookAtStop() {
    mPlayerWatcher->offLookAtStop();
}

}  // namespace al
