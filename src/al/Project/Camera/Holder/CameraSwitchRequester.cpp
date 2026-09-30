#include "Project/Camera/Holder/CameraSwitchRequester.hpp"

#include "Library/Camera/CameraTicket.hpp"
#include "Project/Camera/Holder/CameraSwitchRequestInfo.hpp"

namespace al {

CameraSwitchRequester::CameraSwitchRequester() = default;

void CameraSwitchRequester::init(CameraSwitchRequestInfo* pStartInfos,
                                 CameraSwitchRequestInfo* pEndInfos) {
    mStartInfos = pStartInfos;
    mEndInfos = pEndInfos;
}

void CameraSwitchRequester::requestStart(CameraTicket* pTicket, s32 interpoleStep) {
    pTicket->setActiveCamera(true);
    mStartInfos[pTicket->getPriority()].addRequest(pTicket, interpoleStep, false);
}

void CameraSwitchRequester::requestEnd(CameraTicket* pTicket, s32 interpoleStep,
                                       bool isKeepPose) {
    pTicket->setActiveCamera(false);
    if (!mStartInfos[pTicket->getPriority()].tryRemoveRequestIfExist(pTicket)) {
        mEndInfos[pTicket->getPriority()].addRequest(pTicket, interpoleStep, isKeepPose);
    }
}

void CameraSwitchRequester::requestEndWithNextCameraPose(CameraTicket* pTicket,
                                                         const CameraPoseInfo* pPoseInfo,
                                                         s32 interpoleStep) {
    pTicket->setActiveCamera(false);
    if (!mStartInfos[pTicket->getPriority()].tryRemoveRequestIfExist(pTicket)) {
        mEndInfos[pTicket->getPriority()].addRequestWithNextCameraPose(pTicket, pPoseInfo,
                                                                       interpoleStep);
    }
}

}  // namespace al
