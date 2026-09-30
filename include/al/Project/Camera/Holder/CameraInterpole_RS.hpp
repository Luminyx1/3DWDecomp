#pragma once

#include <gfx/seadCamera.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.hpp"

namespace al {
class CameraTicket;

class CameraInterpole_RS : public NerveExecutor {
public:
    CameraInterpole_RS();
    ~CameraInterpole_RS() override;

    void start(const CameraTicket* pTicket, f32 fovyDegree, s32 step);
    void update(const sead::LookAtCamera& rCamera);
    bool isActive() const;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const;
    void lerpFovyDegree(f32 rate);
    void exeDeactive();
    void exeActiveHermite();
    void exeActiveHermiteDistanceHV();
    void exeActiveRotateAxisY();
    void exeActiveBrigade();

    void setTicket(const CameraTicket* pTicket) { mTicket = pTicket; }

    s32 getStep() const { return mStep; }

    void requestCancel() { mIsRequestCancel = true; }

    f32 getFovyDegree() const { return mFovyDegree; }

private:
    const CameraTicket* mTicket = nullptr;
    void* _18 = nullptr;
    void* _20 = nullptr;
    s32 mStep = -1;
    bool mIsRequestCancel = false;
    void* _30 = nullptr;
    void* _38 = nullptr;
    void* _40 = nullptr;
    sead::Vector3f _48;
    sead::LookAtCamera mCameraPrev;
    sead::LookAtCamera mCameraStart;
    sead::LookAtCamera mCameraNext;
    sead::LookAtCamera mCameraCurrent;
    f32 mFovyDegree = 30.0f;
    f32 mFovyDegreeStart = 30.0f;
    void* _1e0 = nullptr;
    void* _1e8 = nullptr;
    void* _1f0 = nullptr;
};

static_assert(sizeof(CameraInterpole_RS) == 0x1f8);

}  // namespace al
