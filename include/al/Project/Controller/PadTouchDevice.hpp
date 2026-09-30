#pragma once

#include <controller/seadControlDevice.h>
#include <controller/seadController.h>
#include <nn/hid.h>

namespace al {
class PadTouchDevice : public sead::ControlDevice {
public:
    PadTouchDevice(sead::ControllerMgr* pMgr);
    ~PadTouchDevice() override;

    void calc() override;

    const nn::hid::TouchScreenState<1>& getTouchScreenState() const { return mTouchScreenState; }

private:
    nn::hid::TouchScreenState<1> mTouchScreenState;
};

class PadTouchController : public sead::Controller {
public:
    PadTouchController(sead::ControllerMgr* pMgr);
    ~PadTouchController() override;

    bool gatherInput();
    void applyInput();

private:
    void calcImpl_() override;

    nn::hid::TouchScreenState<1> mTouchScreenState;
};
}  // namespace al
