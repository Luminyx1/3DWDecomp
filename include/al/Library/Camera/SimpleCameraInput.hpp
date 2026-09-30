#pragma once

#include "Library/Camera/ICameraInput.hpp"

namespace al {

class SimpleCameraInput : public ICameraInput {
public:
    SimpleCameraInput(s32 port);

    void updateInput() override;
    void calcInputStick(sead::Vector2f* pStick) const override;
    bool isTriggerReset() const override;
    bool isHoldZoom() const override;
    bool tryCalcSnapShotMoveStick(sead::Vector2f* pStick) const override;
    bool isHoldSnapShotZoomIn() const override;
    bool isHoldSnapShotZoomOut() const override;
    bool isHoldSnapShotRollLeft() const override;
    bool isHoldSnapShotRollRight() const override;

    void setDisableInput(bool isDisable) override { mIsDisableInput = isDisable; }

private:
    s32 mPort;
    bool mIsDisableInput = false;
    s32 mResetFrame = -1;
};

}  // namespace al
