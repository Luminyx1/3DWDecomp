#pragma once

#include <math/seadVector.h>

namespace al {
/// Interface for the controller input that drives the cameras.
class ICameraInput {
public:
    virtual void updateInput() = 0;
    virtual void calcInputStick(sead::Vector2f* pInputStick) const = 0;
    virtual bool isTriggerReset() const = 0;
    virtual bool isHoldZoom() const = 0;
    virtual bool tryCalcSnapShotMoveStick(sead::Vector2f* pMoveStick) const = 0;
    virtual bool isHoldSnapShotZoomIn() const = 0;
    virtual bool isHoldSnapShotZoomOut() const = 0;
    virtual bool isHoldSnapShotRollLeft() const = 0;
    virtual bool isHoldSnapShotRollRight() const = 0;

    virtual void calcGyroPose(sead::Vector3f* pSide, sead::Vector3f* pUp, sead::Vector3f* pFront) const {
        if (pSide != nullptr) {
            pSide->set(sead::Vector3f::ex);
        }
        if (pUp != nullptr) {
            pUp->set(sead::Vector3f::ey);
        }
        if (pFront != nullptr) {
            pFront->set(sead::Vector3f::ez);
        }
    }

    virtual void setDisableInput(bool isDisable) = 0;
};
}  // namespace al
