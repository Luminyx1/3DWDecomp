#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraTicket;
struct CameraPoseInfo;

/// Collects the camera start or end requests of one priority during a frame.
class CameraSwitchRequestInfo {
public:
    CameraSwitchRequestInfo();

    void addRequest(CameraTicket* pTicket, s32 interpoleFrame, bool isInterpoleByCameraDistance);
    void addRequestWithNextCameraPose(CameraTicket* pTicket, const CameraPoseInfo* pNextPose, s32 interpoleFrame);
    bool tryRemoveRequestIfExist(CameraTicket* pTicket);
    void reset();

    CameraTicket** mRequests;               // _0
    s32 mNumRequests = 0;                   // _8
    s32 mInterpoleFrame = -1;               // _C
    bool mIsInterpoleByCameraDistance = false;  // _10
    bool mHasNextCameraPose = false;        // _11
    CameraPoseInfo* mNextCameraPose;        // _18
};
}  // namespace al
