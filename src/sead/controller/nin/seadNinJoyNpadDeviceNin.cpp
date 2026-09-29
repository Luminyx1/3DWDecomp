#include "controller/nin/seadNinJoyNpadDevice.h"

#include "mc/seadCoreInfo.h"
#include "prim/seadMemUtil.h"
#include "thread/seadThreadUtil.h"

namespace sead
{
NinJoyNpadDevice::NinJoyNpadDevice(ControllerMgr* pMgr, Heap* pHeap)
    : ControlDevice(pMgr), mNpadIdUpdateNum(8), mNpadJoyHoldType(nn::hid::NpadJoyHoldType::Vertical),
      mVibrationThread(pHeap)
{
    mId = ControllerDefine::cDevice_NinJoyNpad;

    nn::hid::InitializeNpad();

    {
        const u32 npad_ids[] = {0, 1, 2, 3, 4, 5, 6, 7, 0x20};
        nn::hid::SetSupportedNpadIdType(npad_ids, 9);
    }

    for (s32 i = 0; i < 9; i++)
    {
        mNpadStyleTags[i] = nn::hid::NpadStyleTag::NpadStyleInvalid;
        MemUtil::fillZero(&mNpadStates[i], sizeof(NpadState));
    }

    nn::hid::NpadStyleSet style_set;
    style_set.Reset();
    style_set.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleFullKey));
    style_set.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleHandheld));
    style_set.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyDual));
    style_set.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyLeft));
    style_set.Set(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyRight));
    nn::hid::SetSupportedNpadStyleSet(style_set);

    mVibrationThread.setAffinity(CoreIdMask(CoreId::cSub1));
    mVibrationThread.start();
}

/**
 * Stops the vibration thread.
 */
NinJoyNpadDevice::~NinJoyNpadDevice()
{
    mVibrationThread.quitAndDestroySingleThread(false);
}

/**
 * Reads the states of all updated npads, refreshing sensor and vibration handles when the style changes.
 */
void NinJoyNpadDevice::calc()
{
    for (s32 i = 0; i <= mNpadIdUpdateNum; i++)
    {
        s32 idx = i == mNpadIdUpdateNum ? 8 : i;
        u32 npad_id = i == mNpadIdUpdateNum ? 0x20 : i;

        nn::hid::NpadStyleSet style_set = nn::hid::GetNpadStyleSet(npad_id);
        NpadState& state = mNpadStates[idx];

        nn::hid::NpadStyleTag style;
        if (style_set.Test(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyDual)))
        {
            nn::hid::GetNpadStates(
                reinterpret_cast<nn::hid::NpadJoyDualState*>(state.mStates.getBufferPtr()), 16,
                npad_id);
            style = nn::hid::NpadStyleTag::NpadStyleJoyDual;
        }
        else if (style_set.Test(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleFullKey)))
        {
            nn::hid::GetNpadStates(
                reinterpret_cast<nn::hid::NpadFullKeyState*>(state.mStates.getBufferPtr()), 16,
                npad_id);
            style = nn::hid::NpadStyleTag::NpadStyleFullKey;
        }
        else if (style_set.Test(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyLeft)))
        {
            nn::hid::GetNpadStates(
                reinterpret_cast<nn::hid::NpadJoyLeftState*>(state.mStates.getBufferPtr()), 16,
                npad_id);
            style = nn::hid::NpadStyleTag::NpadStyleJoyLeft;
        }
        else if (style_set.Test(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleJoyRight)))
        {
            nn::hid::GetNpadStates(
                reinterpret_cast<nn::hid::NpadJoyRightState*>(state.mStates.getBufferPtr()), 16,
                npad_id);
            style = nn::hid::NpadStyleTag::NpadStyleJoyRight;
        }
        else if (style_set.Test(static_cast<s32>(nn::hid::NpadStyleTag::NpadStyleHandheld)))
        {
            nn::hid::GetNpadStates(
                reinterpret_cast<nn::hid::NpadHandheldState*>(state.mStates.getBufferPtr()), 16,
                npad_id);
            style = nn::hid::NpadStyleTag::NpadStyleHandheld;
        }
        else
        {
            style = nn::hid::NpadStyleTag::NpadStyleInvalid;
        }

        nn::hid::NpadStyleTag& prev_style = mNpadStyleTags[idx];
        if (prev_style != style)
        {
            if (style == nn::hid::NpadStyleTag::NpadStyleInvalid)
            {
                state.mSixAxisDeviceNum = 0;
                state.mVibrationDeviceNum = 0;
            }
            else
            {
                nn::hid::NpadStyleSet six_axis_style;
                six_axis_style.Reset();
                six_axis_style.Set(static_cast<s32>(style));
                state.mSixAxisDeviceNum = nn::hid::GetSixAxisSensorHandles(
                    state.mSixAxisSensorHandles.getBufferPtr(), 2, npad_id, six_axis_style);

                nn::hid::NpadStyleSet vibration_style;
                vibration_style.Reset();
                vibration_style.Set(static_cast<s32>(style));
                state.mVibrationDeviceNum = nn::hid::GetVibrationDeviceHandles(
                    state.mVibrationDeviceHandles.getBufferPtr(), 2, npad_id, vibration_style);

                for (s32 j = 0; j < state.mSixAxisDeviceNum; j++)
                {
                    nn::hid::StartSixAxisSensor(state.mSixAxisSensorHandles[j]);
                }

                for (s32 j = 0; j < state.mVibrationDeviceNum; j++)
                {
                    nn::hid::InitializeVibrationDevice(state.mVibrationDeviceHandles[j]);
                }
            }

            prev_style = style;
        }

        for (s32 j = 0; j < state.mSixAxisDeviceNum; j++)
        {
            nn::hid::GetSixAxisSensorStates(state.mSixAxisSensorStates[j].state, 16,
                                            state.mSixAxisSensorHandles[j]);
        }
    }
}

/**
 * Sets how many numbered npads are read each frame.
 * @param num number of npads, at most 8
 */
void NinJoyNpadDevice::setNpadIdUpdateNum(u32 num)
{
    if (num <= 8)
    {
        mNpadIdUpdateNum = num;
    }
}

/**
 * Sets the npad styles the application supports.
 * @param styleSet supported styles
 */
void NinJoyNpadDevice::setSupportedNpadStyleSet(nn::hid::NpadStyleSet styleSet)
{
    nn::hid::SetSupportedNpadStyleSet(styleSet);
}

/**
 * Sets how Joy-Con are held.
 * @param holdType hold type
 */
void NinJoyNpadDevice::setNpadJoyHoldType(nn::hid::NpadJoyHoldType holdType)
{
    mNpadJoyHoldType = holdType;
    nn::hid::SetNpadJoyHoldType(holdType);
}

/**
 * Gets the Joy-Con assignment mode of an npad.
 * @param port npad id
 * @return the assignment mode
 */
nn::hid::NpadJoyAssignmentMode NinJoyNpadDevice::getNpadJoyAssignment(s32 port)
{
    u32 npad_id = port;
    return nn::hid::GetNpadJoyAssignment(npad_id);
}

/**
 * Switches an npad to single Joy-Con assignment.
 * @param port npad id
 */
void NinJoyNpadDevice::setNpadJoyAssignmentModeSingle(s32 port)
{
    if (u32(port) < 8)
    {
        u32 npad_id = port;
        nn::hid::SetNpadJoyAssignmentModeSingle(npad_id);
    }
}

/**
 * Switches an npad to single Joy-Con assignment, keeping the given side.
 * @param port npad id
 * @param deviceType Joy-Con side to keep
 */
void NinJoyNpadDevice::setNpadJoyAssignmentModeSingle(s32 port,
                                                      nn::hid::NpadJoyDeviceType deviceType)
{
    if (u32(port) < 8)
    {
        u32 npad_id = port;
        nn::hid::SetNpadJoyAssignmentModeSingle(npad_id, deviceType);
    }
}

/**
 * Switches an npad to dual Joy-Con assignment.
 * @param port npad id
 */
void NinJoyNpadDevice::setNpadJoyAssignmentModeDual(s32 port)
{
    if (u32(port) < 8)
    {
        u32 npad_id = port;
        nn::hid::SetNpadJoyAssignmentModeDual(npad_id);
    }
}

/**
 * Merges two single Joy-Con npads into one dual npad.
 * @param port1 first npad id
 * @param port2 second npad id
 * @return the result of the merge, or an error for an invalid npad id
 */
nn::Result NinJoyNpadDevice::mergeSingleJoyAsDualJoy(s32 port1, s32 port2)
{
    if (u32(port1) >= 8 || u32(port2) >= 8)
    {
        return nn::result::detail::ErrorResultBase<202, 604>();
    }

    u32 npad_id1 = port1;
    u32 npad_id2 = port2;
    return nn::hid::MergeSingleJoyAsDualJoy(npad_id1, npad_id2);
}

/**
 * Swaps the controllers assigned to two npads.
 * @param port1 first npad id
 * @param port2 second npad id
 */
void NinJoyNpadDevice::swapNpadAssignment(s32 port1, s32 port2)
{
    if (u32(port1) < 8 && u32(port2) < 8)
    {
        u32 npad_id1 = port1;
        u32 npad_id2 = port2;
        nn::hid::SwapNpadAssignment(npad_id1, npad_id2);
    }
}

/**
 * Disconnects the controller of an npad.
 * @param port npad id
 */
void NinJoyNpadDevice::disconnectNpad(s32 port)
{
    if (u32(port) < 8)
    {
        u32 npad_id = port;
        nn::hid::DisconnectNpad(npad_id);
    }
}

void NinJoyNpadDevice::sendVibrationValue(s32 port, s32 deviceIdx,
                                          const nn::hid::VibrationValue& rValue)
{
    mVibrationThread.requestVibration(mNpadStates[port].mVibrationDeviceHandles[deviceIdx], rValue);
}

void NinJoyNpadDevice::VibrationThread::requestVibration(
    const nn::hid::VibrationDeviceHandle& rHandle, const nn::hid::VibrationValue& rValue)
{
    mCS.lock();

    Request* request = mRequests.emplaceBack();
    if (!request)
    {
        mCS.unlock();
        return;
    }

    request->handle = rHandle;
    request->value = rValue;
    mCS.unlock();
    sendMessage(1, MessageQueue::BlockType::NonBlocking);
}

/**
 * Creates the thread that sends queued vibration values.
 * @param pHeap heap for the thread
 */
NinJoyNpadDevice::VibrationThread::VibrationThread(Heap* pHeap)
    : Thread("VibrationThread", pHeap, ThreadUtil::ConvertPrioritySeadToPlatform(15),
             MessageQueue::BlockType::Blocking, 0x7fffffff, 0x1000, 0x20)
{
}

/**
 * Sends the oldest queued vibration value, if any.
 * @param msg received message
 */
void NinJoyNpadDevice::VibrationThread::calc_(MessageQueue::Element msg)
{
    mCS.lock();

    bool has_request = !mRequests.empty();
    Request request;
    if (has_request)
    {
        mRequests.popFront(&request);
    }

    mCS.unlock();

    if (has_request)
    {
        nn::hid::SendVibrationValue(request.handle, request.value);
    }
}

}  // namespace sead
