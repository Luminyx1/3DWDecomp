#include "Project/Camera/Holder/CameraSwitchRequestInfo.hpp"

#include "Project/Camera/Holder/CameraPoseInfo.hpp"

namespace al {
/** @brief Creates an empty request list with room for four requests. */
CameraSwitchRequestInfo::CameraSwitchRequestInfo() {
    mRequests = new CameraTicket*[4];
    for (s32 i = 0; i < 4; i++) {
        mRequests[i] = nullptr;
    }
    mNextCameraPose = new CameraPoseInfo();
}

/**
 * @brief Adds a request for a ticket.
 * @param pTicket The ticket of the camera.
 * @param interpoleFrame The number of frames to interpolate the switch over.
 * @param isInterpoleByCameraDistance Whether the interpolation length depends on the camera distance.
 */
void CameraSwitchRequestInfo::addRequest(CameraTicket* pTicket, s32 interpoleFrame, bool isInterpoleByCameraDistance) {
    mRequests[mNumRequests] = pTicket;
    mIsInterpoleByCameraDistance = isInterpoleByCameraDistance;
    mHasNextCameraPose = false;
    mNumRequests++;
    mInterpoleFrame = interpoleFrame;
}

/**
 * @brief Adds a request for a ticket along with the pose the camera should continue from.
 * @param pTicket The ticket of the camera.
 * @param pNextPose The pose of the next camera.
 * @param interpoleFrame The number of frames to interpolate the switch over.
 */
void CameraSwitchRequestInfo::addRequestWithNextCameraPose(CameraTicket* pTicket, const CameraPoseInfo* pNextPose,
                                                           s32 interpoleFrame) {
    mRequests[mNumRequests] = pTicket;
    mInterpoleFrame = interpoleFrame;
    mIsInterpoleByCameraDistance = false;
    mHasNextCameraPose = true;
    CameraPoseInfo* nextPose = mNextCameraPose;
    nextPose->mPos.set(pNextPose->mPos);
    nextPose->mAt.set(pNextPose->mAt);
    nextPose->mUp.set(pNextPose->mUp);
    mNumRequests++;
}

/**
 * @brief Removes the request of a ticket if there is one, keeping the order of the others.
 * @param pTicket The ticket to remove.
 * @return True if a request was removed.
 */
bool CameraSwitchRequestInfo::tryRemoveRequestIfExist(CameraTicket* pTicket) {
    bool isFound = false;
    for (s32 i = 0; i < mNumRequests; i++) {
        if (isFound) {
            mRequests[i - 1] = mRequests[i];
        }
        else {
            isFound = mRequests[i] == pTicket;
        }
    }

    if (!isFound) {
        return false;
    }

    mRequests[mNumRequests - 1] = nullptr;
    mNumRequests--;
    return true;
}

/** @brief Clears all requests. */
void CameraSwitchRequestInfo::reset() {
    for (s32 i = 0; i < mNumRequests; i++) {
        mRequests[i] = nullptr;
    }
    mNumRequests = 0;
    mInterpoleFrame = -1;
    mIsInterpoleByCameraDistance = false;
    mHasNextCameraPose = false;
}
}  // namespace al
