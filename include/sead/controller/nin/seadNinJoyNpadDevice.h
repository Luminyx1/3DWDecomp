#pragma once

#include <nn/hid.h>

#include "container/seadRingBuffer.h"
#include "container/seadSafeArray.h"
#include "controller/seadControlDevice.h"
#include "controller/seadController.h"
#include "heap/seadHeap.h"
#include "thread/seadCriticalSection.h"
#include "thread/seadThread.h"

namespace sead
{
class NinJoyNpadDevice : public ControlDevice
{
    SEAD_RTTI_OVERRIDE(NinJoyNpadDevice, ControlDevice);

public:
    class VibrationThread : public Thread
    {
    public:
        explicit VibrationThread(Heap* pHeap);
        ~VibrationThread() override = default;

        void calc_(MessageQueue::Element msg) override;
        void requestVibration(const nn::hid::VibrationDeviceHandle& rHandle,
                              const nn::hid::VibrationValue& rValue);

    private:
        struct Request
        {
            nn::hid::VibrationDeviceHandle handle;
            nn::hid::VibrationValue value;
        };

        FixedRingBuffer<Request, 16> mRequests;
        CriticalSection mCS;
    };

#if SEAD_HOSTIO_NONVIRTUAL
    static_assert(sizeof(VibrationThread) == 0x290);
#else
    static_assert(sizeof(VibrationThread) == 0x298);
#endif

    struct SixAxisState
    {
        nn::hid::SixAxisSensorState state[16];
    };
    static_assert(sizeof(SixAxisState) == 0x600);

    struct NpadState
    {
        SafeArray<nn::hid::NpadBaseState, 16> mStates;
        s32 mSixAxisDeviceNum;
        s32 mVibrationDeviceNum;
        SafeArray<nn::hid::SixAxisSensorHandle, 2> mSixAxisSensorHandles;
        SafeArray<SixAxisState, 2> mSixAxisSensorStates;
        SafeArray<nn::hid::VibrationDeviceHandle, 2> mVibrationDeviceHandles;
    };

    static_assert(sizeof(NpadState) == 0xe98);

    NinJoyNpadDevice(ControllerMgr* pMgr, Heap* pHeap);
    ~NinJoyNpadDevice() override;

    void calc() override;
    void setNpadIdUpdateNum(u32 num);
    void setSupportedNpadStyleSet(nn::hid::NpadStyleSet styleSet);
    void setNpadJoyHoldType(nn::hid::NpadJoyHoldType holdType);
    nn::hid::NpadJoyAssignmentMode getNpadJoyAssignment(s32 port);
    void setNpadJoyAssignmentModeSingle(s32 port);
    void setNpadJoyAssignmentModeSingle(s32 port, nn::hid::NpadJoyDeviceType deviceType);
    void setNpadJoyAssignmentModeDual(s32 port);
    nn::Result mergeSingleJoyAsDualJoy(s32 port1, s32 port2);
    void swapNpadAssignment(s32 port1, s32 port2);
    void disconnectNpad(s32 port);
    void sendVibrationValue(s32 port, s32 deviceIdx, const nn::hid::VibrationValue& rValue);

    nn::hid::NpadJoyHoldType getNpadJoyHoldType() const { return mNpadJoyHoldType; }

    const NpadState& getNpadState(s32 idx) { return mNpadStates[idx]; }

private:
    u32 mNpadIdUpdateNum;
    nn::hid::NpadJoyHoldType mNpadJoyHoldType;
    SafeArray<nn::hid::NpadStyleTag, 9> mNpadStyleTags;
    SafeArray<NpadState, 9> mNpadStates;
    VibrationThread mVibrationThread;
};

}  // namespace sead
