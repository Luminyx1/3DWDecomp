#pragma once

#include "Project/Camera/Core/IUseCameraDirector.hpp"
#include "Project/Camera/Main/IUseCameraDirector_RS.hpp"
#include <math/seadMatrix.h>

namespace al { class ActorInitInfo; }
class GameDataHolder;

class DemoSceneCamera : public al::IUseCamera, public al::IUseCamera_RS {
public:
    explicit DemoSceneCamera(const GameDataHolder* pHolder);
    al::SceneCameraInfo* getSceneCameraInfo() const override;
    al::CameraDirector_RS* getCameraDirector_RS() const override;
    virtual void initDemoSceneActor(const al::ActorInitInfo& rInfo, const al::ActorInitInfo& rCameraInfo, const sead::Matrix34f* pMtx);
    virtual void startAction(int index);
    virtual bool tryStartActionByName(const char* pName);
    virtual void startDemo(const sead::Matrix34f& rMtx);
    virtual void endDemo();
    void setInterpolateFrame(int frames);
    void startCamera(int index);
    bool isEndAnimCamera(int index) const;

private:
    unsigned char _10[0xd8];
};
