#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraSwitchRequestInfo;
class CameraTicket;
struct CameraPoseInfo;

/// Turns camera start and end calls into switch requests per priority.
class CameraSwitchRequester {
public:
    CameraSwitchRequester();

    void init(CameraSwitchRequestInfo* pStartInfos, CameraSwitchRequestInfo* pEndInfos);
    void requestStart(CameraTicket* pTicket, s32 interpoleFrame);
    void requestEnd(CameraTicket* pTicket, s32 interpoleFrame, bool isInterpoleByCameraDistance);
    void requestEndWithNextCameraPose(CameraTicket* pTicket, const CameraPoseInfo* pNextPose, s32 interpoleFrame);

    CameraSwitchRequestInfo* mStartInfos = nullptr;  // _0
    CameraSwitchRequestInfo* mEndInfos = nullptr;    // _8
};
}  // namespace al
