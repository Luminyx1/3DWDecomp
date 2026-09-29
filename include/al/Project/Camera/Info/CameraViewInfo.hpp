#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>

namespace sead {
class LookAtCamera;
class Projection;
}  // namespace sead

namespace al {
class CameraViewFlag;
class OrthoProjectionInfo;
class Projection;

/// Everything describing one camera view: its camera, projection and flags.
class CameraViewInfo {
public:
    CameraViewInfo(s32 index, const sead::LookAtCamera& rLookAtCam, Projection& rProjection,
                   const CameraViewFlag& rFlag, const OrthoProjectionInfo& rOrthoProjectionInfo);

    sead::Projection& getProjectionSead();
    const sead::Projection& getProjectionSead() const;
    const sead::Matrix44f* getProjMtx() const;
    f32 getAspect() const;
    f32 getNear() const;
    f32 getFar() const;

    s32 mIndex;                                        // _0
    bool mIsValid = true;                              // _4
    bool mIsFirstCalc = true;                          // _5
    bool mIsActiveInterpole = false;                   // _6
    const sead::LookAtCamera& mLookAtCam;              // _8
    Projection& mProjection;                           // _10
    const CameraViewFlag& mFlag;                       // _18
    const OrthoProjectionInfo& mOrthoProjectionInfo;   // _20
};
}  // namespace al
