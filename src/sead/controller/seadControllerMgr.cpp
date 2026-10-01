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

/**
 * Constructs the manager as a standalone calculate task with its own heap array.
 */
ControllerMgr::ControllerMgr() : CalculateTask(ConstructArg(), "sead::ControllerMgr")
{
    mDevices.initOffset(offsetof(ControlDevice, mListNode));
}

/**
 * Constructs the manager as a calculate task created by the task manager.
 * @param rArg task construction arguments
 */
ControllerMgr::ControllerMgr(const TaskConstructArg& rArg)
    : CalculateTask(rArg, "sead::ControllerMgr")
{
    mDevices.initOffset(offsetof(ControlDevice, mListNode));
}

/**
 * Initializes from the task parameter if one was given, otherwise with the default setup.
 */
void ControllerMgr::prepare()
{
    auto* parameter = DynamicCast<Parameter>(mParameter);

    if (parameter != nullptr)
    {
        initialize(parameter->controllerMax, nullptr);

        if (parameter->proc != nullptr)
        {
            parameter->proc->invoke(this);
        }
    }
    else
    {
        initializeDefault(nullptr);
    }
}

/**
 * Allocates the controller pointer array.
 * @param controllerMax maximum number of controllers
 * @param pHeap heap to allocate from
 */
void ControllerMgr::initialize(s32 controllerMax, Heap* pHeap)
{
    mControllers.allocBuffer(controllerMax, pHeap);
}

/**
 * Frees the controller pointer array.
 */
void ControllerMgr::finalize()
{
    mControllers.freeBuffer();
}

/**
 * Allocates room for 16 controllers and registers a NinJoyNpadDevice.
 * @param pHeap heap to allocate from
 */
void ControllerMgr::initializeDefault(Heap* pHeap)
{
    initialize(16, pHeap);

    mDevices.pushBack(new (pHeap) NinJoyNpadDevice(this, pHeap));
}

/**
 * Removes and deletes the NinJoyNpadDevice, then frees the controller array.
 */
void ControllerMgr::finalizeDefault()
{
    auto* device = getControlDevice(ControllerDefine::cDevice_NinJoyNpad);

    if (device != nullptr)
    {
        mDevices.erase(device);
        delete device;
    }

    finalize();
}

/**
 * Updates every control device, then every controller.
 */
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

/**
 * Finds the index-th controller with the given id.
 * @param id controller id to look for
 * @param index how many matching controllers to skip
 * @return the controller, or nullptr if there is none
 */
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

/**
 * Finds the first control device with the given id.
 * @param id device id to look for
 * @return the device, or nullptr if there is none
 */
ControlDevice* ControllerMgr::getControlDevice(ControllerDefine::DeviceId id) const
{
    for (auto it = mDevices.begin(); it != mDevices.end(); ++it)
    {
        if (it->mId == id)
        {
            return &*it;
        }
    }

    return nullptr;
}

/**
 * Finds the first addon with the given id on the controller in a port.
 * @param index controller port
 * @param id addon id to look for
 * @return the addon, or nullptr if there is none
 */
ControllerAddon* ControllerMgr::getControllerAddon(s32 index, ControllerDefine::AddonId id) const
{
    Controller* controller = mControllers.at(index);

    if (controller != nullptr)
    {
        return controller->getAddon(id);
    }

    return nullptr;
}

/**
 * Finds the addonIndex-th addon with the given id on the controller in a port.
 * @param controllerIndex controller port
 * @param id addon id to look for
 * @param addonIndex how many matching addons to skip
 * @return the addon, or nullptr if there is none
 */
ControllerAddon* ControllerMgr::getControllerAddonByOrder(s32 controllerIndex,
                                                          ControllerDefine::AddonId id,
                                                          s32 addonIndex) const
{
    Controller* controller = mControllers.at(controllerIndex);

    if (controller != nullptr)
    {
        return controller->getAddonByOrder(id, addonIndex);
    }

    return nullptr;
}

/**
 * Finds the port a controller is registered in.
 * @param pController controller to look for
 * @return the port, or -1 if the controller is not registered
 */
s32 ControllerMgr::findControllerPort(const Controller* pController) const
{
    s32 i = 0;

    for (auto& controller : mControllers)
    {
        if (&controller == pController)
        {
            return i;
        }

        i++;
    }

    return -1;
}

/**
 * Appends a control device to the device list.
 * @param pDevice device to add
 */
void ControllerMgr::pushBackControlDevice(ControlDevice* pDevice)
{
    mDevices.pushBack(pDevice);
}

/**
 * Removes a control device from the device list.
 * @param pDevice device to remove
 */
void ControllerMgr::removeControlDevice(ControlDevice* pDevice)
{
    mDevices.erase(pDevice);
}

/**
 * Appends a controller to the next free port.
 * @param pController controller to add
 */
void ControllerMgr::pushBackController(Controller* pController)
{
    mControllers.pushBack(pController);
}

/**
 * Removes a controller from its port.
 * @param pController controller to remove
 */
void ControllerMgr::removeController(Controller* pController)
{
    s32 index = mControllers.indexOf(pController);

    if (index >= 0)
    {
        mControllers.erase(index);
    }
}

/**
 * Gets the framework that owns the task manager of this task.
 * @return the framework, or nullptr if the task has no manager
 */
Framework* ControllerMgr::getFramework() const
{
    if (mTaskMgr != nullptr)
    {
        return mTaskMgr->mParentFramework;
    }

    return nullptr;
}

}  // namespace sead
