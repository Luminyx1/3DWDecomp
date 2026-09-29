#pragma once

#include <nn/hid.h>

#include "controller/seadControlDevice.h"

namespace sead
{
class NinDebugPadDevice : public ControlDevice
{
    SEAD_RTTI_OVERRIDE(NinDebugPadDevice, ControlDevice)

public:
    explicit NinDebugPadDevice(ControllerMgr* pMgr);
    ~NinDebugPadDevice() override;

    void calc() override;

    const nn::hid::DebugPadState& getState() const
    {
        return *reinterpret_cast<const nn::hid::DebugPadState*>(mStateStorage);
    }
    bool isConnected() const { return mIsConnected; }

private:
    alignas(nn::hid::DebugPadState) u8 mStateStorage[sizeof(nn::hid::DebugPadState)];
    bool mIsConnected;
};

}  // namespace sead
