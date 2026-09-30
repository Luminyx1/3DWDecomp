#pragma once

#include <math/seadVector.h>

namespace al {

class ICameraInput {
public:
    virtual void updateInput() = 0;
    virtual void calcInputStick(sead::Vector2f* pStick) const = 0;
    virtual bool isTriggerReset() const = 0;
    virtual bool isHoldZoom() const = 0;

    virtual bool tryCalcSnapShotMoveStick(sead::Vector2f* pStick) const { return false; }

    virtual bool isHoldSnapShotZoomIn() const = 0;
    virtual bool isHoldSnapShotZoomOut() const = 0;

    virtual bool isHoldSnapShotRollLeft() const { return false; }

    virtual bool isHoldSnapShotRollRight() const { return false; }

    virtual void calcGyroPose(sead::Vector3f* pSide, sead::Vector3f* pUp,
                              sead::Vector3f* pFront) const {
        if (pSide) {
            pSide->set(sead::Vector3f::ex);
        }
        if (pUp) {
            pUp->set(sead::Vector3f::ey);
        }
        if (pFront) {
            pFront->set(sead::Vector3f::ez);
        }
    }

    virtual void setDisableInput(bool isDisable) = 0;
};

}  // namespace al
