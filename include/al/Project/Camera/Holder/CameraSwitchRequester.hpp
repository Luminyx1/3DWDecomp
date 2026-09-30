#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraSwitchRequestInfo;
class CameraTicket;
struct CameraPoseInfo;

class CameraSwitchRequester {
public:
    CameraSwitchRequester();

    void init(CameraSwitchRequestInfo* pStartInfos, CameraSwitchRequestInfo* pEndInfos);
    void requestStart(CameraTicket* pTicket, s32 interpoleStep);
    void requestEnd(CameraTicket* pTicket, s32 interpoleStep, bool isKeepPose);
    void requestEndWithNextCameraPose(CameraTicket* pTicket, const CameraPoseInfo* pPoseInfo,
                                      s32 interpoleStep);

private:
    CameraSwitchRequestInfo* mStartInfos = nullptr;
    CameraSwitchRequestInfo* mEndInfos = nullptr;
};

}  // namespace al
