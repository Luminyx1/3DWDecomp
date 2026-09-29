#include "controller/nin/seadNinDebugPadDevice.h"

namespace sead
{
/**
 * Constructs a disconnected debug pad device.
 * @param pMgr owning controller manager
 */
NinDebugPadDevice::NinDebugPadDevice(ControllerMgr* pMgr) : ControlDevice(pMgr), mIsConnected(false)
{
    mId = ControllerDefine::cDevice_NinDebugPad;
}

/**
 * Destroys the device.
 */
NinDebugPadDevice::~NinDebugPadDevice() = default;

/**
 * Does nothing; the debug pad is not read in release builds.
 */
void NinDebugPadDevice::calc() {}

}  // namespace sead
