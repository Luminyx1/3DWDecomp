#pragma once

#include "Project/Camera/Holder/ICameraInput.hpp"

namespace al {
/// Camera input read from a controller port.
class SimpleCameraInput : public ICameraInput {
public:
    SimpleCameraInput(s32 port);

    void updateInput() override;
    void calcInputStick(sead::Vector2f* pInputStick) const override;
    bool isTriggerReset() const override;
    bool isHoldZoom() const override;
    bool tryCalcSnapShotMoveStick(sead::Vector2f* pMoveStick) const override;
    bool isHoldSnapShotZoomIn() const override;
    bool isHoldSnapShotZoomOut() const override;
    bool isHoldSnapShotRollLeft() const override;
    bool isHoldSnapShotRollRight() const override;
    void setDisableInput(bool isDisable) override { mIsDisableInput = isDisable; }

    s32 mPort;                     // _8
    bool mIsDisableInput = false;  // _C
    s32 mHoldLFrame = -1;          // _10
};
}  // namespace al
