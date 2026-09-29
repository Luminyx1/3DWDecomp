#pragma once

#include <math/seadMatrix.h>

#include "Project/Se/SeSource.hpp"

namespace al {
class SeSourcePose3DMtxBase;

class SeSource3DRing : public SeSource3D {
public:
    SeSource3DRing(SeSourcePose3DMtxBase* pPose, const f32* pRadius, AudioSystemInfo* pInfo);

    void calcPositionInitialize() override;
    void calcPositionDynamic() override;
    const sead::Vector3f* calcPosition(const sead::Vector3f& rListenerPos) override;

    SeSourcePose3DMtxBase* mPoseMtx;                                    // _40
    const f32* mRadius;                                                 // _48
    sead::Vector3f mCalcPos = {0.0f, 0.0f, 0.0f};                       // _50
    sead::Matrix34f mInvPoseMtx;                                        // _5C
};
}  // namespace al
