#include "Library/Nfp/NfpControllerThread.hpp"

#include <mc/seadCoreInfo.h>
#include <nn/err.h>
#include <nn/nfp/nfp.h>
#include <nn/nfp/nfp_result.h>
#include <prim/seadDelegate.h>
#include <thread/seadDelegateThread.h>
#include <time/seadTickSpan.h>

#include "Library/Nfp/NfpTypes.hpp"

namespace al {
/**
 * Allocates an uninitialized system event.
 * @return the new system event
 */
static nn::os::SystemEventType* createSystemEvent() {
    nn::os::SystemEventType* pEvent = new nn::os::SystemEventType;
    pEvent->state = nn::os::SystemEventType::State_NotInitialized;
    return pEvent;
}

/**
 * Destroys and frees a system event if it exists.
 * @param ppEvent pointer to the event pointer, cleared afterwards
 */
static void deleteSystemEvent(nn::os::SystemEventType** ppEvent) {
    nn::os::SystemEventType* pEvent = *ppEvent;

    if (pEvent == nullptr) {
        return;
    }

    if (pEvent->state != nn::os::SystemEventType::State_NotInitialized) {
        nn::os::DestroySystemEvent(pEvent);
    }

    delete pEvent;
    *ppEvent = nullptr;
}

/**
 * Allocates the command queues and events and starts the controller thread.
 * @param priority thread priority
 */
NfpControllerThread::NfpControllerThread(s32 priority) {
    mCommands.tryAllocBuffer(5, nullptr);
    mExecCommands.tryAllocBuffer(5, nullptr);

    for (s32 i = 0; i < 5; i++) {
        mCommands[i] = NfpControllerCommand::None;
        mExecCommands[i] = NfpControllerCommand::None;
    }

    mActivateEvent = createSystemEvent();
    mDeactivateEvent = createSystemEvent();

    mThread = new sead::DelegateThread(
        "NFPスレッド(Controller)",
        new sead::Delegate2<NfpControllerThread, sead::Thread*, sead::MessageQueue::Element>(
            this, &NfpControllerThread::executeCommand),
        nullptr, priority, sead::MessageQueue::BlockType::NonBlocking, 0x7fffffff, 0x2000, 0x20);
    mThread->start();
    mThread->setAffinity(sead::CoreIdMask(sead::CoreId::cSub2));
}

void NfpControllerThread::executeCommand(sead::Thread* pThread,
                                         sead::MessageQueue::Element message) {
    sead::TickSpan span = sead::TickSpan::makeFromMilliSeconds(60);

    while (true) {
        mCriticalSection.lock();

        for (s32 i = 0; i < mCommandNum; i++) {
            mExecCommands[i] = mCommands[i];
            mCommands[i] = NfpControllerCommand::None;
        }

        s32 commandNum = mCommandNum;
        mCommandNum = 0;
        mCriticalSection.unlock();

        for (s32 i = 0; i < commandNum; i++) {
            mCriticalSection.lock();
            NfpControllerState state = mState;
            mCriticalSection.unlock();

            if (s32(state) != NfpControllerState::Invalid) {
                switch (mExecCommands[i]) {
                case NfpControllerCommand::AttachEvent:
                    executeCommandAttachEvent();
                    break;
                case NfpControllerCommand::StartDetection:
                    executeCommandStartDetection();
                    break;
                case NfpControllerCommand::StopDetection:
                    executeCommandStopDetection();
                    break;
                case NfpControllerCommand::TagDetect:
                    executeCommandTagDetect();
                    break;
                case NfpControllerCommand::Mount:
                    executeCommandMount();
                    break;
                case NfpControllerCommand::TagRead:
                    executeCommandTagRead();
                    break;
                case NfpControllerCommand::Deactive:
                    executeCommandDeactive();
                    break;
                case NfpControllerCommand::Sleep:
                    executeCommandSleep();
                    break;
                default:
                    break;
                }
            }

            mExecCommands[i] = NfpControllerCommand::None;
        }

        sead::Thread::sleep(span);
    }
}

/**
 * Destroys the thread and the system events.
 */
NfpControllerThread::~NfpControllerThread() {
    if (mThread != nullptr) {
        mThread->destroy();

        if (mThread != nullptr) {
            delete mThread;
            mThread = nullptr;
        }
    }

    deleteSystemEvent(&mActivateEvent);
    deleteSystemEvent(&mDeactivateEvent);
}

/**
 * Sets the NFP device, resets the state and queues the event attachment.
 * @param rHandle NFP device handle
 * @param npadId npad id of the controller
 */
void NfpControllerThread::setDeviceHandleAndNpadId(const nn::nfp::DeviceHandle& rHandle,
                                                   u32 npadId) {
    mCriticalSection.lock();
    mDeviceHandle = rHandle;
    mNpadId = npadId;
    mState = NfpControllerState::None;

    for (s32 i = 0; i < 5; i++) {
        mCommands[i] = NfpControllerCommand::None;
    }

    mCommandNum = 0;
    enqueueCommand(NfpControllerCommand::AttachEvent);
    mCriticalSection.unlock();
}

/**
 * Appends a command to the command queue.
 * @param command command to queue
 */
void NfpControllerThread::enqueueCommand(NfpControllerCommand command) {
    mCriticalSection.lock();
    mCommands[mCommandNum] = command;
    mCommandNum++;
    mCriticalSection.unlock();
}

/**
 * Starts tag detection.
 */
void NfpControllerThread::start() {
    mIsRunning = true;
    mIsUnavailable = false;
    mIsDetectionFailed = false;
    enqueueCommand(NfpControllerCommand::Sleep);
    enqueueCommand(NfpControllerCommand::StartDetection);
}

/**
 * Stops tag detection, or clears the invalid state.
 */
void NfpControllerThread::stop() {
    mCriticalSection.lock();
    NfpControllerState state = mState;

    if (state == NfpControllerState::Invalid) {
        mResult = nn::ResultSuccess();
        mState = NfpControllerState::Idle;
        mIsTagRead = false;
        clearCommand();
        mCriticalSection.unlock();
        return;
    }

    mIsRunning = false;
    mCriticalSection.unlock();
    clearCommand();

    switch (state.value()) {
    case NfpControllerState::Detecting:
    case NfpControllerState::Activated:
    case NfpControllerState::Mounted:
    case NfpControllerState::TagRead:
        enqueueCommand(NfpControllerCommand::StopDetection);
        enqueueCommand(NfpControllerCommand::Sleep);
        break;
    default:
        break;
    }
}

/**
 * Clears the command queue.
 */
void NfpControllerThread::clearCommand() {
    mCriticalSection.lock();
    mCommandNum = 0;
    mCriticalSection.unlock();
}

/**
 * Attaches the activate and deactivate events to the device.
 */
void NfpControllerThread::executeCommandAttachEvent() {
    if (mActivateEvent->state != nn::os::SystemEventType::State_NotInitialized) {
        nn::os::DestroySystemEvent(mActivateEvent);
    }

    if (mDeactivateEvent->state != nn::os::SystemEventType::State_NotInitialized) {
        nn::os::DestroySystemEvent(mDeactivateEvent);
    }

    nn::Result result = nn::nfp::AttachActivateEvent(mActivateEvent, mDeviceHandle);

    if (handleError(result, true)) {
        return;
    }

    result = nn::nfp::AttachDeactivateEvent(mDeactivateEvent, mDeviceHandle);

    if (handleError(result, true)) {
        return;
    }

    setState(NfpControllerState::Idle);
}

/**
 * Starts tag detection on the device.
 */
void NfpControllerThread::executeCommandStartDetection() {
    mCriticalSection.lock();
    NfpControllerState state = mState;
    mCriticalSection.unlock();

    if (s32(state) != NfpControllerState::None && s32(state) != NfpControllerState::Idle &&
        s32(state) != NfpControllerState::Sleep) {
        return;
    }

    nn::nfp::GetDeviceState(mDeviceHandle);

    if (handleError(nn::nfp::StartDetection(mDeviceHandle), false)) {
        mIsDetectionFailed = true;
        return;
    }

    setState(NfpControllerState::Detecting);

    if (mIsRunning) {
        enqueueCommand(NfpControllerCommand::TagDetect);
    } else {
        enqueueCommand(NfpControllerCommand::StopDetection);
    }
}

/**
 * Stops tag detection on the device.
 */
void NfpControllerThread::executeCommandStopDetection() {
    mCriticalSection.lock();
    mState = NfpControllerState::Idle;
    mCommandNum = 0;
    mCriticalSection.unlock();
    nn::nfp::StopDetection(mDeviceHandle);
    mResult = nn::ResultSuccess();
    mIsDetectionFailed = mIsUnavailable;
    mIsUnavailable = false;
}

/**
 * Waits for a tag to be detected.
 */
void NfpControllerThread::executeCommandTagDetect() {
    if (getState() == NfpControllerState::Idle) {
        return;
    }

    nn::nfp::DeviceState deviceState = nn::nfp::GetDeviceState(mDeviceHandle);

    if (deviceState == nn::nfp::DeviceState_Initialized) {
        enqueueCommand(NfpControllerCommand::Deactive);
    } else if (deviceState == nn::nfp::DeviceState_Unavailable) {
        mCriticalSection.lock();
        mIsUnavailable = true;
        enqueueCommand(NfpControllerCommand::StopDetection);
        enqueueCommand(NfpControllerCommand::Sleep);
        mCriticalSection.unlock();
    } else if (nn::os::TimedWaitSystemEvent(mActivateEvent,
                                            nn::TimeSpan::FromNanoSeconds(100000))) {
        setState(NfpControllerState::Activated);
        enqueueCommand(NfpControllerCommand::Mount);
    } else {
        enqueueCommand(NfpControllerCommand::TagDetect);
    }
}

void NfpControllerThread::executeCommandMount() {
    if (nn::nfp::GetDeviceState(mDeviceHandle) == nn::nfp::DeviceState_Initialized) {
        enqueueCommand(NfpControllerCommand::Deactive);
        setState(NfpControllerState::Idle);
        return;
    }

    if (nn::os::TimedWaitSystemEvent(mDeactivateEvent, nn::TimeSpan::FromNanoSeconds(100000))) {
        enqueueCommand(NfpControllerCommand::TagDetect);
        setState(NfpControllerState::Detecting);
    }

    nn::Result result;
    bool isMounted = false;

    for (s32 i = 0; i < 3; i++) {
        result = nn::nfp::Mount(mDeviceHandle, nn::nfp::ModelType_Amiibo, nn::nfp::MountTarget_Rom);

        if (nn::nfp::ResultNeedRetry::Includes(result)) {
            continue;
        }

        if (handleError(result, true)) {
            return;
        }

        if (result.IsSuccess() || nn::nfp::ResultNeedRestore::Includes(result) ||
            nn::nfp::ResultNeedFormat::Includes(result)) {
            isMounted = true;
            break;
        }
    }

    if (!isMounted) {
        enqueueCommand(NfpControllerCommand::StartDetection);
        return;
    }

    if (result.IsFailure() && !nn::nfp::ResultNeedRestore::Includes(result) &&
        !nn::nfp::ResultNeedFormat::Includes(result)) {
        nn::err::ShowError(result);
        return;
    }

    setState(NfpControllerState::Mounted);
    enqueueCommand(NfpControllerCommand::TagRead);
}

/**
 * Reads the tag and model info of the mounted tag.
 */
void NfpControllerThread::executeCommandTagRead() {
    if (nn::nfp::GetDeviceState(mDeviceHandle) == nn::nfp::DeviceState_Initialized) {
        enqueueCommand(NfpControllerCommand::Deactive);
        setState(NfpControllerState::Idle);
        return;
    }

    if (!readTagInfo()) {
        return;
    }

    enqueueCommand(NfpControllerCommand::Deactive);
    mCriticalSection.lock();
    mState = NfpControllerState::TagRead;
    mIsTagRead = true;
    mCriticalSection.unlock();
}

/**
 * Waits for the tag to be removed.
 */
void NfpControllerThread::executeCommandDeactive() {
    nn::nfp::DeviceState deviceState = nn::nfp::GetDeviceState(mDeviceHandle);

    if (deviceState == nn::nfp::DeviceState_TagFound) {
        enqueueCommand(NfpControllerCommand::Deactive);
    } else if (deviceState == nn::nfp::DeviceState_TagMounted) {
        nn::nfp::Unmount(mDeviceHandle);
        enqueueCommand(NfpControllerCommand::Deactive);
    } else if (getState() != NfpControllerState::Idle) {
        enqueueCommand(NfpControllerCommand::Sleep);
        enqueueCommand(NfpControllerCommand::StartDetection);
        setState(NfpControllerState::Idle);
    }
}

/**
 * Sleeps for 60 milliseconds.
 */
void NfpControllerThread::executeCommandSleep() {
    setState(NfpControllerState::Sleep);
    sead::Thread::sleep(sead::TickSpan::makeFromMilliSeconds(60));
}

bool NfpControllerThread::handleError(const nn::Result& rResult, bool isShowError) {
    if (rResult.IsFailure() && !nn::nfp::ResultNeedRestore::Includes(rResult) &&
        !nn::nfp::ResultNeedFormat::Includes(rResult)) {
        if (nn::nfp::GetDeviceState(mDeviceHandle) != nn::nfp::DeviceState_Initialized) {
            nn::nfp::StopDetection(mDeviceHandle);
        }
    }

    if (nn::nfp::ResultNeedRestart::Includes(rResult)) {
        enqueueCommand(NfpControllerCommand::Sleep);
        enqueueCommand(NfpControllerCommand::StartDetection);
        return true;
    }

    if (nn::nfp::ResultNfcDeviceNotFound::Includes(rResult)) {
        invalidate(rResult, isShowError);
        return true;
    }

    if (nn::nfp::ResultNfcDisabled::Includes(rResult)) {
        invalidate(rResult, true);
        return true;
    }

    if (nn::nfp::ResultNotSupported::Includes(rResult)) {
        invalidate(rResult, true);
        return true;
    }

    if (nn::nfp::ResultInvalidFormatVersion::Includes(rResult)) {
        invalidate(rResult, true);
        return true;
    }

    return false;
}

/**
 * Puts the controller into the invalid state.
 * @param rResult error result
 * @param isShowError whether the error should be shown
 */
void NfpControllerThread::invalidate(const nn::Result& rResult, bool isShowError) {
    mCriticalSection.lock();
    mState = NfpControllerState::Invalid;
    mIsShowError = isShowError;
    mResult = rResult;
    mCriticalSection.unlock();
}

/**
 * Checks whether the controller is in the invalid state.
 * @return whether the controller is invalid
 */
bool NfpControllerThread::isInvalid() {
    mCriticalSection.lock();
    bool isInvalid = mState == NfpControllerState::Invalid;
    mCriticalSection.unlock();
    return isInvalid;
}

/**
 * Checks whether the NFP device is available.
 * @return whether the device is connected
 */
bool NfpControllerThread::isConnected() {
    return nn::nfp::GetDeviceState(mDeviceHandle) != nn::nfp::DeviceState_Unavailable;
}

/**
 * Copies the detected tag or the error into an NFP info.
 * @param pInfo NFP info to write
 */
void NfpControllerThread::setNfpInfo(NfpInfo* pInfo) {
    mCriticalSection.lock();

    if (mState == NfpControllerState::Invalid) {
        pInfo->result = mResult;
        pInfo->isError = true;
        pInfo->_9e = false;
        pInfo->isAmiibo = false;
        pInfo->isShowError = mIsShowError;
    } else {
        pInfo->isError = false;
        pInfo->_9e = mIsTagRead;
        pInfo->tagInfo = mTagInfo;
        pInfo->modelInfo = mModelInfo;
        pInfo->isAmiibo = true;
        pInfo->isShowError = false;
        mIsTagRead = false;
    }

    mCriticalSection.unlock();
}

/**
 * Clears the detection flags of an NFP info.
 * @param pInfo NFP info to reset
 */
void NfpControllerThread::resetNfpInfo(NfpInfo* pInfo) {
    pInfo->isError = false;
    pInfo->_9e = false;
    pInfo->isAmiibo = false;
    mIsTagRead = false;
}

/**
 * Quits the controller thread.
 */
void NfpControllerThread::quitThread() {
    mThread->quit(false);
    mState = NfpControllerState::None;
}
}  // namespace al
