#include "Project/Camera/Holder/CameraSwitchRequester.hpp"

#include "Project/Camera/Holder/CameraSwitchRequestInfo.hpp"
#include "Project/Camera/Holder/CameraTicket.hpp"

namespace al {
/** @brief Creates a requester without request lists. */
CameraSwitchRequester::CameraSwitchRequester() = default;

/**
 * @brief Sets the request lists, one entry per priority.
 * @param pStartInfos The start request lists.
 * @param pEndInfos The end request lists.
 */
void CameraSwitchRequester::init(CameraSwitchRequestInfo* pStartInfos, CameraSwitchRequestInfo* pEndInfos) {
    mStartInfos = pStartInfos;
    mEndInfos = pEndInfos;
}

/**
 * @brief Requests to start the camera of a ticket.
 * @param pTicket The ticket of the camera.
 * @param interpoleFrame The number of frames to interpolate the switch over.
 */
void CameraSwitchRequester::requestStart(CameraTicket* pTicket, s32 interpoleFrame) {
    pTicket->setActiveCamera(true);
    mStartInfos[pTicket->getPriority()].addRequest(pTicket, interpoleFrame, false);
}

/**
 * @brief Requests to end the camera of a ticket, or cancels its pending start request.
 * @param pTicket The ticket of the camera.
 * @param interpoleFrame The number of frames to interpolate the switch over.
 * @param isInterpoleByCameraDistance Whether the interpolation length depends on the camera distance.
 */
void CameraSwitchRequester::requestEnd(CameraTicket* pTicket, s32 interpoleFrame, bool isInterpoleByCameraDistance) {
    pTicket->setActiveCamera(false);
    if (!mStartInfos[pTicket->getPriority()].tryRemoveRequestIfExist(pTicket)) {
        mEndInfos[pTicket->getPriority()].addRequest(pTicket, interpoleFrame, isInterpoleByCameraDistance);
    }
}

/**
 * @brief Requests to end the camera of a ticket and continue from a given pose, or cancels its pending start.
 * @param pTicket The ticket of the camera.
 * @param pNextPose The pose the next camera should start from.
 * @param interpoleFrame The number of frames to interpolate the switch over.
 */
void CameraSwitchRequester::requestEndWithNextCameraPose(CameraTicket* pTicket, const CameraPoseInfo* pNextPose,
                                                         s32 interpoleFrame) {
    pTicket->setActiveCamera(false);
    if (!mStartInfos[pTicket->getPriority()].tryRemoveRequestIfExist(pTicket)) {
        mEndInfos[pTicket->getPriority()].addRequestWithNextCameraPose(pTicket, pNextPose, interpoleFrame);
    }
}
}  // namespace al
