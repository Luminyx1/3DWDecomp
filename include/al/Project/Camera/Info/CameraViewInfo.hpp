#pragma once

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <math/seadMatrix.h>

namespace al {
class CameraViewFlag;
class Projection;
struct OrthoProjectionInfo;

class CameraViewInfo {
public:
    CameraViewInfo(s32 index, const sead::LookAtCamera& rCamera, Projection& rProjection,
                   const CameraViewFlag& rFlag, const OrthoProjectionInfo& rOrthoInfo);

    sead::Projection& getProjectionSead();
    const sead::Projection& getProjectionSead() const;
    const sead::Matrix44f* getProjMtx() const;
    f32 getAspect() const;
    f32 getNear() const;
    f32 getFar() const;

    s32 getIndex() const { return mIndex; }

    const sead::LookAtCamera& getLookAtCam() const { return mLookAtCam; }

    const Projection& getProjection() const { return mProjection; }

    bool isValid() const { return mIsValid; }

    bool isFirstCalc() const { return mIsFirstCalc; }

    bool isActiveInterpole() const { return mIsActiveInterpole; }

    void setValid(bool isValid) { mIsValid = isValid; }

    void setFirstCalc(bool isFirstCalc) { mIsFirstCalc = isFirstCalc; }

    void setActiveInterpole(bool isActive) { mIsActiveInterpole = isActive; }

private:
    s32 mIndex;
    bool mIsValid = true;
    bool mIsFirstCalc = true;
    bool mIsActiveInterpole = false;
    const sead::LookAtCamera& mLookAtCam;
    Projection& mProjection;
    const CameraViewFlag& mFlag;
    const OrthoProjectionInfo& mOrthoProjectionInfo;
};

}  // namespace al
