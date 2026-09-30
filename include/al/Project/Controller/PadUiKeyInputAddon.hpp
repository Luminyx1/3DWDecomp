#pragma once

#include <controller/seadControllerAddon.h>
#include <prim/seadBitFlag.h>

namespace al {
class PadUiKeyInputAddon : public sead::ControllerAddon {
    SEAD_RTTI_OVERRIDE(PadUiKeyInputAddon, sead::ControllerAddon)

public:
    PadUiKeyInputAddon(sead::Controller* pController);

    bool calc() override;

    const sead::BitFlag32& getPadTrig() const { return mPadTrig; }
    const sead::BitFlag32& getPadHold() const { return mPadHold; }
    const sead::BitFlag32& getPadHoldAndPrev() const { return mPadHoldAndPrev; }
    const sead::BitFlag32& getPadRepeat() const { return mPadRepeat; }

private:
    sead::BitFlag32 mPadTrig = 0;
    sead::BitFlag32 mPadHold = 0;
    sead::BitFlag32 mPadHoldAndPrev = 0;
    sead::BitFlag32 mPadRepeat = 0;
};
}  // namespace al
