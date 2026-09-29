#include "controller/seadControllerMgr.h"
#include "basis/seadNew.h"
#include "controller/nin/seadNinJoyNpadDevice.h"
#include "controller/seadControlDevice.h"
#include "framework/seadTaskID.h"
#include "prim/seadDelegate.h"
#include "thread/seadDelegateThread.h"

namespace sead
{
SEAD_TASK_SINGLETON_IMPL(ControllerMgr)

// NON_MATCHING: storing too much 00s into stack (for ConstructArg)
ControllerMgr::ControllerMgr() : CalculateTask(ConstructArg(), "sead::ControllerMgr")
{
    mDevices.initOffset(offsetof(ControlDevice, mListNode));
}

ControllerMgr::ControllerMgr(const TaskConstructArg& rArg)
    : CalculateTask(rArg, "sead::ControllerMgr")
{
    mDevices.initOffset(offsetof(ControlDevice, mListNode));
}

void ControllerMgr::prepare()
{
    auto* parameter = DynamicCast<Parameter>(mParameter);
    if (parameter)
    {
        initialize(parameter->controllerMax, nullptr);
        if (parameter->proc)
        {
            parameter->proc->invoke(this);
        }
    }
    else
    {
        initializeDefault(nullptr);
    }
}

void ControllerMgr::initialize(s32 controllerMax, Heap* pHeap)
{
    mControllers.allocBuffer(controllerMax, pHeap);
}

void ControllerMgr::finalize()
{
    mControllers.freeBuffer();
}

void ControllerMgr::initializeDefault(Heap* pHeap)
{
    s32 controller_max;
#ifdef cafe
    controller_max = 6;
#elif defined(NNSDK)
    controller_max = 16;
#else
#error "Unknown Platform"
#endif
    initialize(controller_max, pHeap);

#ifdef NNSDK
    mDevices.pushBack(new (pHeap) NinJoyNpadDevice(this, pHeap));
#endif
}

void ControllerMgr::finalizeDefault()
{
#ifdef NNSDK
    // NON_MATCHING: missing cbz instruction within loop
    for (auto& device : mDevices)
    {
        if (device.getId() == 13)
        {
            mDevices.erase(&device);
            delete &device;
            break;
        }
    }
#endif  // cafe

    finalize();
}

void ControllerMgr::calc()
{
    for (auto it = mDevices.begin(); it != mDevices.end(); ++it)
    {
        it->calc();
    }

    for (auto it = mControllers.begin(); it != mControllers.end(); ++it)
    {
        it->calc();
    }
}

Controller* ControllerMgr::getControllerByOrder(ControllerDefine::ControllerId id, s32 index) const
{
    for (auto& controller : mControllers)
    {
        if (controller.mId == id)
        {
            if (index == 0)
            {
                return &controller;
            }

            index--;
        }
    }

    return nullptr;
}

ControlDevice* ControllerMgr::getControlDevice(ControllerDefine::DeviceId id) const
{
    for (auto& device : mDevices)
    {
        if (device.mId == id)
        {
            return &device;
        }
    }

    return nullptr;
}

ControllerAddon* ControllerMgr::getControllerAddon(s32 index, ControllerDefine::AddonId id) const
{
    Controller* controller = mControllers.at(index);
    if (controller)
    {
        return controller->getAddon(id);
    }

    return nullptr;
}

ControllerAddon* ControllerMgr::getControllerAddonByOrder(s32 controllerIndex,
                                                          ControllerDefine::AddonId id,
                                                          s32 addonIndex) const
{
    Controller* controller = mControllers.at(controllerIndex);
    if (controller)
    {
        return controller->getAddonByOrder(id, addonIndex);
    }

    return nullptr;
}

s32 ControllerMgr::findControllerPort(const Controller* pController) const
{
    SEAD_ASSERT(pController);

    s32 i = 0;
    for (auto& controller_it : mControllers)
    {
        if (&controller_it == pController)
        {
            return i;
        }
        i++;
    }
    return -1;
}

Framework* ControllerMgr::getFramework() const
{
    if (mTaskMgr)
    {
        return mTaskMgr->mParentFramework;
    }
    return nullptr;
}

}  // namespace sead
