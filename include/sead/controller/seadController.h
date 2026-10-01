#pragma once

#include "container/seadOffsetList.h"
#include "controller/seadControllerBase.h"
#include "controller/seadControllerDefine.h"

namespace sead
{
class ControllerAddon;
class ControllerMgr;
class ControllerWrapperBase;

class Controller : public ControllerBase
{
    SEAD_RTTI_OVERRIDE(Controller, ControllerBase)
public:
    enum PadIdx
    {
        cPadIdx_A = 0,
        cPadIdx_B = 1,
        cPadIdx_C = 2,
        cPadIdx_X = 3,
        cPadIdx_Y = 4,
        cPadIdx_Z = 5,
        cPadIdx_2 = 6,  // Also Right-Stick Click
        cPadIdx_1 = 7,  // Also Left-Stick Click
        cPadIdx_Home = 8,
        cPadIdx_Minus = 9,
        cPadIdx_Plus = 10,
        cPadIdx_Start = 11,
        cPadIdx_Select = 12,
        cPadIdx_ZL = cPadIdx_C,
        cPadIdx_ZR = cPadIdx_Z,
        cPadIdx_L = 13,
        cPadIdx_R = 14,
        cPadIdx_Touch = 15,
        cPadIdx_Up = 16,
        cPadIdx_Down = 17,
        cPadIdx_Left = 18,
        cPadIdx_Right = 19,
        cPadIdx_LeftStickUp = 20,
        cPadIdx_LeftStickDown = 21,
        cPadIdx_LeftStickLeft = 22,
        cPadIdx_LeftStickRight = 23,
        cPadIdx_RightStickUp = 24,
        cPadIdx_RightStickDown = 25,
        cPadIdx_RightStickLeft = 26,
        cPadIdx_RightStickRight = 27,
        cPadIdx_Max = 28
    };

    enum PadMask
    {
        cPadMask_A = 1 << cPadIdx_A,
        cPadMask_B = 1 << cPadIdx_B,
        cPadMask_C = 1 << cPadIdx_C,
        cPadMask_X = 1 << cPadIdx_X,
        cPadMask_Y = 1 << cPadIdx_Y,
        cPadMask_Z = 1 << cPadIdx_Z,
        cPadMask_2 = 1 << cPadIdx_2,
        cPadMask_1 = 1 << cPadIdx_1,
        cPadMask_Home = 1 << cPadIdx_Home,
        cPadMask_Minus = 1 << cPadIdx_Minus,
        cPadMask_Plus = 1 << cPadIdx_Plus,
        cPadMask_Start = 1 << cPadIdx_Start,
        cPadMask_Select = 1 << cPadIdx_Select,
        cPadMask_L = 1 << cPadIdx_L,
        cPadMask_R = 1 << cPadIdx_R,
        cPadMask_Touch = 1 << cPadIdx_Touch,
        cPadMask_Up = 1 << cPadIdx_Up,
        cPadMask_Down = 1 << cPadIdx_Down,
        cPadMask_Left = 1 << cPadIdx_Left,
        cPadMask_Right = 1 << cPadIdx_Right,
        cPadMask_LeftStickUp = 1 << cPadIdx_LeftStickUp,
        cPadMask_LeftStickDown = 1 << cPadIdx_LeftStickDown,
        cPadMask_LeftStickLeft = 1 << cPadIdx_LeftStickLeft,
        cPadMask_LeftStickRight = 1 << cPadIdx_LeftStickRight,
        cPadMask_RightStickUp = 1 << cPadIdx_RightStickUp,
        cPadMask_RightStickDown = 1 << cPadIdx_RightStickDown,
        cPadMask_RightStickLeft = 1 << cPadIdx_RightStickLeft,
        cPadMask_RightStickRight = 1 << cPadIdx_RightStickRight,
        cPadMask_ZL = cPadMask_C,
        cPadMask_ZR = cPadMask_Z,
    };

    explicit Controller(ControllerMgr* pMgr);
    virtual ~Controller() = default;

    virtual void calc();
    virtual bool isConnected() const { return true; }
    ControllerAddon* getAddonByOrder(ControllerDefine::AddonId id, s32 index) const;
    ControllerAddon* getAddon(ControllerDefine::AddonId id) const;
    ControllerMgr* getMgr() const { return mMgr; }

    template <typename T>
    T getAddonAs() const;
    template <typename T>
    T getAddonByOrderAs(s32 index) const;
    template <typename T>
    T getWrapperAs() const;

    OffsetList<ControllerAddon>& getAddonList() { return mAddons; }

protected:
    virtual void calcImpl_() = 0;
    virtual bool isIdle_();
    virtual void setIdle_();

    ControllerDefine::ControllerId mId;

private:
    ControllerMgr* mMgr;
    OffsetList<ControllerAddon> mAddons;
    OffsetList<ControllerWrapperBase> mWrappers;

    friend class ControllerWrapperBase;
    friend class ControllerMgr;
};
#ifdef cafe
static_assert(sizeof(Controller) == 0x15C, "sead::Controller size mismatch");
#endif  // cafe

template <typename T>
T Controller::getAddonAs() const
{
    for (auto& addon : mAddons)
    {
        T result = DynamicCast<typename std::remove_pointer<T>::type>(&addon);
        if (result)
            return result;
    }

    return nullptr;
}

template <typename T>
T Controller::getAddonByOrderAs(s32 index) const
{
    for (auto it = mAddons.begin(); it != mAddons.end(); ++it)
    {
        T result = DynamicCast<typename std::remove_pointer<T>::type>(&*it);
        if (result)
        {
            if (index == 0)
                return result;
            index--;
        }
    }

    return nullptr;
}

template <typename T>
T Controller::getWrapperAs() const
{
    for (auto it = mWrappers.begin(); it != mWrappers.end(); ++it)
    {
        T result = DynamicCast<typename std::remove_pointer<T>::type>(&*it);
        if (result)
            return result;
    }

    return nullptr;
}

}  // namespace sead
