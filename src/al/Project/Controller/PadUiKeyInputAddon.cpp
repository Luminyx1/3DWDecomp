#include "Project/Controller/PadUiKeyInputAddon.hpp"

#include <controller/seadController.h>
#include <controller/seadControllerMgr.h>

#include "Library/Controller/PadReplayFunction.hpp"
#include "Library/Controller/ReplayController.hpp"

namespace al {
/**
 * Creates the UI cursor key input addon.
 * @param pController controller the addon belongs to
 */
PadUiKeyInputAddon::PadUiKeyInputAddon(sead::Controller* pController)
    : sead::ControllerAddon(pController) {
    mId = sead::ControllerDefine::cAddon_UiKeyInput;
}

/**
 * Updates the UI cursor key state from the direction buttons and the left stick.
 * @return whether any direction is held
 */
bool PadUiKeyInputAddon::calc() {
    s32 port = mController->getMgr()->findControllerPort(mController);
    sead::ControllerBase* controller;

    if (isValidReplayController(port)) {
        controller = getReplayController(port);
    } else {
        controller = mController;
    }

    u32 prevPadHold = mPadHold.getDirect();

    u32 maskUp = sead::Controller::cPadMask_Up | sead::Controller::cPadMask_LeftStickUp;
    u32 maskDown = sead::Controller::cPadMask_Down | sead::Controller::cPadMask_LeftStickDown;
    u32 maskLeft = sead::Controller::cPadMask_Left | sead::Controller::cPadMask_LeftStickLeft;
    u32 maskRight = sead::Controller::cPadMask_Right | sead::Controller::cPadMask_LeftStickRight;

    mPadHold.changeBit(0, controller->isHold(maskUp));
    mPadHold.changeBit(1, controller->isHold(maskDown));
    mPadHold.changeBit(2, controller->isHold(maskLeft));
    mPadHold.changeBit(3, controller->isHold(maskRight));

    mPadTrig.setDirect(mPadHold.getDirect() & ~prevPadHold);
    mPadHoldAndPrev.setDirect(mPadHold.getDirect() & prevPadHold);

    mPadRepeat.changeBit(0, controller->isTrigWithRepeat(maskUp));
    mPadRepeat.changeBit(1, controller->isTrigWithRepeat(maskDown));
    mPadRepeat.changeBit(2, controller->isTrigWithRepeat(maskLeft));
    mPadRepeat.changeBit(3, controller->isTrigWithRepeat(maskRight));

    return mPadHold.getDirect() != 0;
}
}  // namespace al
