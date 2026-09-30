#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraTicket;
struct CameraPoseInfo;

class CameraSwitchRequestInfo {
public:
    CameraSwitchRequestInfo();

    void addRequest(CameraTicket* pTicket, s32 interpoleStep, bool isKeepPose);
    void addRequestWithNextCameraPose(CameraTicket* pTicket, const CameraPoseInfo* pPoseInfo,
                                      s32 interpoleStep);
    bool tryRemoveRequestIfExist(CameraTicket* pTicket);
    void reset();

    CameraTicket* getRequest(s32 index) const { return mRequests[index]; }

    s32 getRequestNum() const { return mRequestNum; }

private:
    CameraTicket** mRequests;
    s32 mRequestNum = 0;
    s32 mInterpoleStep = -1;
    bool mIsKeepPose = false;
    bool mIsSetNextPoseInfo = false;
    CameraPoseInfo* mNextPoseInfo;
};

static_assert(sizeof(CameraSwitchRequestInfo) == 0x20);

}  // namespace al
