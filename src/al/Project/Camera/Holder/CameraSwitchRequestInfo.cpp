#include "Project/Camera/Holder/CameraSwitchRequestInfo.hpp"

#include "Library/Camera/CameraPoseInfo.hpp"

namespace al {

CameraSwitchRequestInfo::CameraSwitchRequestInfo() {
    mRequests = new CameraTicket*[4];
    for (s32 i = 0; i < 4; i++) {
        mRequests[i] = nullptr;
    }

    mNextPoseInfo = new CameraPoseInfo();
}

void CameraSwitchRequestInfo::addRequest(CameraTicket* pTicket, s32 interpoleStep,
                                         bool isKeepPose) {
    mRequests[mRequestNum] = pTicket;
    mIsKeepPose = isKeepPose;
    mIsSetNextPoseInfo = false;
    mRequestNum++;
    mInterpoleStep = interpoleStep;
}

void CameraSwitchRequestInfo::addRequestWithNextCameraPose(CameraTicket* pTicket,
                                                           const CameraPoseInfo* pPoseInfo,
                                                           s32 interpoleStep) {
    mRequests[mRequestNum] = pTicket;
    mInterpoleStep = interpoleStep;
    mIsKeepPose = false;
    mIsSetNextPoseInfo = true;
    CameraPoseInfo* poseInfo = mNextPoseInfo;
    poseInfo->pos.set(pPoseInfo->pos);
    poseInfo->at.set(pPoseInfo->at);
    poseInfo->up.set(pPoseInfo->up);
    mRequestNum++;
}

bool CameraSwitchRequestInfo::tryRemoveRequestIfExist(CameraTicket* pTicket) {
    bool isFound = false;
    for (s32 i = 0; i < mRequestNum; i++) {
        if (isFound) {
            mRequests[i - 1] = mRequests[i];
        } else {
            isFound = mRequests[i] == pTicket;
        }
    }

    if (!isFound) {
        return false;
    }

    mRequests[mRequestNum - 1] = nullptr;
    mRequestNum--;
    return true;
}

void CameraSwitchRequestInfo::reset() {
    for (s32 i = 0; i < mRequestNum; i++) {
        mRequests[i] = nullptr;
    }

    mRequestNum = 0;
    mInterpoleStep = -1;
    mIsKeepPose = false;
    mIsSetNextPoseInfo = false;
}

}  // namespace al
