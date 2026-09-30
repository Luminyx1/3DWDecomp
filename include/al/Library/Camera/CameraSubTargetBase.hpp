#pragma once

#include "Library/Camera/CameraTargetBase.hpp"

namespace al {
struct CameraSubTargetTurnParam;

class CameraSubTargetBase : public CameraTargetBase {
public:
    CameraSubTargetBase();

    void calcSide(sead::Vector3f* pSide) const override { pSide->set(1.0f, 0.0f, 0.0f); }

    void calcUp(sead::Vector3f* pUp) const override { pUp->set(0.0f, 1.0f, 0.0f); }

    void calcFront(sead::Vector3f* pFront) const override { pFront->set(0.0f, 0.0f, 1.0f); }

    void calcVelocity(sead::Vector3f* pVelocity) const override { pVelocity->set(0.0f, 0.0f, 0.0f); }

    const CameraSubTargetTurnParam* getSubTargetTurnParam() const { return mTurnParam; }

    void setSubTargetTurnParam(const CameraSubTargetTurnParam* pParam) { mTurnParam = pParam; }

private:
    const CameraSubTargetTurnParam* mTurnParam;
};

static_assert(sizeof(CameraSubTargetBase) == 0x18);

}  // namespace al
