#include "controller/seadController.h"

#include "controller/seadControllerAddon.h"
#include "controller/seadControllerDefine.h"
#include "controller/seadControllerMgr.h"
#include "controller/seadControllerWrapperBase.h"

namespace sead
{
/**
 * Sets up the standard pad layout with stick crosses and a touch key.
 * @param pMgr owning controller manager
 */
Controller::Controller(ControllerMgr* pMgr)
    : ControllerBase(cPadIdx_Max, cPadIdx_LeftStickUp, cPadIdx_RightStickUp, cPadIdx_Touch),
      mId(ControllerDefine::cController_Null), mMgr(pMgr)
{
    mAddons.initOffset(offsetof(ControllerAddon, mListNode));
    mWrappers.initOffset(offsetof(ControllerWrapperBase, mListNode));
}

/**
 * Reads new input, updates addons and the idle counter, then updates all registered wrappers.
 */
void Controller::calc()
{
    u32 prev_hold = getHoldMask();
    bool prev_pointer_on = isPointerOn();

    calcImpl_();

    updateDerivativeParams_(prev_hold, prev_pointer_on);

    bool is_idle = true;

    for (auto it = mAddons.begin(); it != mAddons.end(); ++it)
    {
        if (it->calc())
        {
            is_idle = false;
        }
    }

    if (is_idle && isIdle_())
    {
        mIdleFrame++;
    }
    else
    {
        mIdleFrame = 0;
    }

    for (auto it = mWrappers.begin(); it != mWrappers.end(); ++it)
    {
        it->calc(prev_hold, prev_pointer_on);
    }
}

/**
 * Finds the first addon with the given id.
 * @param id addon id to look for
 * @return the addon, or nullptr if there is none
 */
ControllerAddon* Controller::getAddon(ControllerDefine::AddonId id) const
{
    return getAddonByOrder(id, 0);
}

/**
 * Finds the index-th addon with the given id.
 * @param id addon id to look for
 * @param index how many matching addons to skip
 * @return the addon, or nullptr if there is none
 */
ControllerAddon* Controller::getAddonByOrder(ControllerDefine::AddonId id, s32 index) const
{
    for (auto it = mAddons.begin(); it != mAddons.end(); ++it)
    {
        if (it->mId == id)
        {
            if (index == 0)
            {
                return &*it;
            }
            index--;
        }
    }

    return nullptr;
}

/**
 * Checks whether no input is active.
 * @return true if the controller is idle
 */
bool Controller::isIdle_()
{
    return isIdleBase_();
}

/**
 * Clears all input state of the controller and its wrappers.
 */
void Controller::setIdle_()
{
    setIdleBase_();

    for (auto it = mWrappers.begin(); it != mWrappers.end(); ++it)
    {
        it->setIdle();
    }
}

}  // namespace sead
