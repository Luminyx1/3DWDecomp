#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadCamera.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.hpp"
#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
class CameraInterpole_RS;
class CameraObjectRequestInfo;
class CameraParamTransfer;
class CameraShaker_RS;
class CameraStartParamCtrl;
class CameraStopJudge;
class CameraSwitcher_RS;
class CameraTicket;
class CameraTurnInfo;
class CameraViewFlag;
class CameraViewInfo;
class ClippingDirectorBase;
struct OrthoProjectionInfo;
class PauseCameraCtrl;
class SceneCameraViewCtrl;
class Projection;
class SceneCameraInfo;

class CameraPoseUpdater : public NerveExecutor, public IUseAreaObj {
public:
    CameraPoseUpdater(SceneCameraInfo* pSceneCameraInfo, s32 viewIndex);
    ~CameraPoseUpdater() override;

    Projection* getProjection();
    void init(const CameraParamTransfer* pParamTransfer, const CameraStopJudge* pStopJudge,
              CameraStartParamCtrl* pStartParamCtrl);
    f32 getNearClipDistance() const;
    void update(bool isPaused);
    bool trySwitchCamera();
    bool isActiveInterpole() const;
    void startInterpole(s32 step);
    void requestCancelInterpole();
    bool calcCameraPoseWithoutInterpole(sead::LookAtCamera* pCamera) const;
    void startSnapShotMode(bool isLock);
    void enableSnapShotRoll(bool isEnable);
    void endSnapShotMode();
    bool isSnapShotOrientationRotate90() const;
    bool isSnapShotOrientationRotate270() const;
    void setDefaultTicket(CameraTicket* pTicket);
    void setClippingDirector(ClippingDirectorBase* pDirector);
    void exeActive();
    void exeDeactive();
    void exeStop();
    void exePause();
    void exeSnapShot();
    void endSnapShot();
    void exeSnapShotNoUpdate();
    bool isCurrentCameraPriority(s32 priority) const;
    bool isInvalidChangeSubjectiveCamera() const;
    bool isCurrentCameraZooming() const;
    bool isCurrentCameraEnableRotateByPad() const;
    bool isCurrentCameraDisasterOn() const;
    bool tryReceiveCameraRequestFromObject(const CameraObjectRequestInfo& rInfo);
    bool tryRequestCameraTurnToDirection(const CameraTurnInfo* pInfo);

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

    void setNearClipDistance(f32 distance) { mNearClipDistance = distance; }

    void setFarClipDistance(f32 distance) { mFarClipDistance = distance; }

    CameraShaker_RS* getShaker() const { return mShaker; }

private:
    s32 mSnapShotOrientation = 0;
    SceneCameraInfo* mSceneCameraInfo;
    SceneCameraViewCtrl* mSceneCameraViewCtrl = nullptr;
    CameraViewInfo* mViewInfo = nullptr;
    CameraViewFlag* mViewFlag = nullptr;
    bool mIsMainView;
    s32 mViewIndex;
    sead::LookAtCamera mLookAtCamera;
    CameraTicket* mDefaultTicket = nullptr;
    CameraTicket* mTicket = nullptr;
    Projection* mProjection = nullptr;
    OrthoProjectionInfo* mOrthoProjectionInfo = nullptr;
    f32 mNearClipDistance = 10.0f;
    f32 mFarClipDistance = 150000.0f;
    f32 mAspect = 16.0f / 9.0f;
    f32 mFovyDegree = 30.0f;
    CameraSwitcher_RS* mSwitcher = nullptr;
    CameraStartParamCtrl* mStartParamCtrl = nullptr;
    const CameraStopJudge* mStopJudge = nullptr;
    const CameraParamTransfer* mParamTransfer = nullptr;
    PauseCameraCtrl* mPauseCameraCtrl = nullptr;
    CameraInterpole_RS* mInterpole = nullptr;
    CameraShaker_RS* mShaker = nullptr;
    ClippingDirectorBase* mClippingDirector = nullptr;
    AreaObjDirector* mAreaObjDirector = nullptr;
};

static_assert(sizeof(CameraPoseUpdater) == 0x120);
}  // namespace al
