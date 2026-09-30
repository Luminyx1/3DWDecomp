#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraSwitchRequestInfo;
class CameraTicket;
struct CameraPoseInfo;

class CameraSwitcher_RS {
public:
    CameraSwitcher_RS();

    void init(CameraSwitchRequestInfo* pStartInfo, CameraSwitchRequestInfo* pEndInfo);
    void initAfterPlacement();
    void update();
    bool isExistNextCamera() const;
    CameraTicket* getNextCamera() const;
    s32 getNextInterpoleStep() const;
    bool isNextKeepPose() const;
    bool isSetNextPoseInfo() const;
    const CameraPoseInfo* getNextPoseInfo() const;

    bool isChanged() const { return mIsChanged; }

private:
    void* _0 = nullptr;
    void* _8 = nullptr;
    bool mIsChanged = false;
    void* _18 = nullptr;
};

}  // namespace al
