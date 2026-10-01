#pragma once

#include <container/seadBuffer.h>
#include <nn/nfp/nfp.h>
#include <nn/nfp/nfp_types.h>
#include <nn/os.h>
#include <nn/types.h>
#include <prim/seadEnum.h>
#include <thread/seadCriticalSection.h>
#include <thread/seadMessageQueue.h>

namespace sead {
class DelegateThread;
class Thread;
}  // namespace sead

namespace al {
struct NfpInfo;

SEAD_ENUM(NfpControllerCommand, None, AttachEvent, StartDetection, StopDetection, TagDetect, Mount,
          TagRead, Deactive, Sleep)

SEAD_ENUM(NfpControllerState, None, Idle, Detecting, Activated, Mounted, TagRead, Sleep, Invalid)

class NfpControllerThread {
public:
    using CommandBuffer = sead::Buffer<NfpControllerCommand>;

    NfpControllerThread(s32 priority);
    virtual ~NfpControllerThread();

    void executeCommand(sead::Thread* pThread, sead::MessageQueue::Element message);
    void setDeviceHandleAndNpadId(const nn::nfp::DeviceHandle& rHandle, u32 npadId);
    void enqueueCommand(NfpControllerCommand command);
    void start();
    void stop();
    void clearCommand();
    void executeCommandAttachEvent();
    void executeCommandStartDetection();
    void executeCommandStopDetection();
    void executeCommandTagDetect();
    void executeCommandMount();
    void executeCommandTagRead();
    void executeCommandDeactive();
    void executeCommandSleep();
    bool handleError(const nn::Result& rResult, bool isShowError);
    void invalidate(const nn::Result& rResult, bool isShowError);
    bool isInvalid();
    bool isConnected();
    void setNfpInfo(NfpInfo* pInfo);
    void resetNfpInfo(NfpInfo* pInfo);
    void quitThread();

    bool isDetectionFailed() const { return mIsDetectionFailed; }

    void resetDetectionFailed() { mIsDetectionFailed = false; }

private:
    bool readTagInfo() {
        nn::Result result = nn::nfp::GetTagInfo(&mTagInfo, mDeviceHandle);

        if (handleError(result, true)) {
            return false;
        }

        result = nn::nfp::GetModelInfo(&mModelInfo, mDeviceHandle);
        return !handleError(result, true);
    }

    NfpControllerState getState() {
        mCriticalSection.lock();
        NfpControllerState state = mState;
        mCriticalSection.unlock();
        return state;
    }

    void setState(NfpControllerState state) {
        mCriticalSection.lock();
        mState = state;
        mCriticalSection.unlock();
    }

    sead::DelegateThread* mThread = nullptr;
    sead::CriticalSection mCriticalSection;
    nn::nfp::DeviceHandle mDeviceHandle = {};
    u32 mNpadId = 0;
    NfpControllerState mState = NfpControllerState::None;
    nn::os::SystemEventType* mActivateEvent = nullptr;
    nn::os::SystemEventType* mDeactivateEvent = nullptr;
    CommandBuffer mCommands;
    CommandBuffer mExecCommands;
    s32 mCommandNum = 0;
    nn::nfp::TagInfo mTagInfo = {};
    nn::nfp::ModelInfo mModelInfo = {};
    bool mIsTagRead = false;
    bool mIsDetectionFailed = false;
    nn::Result mResult;
    bool mIsRunning = false;
    bool mIsUnavailable = false;
    bool mIsShowError;
};

static_assert(sizeof(NfpControllerThread) == 0x138);
}  // namespace al
