#include "controller/seadControllerWrapperBase.h"

#include "prim/seadMemUtil.h"

namespace sead
{
/**
 * Constructs an enabled wrapper that is not registered with any controller.
 */
ControllerWrapperBase::ControllerWrapperBase()
    : ControllerBase(Controller::cPadIdx_Max, -1, -1, Controller::cPadIdx_Touch),
      mController(nullptr), mIsEnable(true)
{
}

/**
 * Unregisters the wrapper from its controller.
 */
ControllerWrapperBase::~ControllerWrapperBase()
{
    unregister();
}

/**
 * Removes the wrapper from the wrapper list of its controller.
 */
void ControllerWrapperBase::unregister()
{
    if (mController)
    {
        mController->mWrappers.erase(this);
        mController = nullptr;
    }
}

/**
 * Registers the wrapper with a controller, leaving the previous one.
 * @param pController controller to wrap
 * @param copyRepeatSetting_ whether to copy the controller's pad repeat settings
 */
void ControllerWrapperBase::registerWith(Controller* pController, bool copyRepeatSetting_)
{
    unregister();

    pController->mWrappers.pushBack(this);
    mController = pController;

    if (copyRepeatSetting_)
    {
        copyRepeatSetting(pController);
    }
}

/**
 * Copies the pad repeat delays and intervals of a controller.
 * @param pController controller to copy from
 */
void ControllerWrapperBase::copyRepeatSetting(const Controller* pController)
{
    MemUtil::copy(mPadRepeatDelays, pController->mPadRepeatDelays, cPadIdx_MaxBase);
    MemUtil::copy(mPadRepeatPulses, pController->mPadRepeatPulses, cPadIdx_MaxBase);
}

/**
 * Enables or disables the wrapper.
 * @param enable whether the wrapper is enabled
 */
void ControllerWrapperBase::setEnable(bool enable)
{
    mIsEnable = enable;
}

/**
 * Enables or disables every other wrapper registered with the same controller.
 * @param enable whether the other wrappers are enabled
 */
void ControllerWrapperBase::setEnableOtherWrappers(bool enable) const
{
    if (!mController)
    {
        return;
    }

    OffsetList<ControllerWrapperBase>& wrappers = mController->mWrappers;

    for (auto it = wrappers.begin(); it != wrappers.end(); ++it)
    {
        if (&*it != this)
        {
            it->setEnable(enable);
        }
    }
}

/**
 * Clears all input state.
 */
void ControllerWrapperBase::setIdle()
{
    setIdleBase_();
}

/**
 * Checks whether no input is active.
 * @return true if the wrapper is idle
 */
bool ControllerWrapperBase::isIdle_()
{
    return isIdleBase_();
}

}  // namespace sead
