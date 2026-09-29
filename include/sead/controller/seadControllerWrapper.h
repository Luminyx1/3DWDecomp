#pragma once

#include "controller/seadControllerWrapperBase.h"

namespace sead
{
class ControllerWrapper : public ControllerWrapperBase
{
    SEAD_RTTI_OVERRIDE(ControllerWrapper, ControllerWrapperBase)

public:
    static const u8 cPadConfigDefault[Controller::cPadIdx_Max];

    ControllerWrapper();
    ~ControllerWrapper() override = default;

    void calc(u32 prevHold, bool prevPointerOn) override;

    u32 createPadMaskFromControllerPadMask_(u32 controllerMask) const;
    void setPadConfig(s32 padBitMax, const u8* pPadConfig, bool enableStickCrossEmulation);

protected:
    u8 mPadConfig[cPadIdx_MaxBase];
};
#ifdef cafe
static_assert(sizeof(ControllerWrapper) == 0x194, "sead::ControllerWrapper size mismatch");
#endif  // cafe

}  // namespace sead
