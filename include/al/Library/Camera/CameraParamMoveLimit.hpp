#pragma once

#include <gfx/seadCamera.h>
#include <math/seadMatrix.h>

namespace al {
class ByamlIter;
class CameraPoser_RS;

class CameraParamMoveLimit {
public:
    static CameraParamMoveLimit* create(const CameraPoser_RS* pPoser);

    CameraParamMoveLimit();

    void load(const ByamlIter& rIter);
    void setPauseApply(bool isPause);
    void setWaterHeight(f32 height);
    void pauseInterpolate(sead::LookAtCamera* pCamera, sead::Vector3f& rPos);
    void apply(sead::LookAtCamera* pCamera);

private:
    sead::Vector3f mPlus = {100000.0f, 100000.0f, 100000.0f};
    sead::Vector3f mMinus = {-100000.0f, -100000.0f, -100000.0f};
    bool mHasPlusX = false;
    bool mHasMinusX = false;
    bool mHasPlusY = false;
    bool mHasMinusY = false;
    bool mHasPlusZ = false;
    bool mHasMinusZ = false;
    bool mIsPauseApply;
    bool mIsPauseInterpolate;
    bool mHasWaterHeight;
    f32 mWaterHeight;
    sead::Vector3f mPauseOffset;
    sead::Vector3f mPauseOffsetTarget;
    sead::Matrix34f mViewMtx = sead::Matrix34f::ident;
    sead::Matrix34f mInvViewMtx = sead::Matrix34f::ident;
    f32 mRotYDegree = 0.0f;
};

static_assert(sizeof(CameraParamMoveLimit) == 0xa4);

}  // namespace al
