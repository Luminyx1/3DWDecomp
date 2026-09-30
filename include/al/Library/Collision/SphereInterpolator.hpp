#pragma once

#include <math/seadVector.h>

namespace al {
class SphereInterpolator {
public:
    SphereInterpolator() {}

    void startInterp(const sead::Vector3f& rPosStart, const sead::Vector3f& rPosEnd,
                     f32 sizeStart, f32 sizeEnd, f32 steps);
    void nextStep();
    void calcInterpPos(sead::Vector3f* pPos) const;
    void calcInterp(sead::Vector3f* pPos, f32* pSize, sead::Vector3f* pRemainMoveVec) const;
    void calcRemainMoveVector(sead::Vector3f* pRemainMoveVec) const;
    void getMoveVector(sead::Vector3f* pMoveVec);

private:
    sead::Vector3f mPos;
    sead::Vector3f mMove;
    f32 mSizeStart;
    f32 mSizeEnd;
    f32 mStepSize;
    f32 mCurrentStep;
    f32 mPrevStep;
};
}  // namespace al
