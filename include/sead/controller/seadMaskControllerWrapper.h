#pragma once

#include "controller/seadControllerWrapperBase.h"

namespace sead
{
class MaskControllerWrapper : public ControllerWrapperBase
{
    SEAD_RTTI_OVERRIDE(MaskControllerWrapper, ControllerWrapperBase)

public:
    static const u32 cPadConfigDefault[Controller::cPadIdx_Max];

    MaskControllerWrapper();
    ~MaskControllerWrapper() override = default;

    void calc(u32 prevHold, bool prevPointerOn) override;

    u32 createPadMaskFromControllerPadMask_(u32 controllerMask) const;
    void setPadConfig(s32 padBitMax, const u32* pPadConfig, bool enableStickCrossEmulation);

protected:
    u32 mPadConfig[cPadIdx_MaxBase];
};

}  // namespace sead
