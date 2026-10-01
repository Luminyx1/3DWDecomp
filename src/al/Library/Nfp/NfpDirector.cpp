#include "Library/Nfp/NfpDirector.hpp"

#include <cstdio>
#include <nn/err.h>
#include <nn/nfp.h>
#include <nn/nfp/nfp_result.h>
#include <nn/time.h>
#include <prim/seadDelegate.h>
#include <thread/seadDelegateThread.h>
#include <time/seadTickSpan.h>

#include "Library/Nfp/NfpControllerThread.hpp"
#include "Library/Nfp/NfpTypes.hpp"

namespace {
s32 sThreadPriority = sead::Thread::cDefaultPriority;
s32 sControllerThreadPriority = sead::Thread::cDefaultPriority;
}  // namespace

namespace al {
/**
 * Allocates the command queues, NFP infos and controller threads and starts the director thread.
 * @param maxDeviceNum maximum number of NFP devices
 */
NfpDirector::NfpDirector(s32 maxDeviceNum) : mMaxDeviceNum(maxDeviceNum) {
    mCommands.tryAllocBuffer(cCommandBufferSize, nullptr);
    mExecCommands.tryAllocBuffer(cCommandBufferSize, nullptr);

    for (s32 i = 0; i < cCommandBufferSize; i++) {
        mCommands[i] = NfpApplicationCommand::None;
        mExecCommands[i] = NfpApplicationCommand::None;
    }

    mDeviceInfos.tryAllocBuffer(cDeviceNumMax, nullptr);

    for (s32 i = 0; i < cDeviceNumMax; i++) {
        mDeviceInfos[i].npadId = 0;
    }

    mNfpInfos.allocBuffer(maxDeviceNum, nullptr);

    for (s32 i = 0; i < maxDeviceNum; i++) {
        mNfpInfos.pushBack(new NfpInfo());
    }

    mThread = new sead::DelegateThread(
        "NFPスレッド(Master)",
        new sead::Delegate2<NfpDirector, sead::Thread*, s64>(this, &NfpDirector::executeCommand),
        nullptr, sThreadPriority, sead::MessageQueue::BlockType::NonBlocking, 0x7fffffff, 0x2000,
        0x20);
    mControllerThreads.allocBuffer(maxDeviceNum, nullptr);

    for (s32 i = 0; i < maxDeviceNum; i++) {
        mControllerThreads.pushBack(new NfpControllerThread(sControllerThreadPriority));
    }

    mIsActiveDevice.tryAllocBuffer(cDeviceNumMax, nullptr);

    for (s32 i = 0; i < cDeviceNumMax; i++) {
        mIsActiveDevice[i] = false;
    }

    mThread->start();
}

/**
 * Thread function that executes the queued commands every 60 milliseconds.
 * @param pThread unused
 * @param message unused
 */
void NfpDirector::executeCommand(sead::Thread* pThread, s64 message) {
    while (true) {
        mCriticalSection.lock();

        for (s32 i = 0; i < mCommandNum; i++) {
            mExecCommands[i] = mCommands[i];
            mCommands[i] = NfpApplicationCommand::None;
        }

        s32 commandNum = mCommandNum;
        mCommandNum = 0;
        mCriticalSection.unlock();

        for (s32 i = 0; i < commandNum; i++) {
            switch (mExecCommands[i]) {
            case NfpApplicationCommand::Initialize:
                executeCommandInitialize();
                break;
            case NfpApplicationCommand::ListDevices:
                executeCommandListDevices();
                break;
            case NfpApplicationCommand::Start:
                executeCommandStart();
                break;
            case NfpApplicationCommand::DelayStart:
                executeCommandDelayStart();
                break;
            case NfpApplicationCommand::Stop:
                executeCommandStop();
                break;
            case NfpApplicationCommand::Finalize:
                executeCommandFinalize();
                break;
            default:
                break;
            }

            mExecCommands[i] = NfpApplicationCommand::None;
        }

        sead::Thread::sleep(sead::TickSpan::makeFromMilliSeconds(60));
    }
}

/**
 * Destroys the director thread and the controller threads.
 */
NfpDirector::~NfpDirector() {
    if (mThread != nullptr) {
        mThread->destroy();

        if (mThread != nullptr) {
            delete mThread;
            mThread = nullptr;
        }
    }

    s32 size = mControllerThreads.size();

    for (s32 i = 0; i < size; i++) {
        delete mControllerThreads[i];
    }
}

/**
 * Queues the initialize command.
 */
void NfpDirector::initialize() {
    enqueueCommand(NfpApplicationCommand::Initialize);
}

/**
 * Adds a command to the command queue.
 * @param command command to add
 */
void NfpDirector::enqueueCommand(NfpApplicationCommand command) {
    mCriticalSection.lock();
    mCommands[mCommandNum] = command;
    mCommandNum++;
    mCriticalSection.unlock();
}

/**
 * Queues the finalize command.
 */
void NfpDirector::finalize() {
    enqueueCommand(NfpApplicationCommand::Finalize);
}

void NfpDirector::update() {
    mCriticalSection.lock();
    s32 state = mState;
    s32 num = mDeviceNum < mMaxDeviceNum ? mDeviceNum : mMaxDeviceNum;
    mActiveDeviceNum = num;
    mUpdateErrorInfo = mErrorInfo;
    mCriticalSection.unlock();

    bool isNeedRestart = false;

    for (s32 i = 0; i < num; i++) {
        if (mIsActiveDevice[i]) {
            NfpControllerThread* controller = mControllerThreads.unsafeAt(i);
            isNeedRestart |= controller->isDetectionFailed();
            controller->resetDetectionFailed();
        }
    }

    if (state == NfpDirectorState_Started &&
        nn::os::TryWaitSystemEvent(&mAvailabilityChangeEvent)) {
        for (s32 i = 0; i < num; i++) {
            if (mIsActiveDevice[i]) {
                isNeedRestart |= !mControllerThreads[i]->isConnected();
            }
        }

        if (!isNeedRestart) {
            nn::nfp::DeviceHandle handles[cDeviceNumMax];
            s32 total = 0;

            if (nn::nfp::ListDevices(handles, &total, cDeviceNumMax).IsSuccess() &&
                total != num) {
                isNeedRestart = true;
            }
        }
    }

    if (isNeedRestart) {
        mIsRestart = true;
        stop(false);
        return;
    }

    if (state == NfpDirectorState_Stopped) {
        if (mUpdateErrorInfo.isError) {
            resetError();
        }

        for (s32 i = 0; i < num; i++) {
            if (mIsActiveDevice[i]) {
                mNfpInfos.unsafeAt(i)->resetError();
            }
        }

        return;
    }

    for (s32 i = 0; i < num; i++) {
        if (mIsActiveDevice[i]) {
            mControllerThreads[i]->setNfpInfo(mNfpInfos[i]);
        }
    }

    if (mUpdateErrorInfo.isError) {
        showError(mUpdateErrorInfo.result);
        return;
    }

    for (s32 i = 0; i < num; i++) {
        if (mIsActiveDevice[i] && mNfpInfos.unsafeAt(i)->isError) {
            if (mNfpInfos.unsafeAt(i)->result.IsSuccess()) {
                mNfpInfos.unsafeAt(i)->isError = false;
                return;
            }

            if (mNfpInfos.unsafeAt(i)->isShowError) {
                showError(mNfpInfos[i]->result);
                return;
            }

            mHadError = true;
            resetError();
            stop(false);
            return;
        }
    }
}

/**
 * Clears the command queue and stops the controllers if they were started.
 * @param isClearRestart whether to cancel a pending restart
 */
void NfpDirector::stop(bool isClearRestart) {
    mCriticalSection.lock();

    if (mState != NfpDirectorState_Stopped) {
        clearCommand();

        if (isClearRestart) {
            mIsRestart = false;
        }

        if (mState == NfpDirectorState_Started) {
            enqueueCommand(NfpApplicationCommand::Stop);
        }

        mState = NfpDirectorState_Stopped;
    }

    mCriticalSection.unlock();
}

/**
 * Clears the director error and the errors of all NFP infos.
 */
void NfpDirector::resetError() {
    mCriticalSection.lock();
    mErrorInfo.isError = false;
    mUpdateErrorInfo.isError = false;
    s32 num = mMaxDeviceNum < mDeviceNum ? mMaxDeviceNum : mDeviceNum;

    for (s32 i = 0; i < num; i++) {
        mNfpInfos.unsafeAt(i)->resetError();
    }

    mCriticalSection.unlock();
}

/**
 * Queues the list devices command.
 */
void NfpDirector::listDevices() {
    enqueueCommand(NfpApplicationCommand::ListDevices);
}

/**
 * Resets the NFP infos and queues the list devices and start commands.
 */
void NfpDirector::start() {
    if (mState == NfpDirectorState_Started) {
        return;
    }

    clearCommand();
    s32 size = mControllerThreads.size();

    for (s32 i = 0; i < size; i++) {
        mControllerThreads[i]->resetNfpInfo(mNfpInfos[i]);
    }

    enqueueCommand(NfpApplicationCommand::ListDevices);
    enqueueCommand(NfpApplicationCommand::Start);
}

/**
 * Clears the command queue.
 */
void NfpDirector::clearCommand() {
    mCriticalSection.lock();
    mCommandNum = 0;
    mCriticalSection.unlock();
}

/**
 * Checks and clears whether an error occurred.
 * @return whether an error occurred
 */
bool NfpDirector::hadError() {
    bool hadError = mHadError;
    mHadError = false;
    return hadError;
}

/**
 * Sets the npad id whose NFP device is used.
 * @param npadId npad id
 */
void NfpDirector::setRequestNpadId(s32 npadId) {
    mRequestNpadId = npadId;
}

/**
 * Shows the system error dialog, resets the errors and stops.
 * @param rResult error result
 */
void NfpDirector::showError(const nn::Result& rResult) {
    mHadError = true;
    nn::err::ShowError(rResult);
    resetError();
    stop(false);
}

/**
 * Initializes the NFP library and attaches the availability change event.
 */
void NfpDirector::executeCommandInitialize() {
    setState(NfpDirectorState_Initializing);
    nn::nfp::Initialize();
    nn::nfp::AttachAvailabilityChangeEvent(&mAvailabilityChangeEvent);
    setState(NfpDirectorState_Initialized);
}

/**
 * Finalizes the NFP library.
 */
void NfpDirector::executeCommandFinalize() {
    nn::nfp::Finalize();
    setState(NfpDirectorState_None);
}

/**
 * Lists the NFP devices and assigns them to the controller threads.
 */
void NfpDirector::executeCommandListDevices() {
    setState(NfpDirectorState_ListingDevices);
    nn::nfp::DeviceHandle handles[cDeviceNumMax];
    s32 total = 0;
    u32 npadIds[cDeviceNumMax];
    nn::Result result;

    for (s32 i = 0; i < 3; i++) {
        printf("List devices: try %i\n", i);
        result = nn::nfp::ListDevices(handles, &total, cDeviceNumMax);

        if (result.IsSuccess()) {
            break;
        }

        if (nn::nfp::ResultNfcDisabled::Includes(result)) {
            mCriticalSection.lock();
            mErrorInfo.isError = true;
            mErrorInfo.result = result;
            mDeviceNum = 0;
            mCriticalSection.unlock();
            return;
        }

        nn::os::SleepThread(nn::TimeSpan::FromMilliSeconds(1100));
    }

    if (nn::nfp::ResultNfcDeviceNotFound::Includes(result)) {
        mErrorInfo.isError = true;
        mErrorInfo.result = result;
        mDeviceNum = 0;
        return;
    }

    for (s32 i = 0; i < mMaxDeviceNum; i++) {
        mIsActiveDevice[i] = false;
    }

    for (s32 i = 0; i < total; i++) {
        if (nn::nfp::GetNpadId(&npadIds[i], handles[i]).IsFailure()) {
            setState(NfpDirectorState_Initialized);

            if (mIsRetryListDevices) {
                enqueueCommand(NfpApplicationCommand::ListDevices);
            }

            return;
        }
    }

    bool isFound = false;

    for (s32 i = 0; i < total; i++) {
        if (npadIds[i] == static_cast<u32>(mRequestNpadId)) {
            isFound = true;
            break;
        }
    }

    if (!isFound) {
        mErrorInfo.isError = true;
        mErrorInfo.result = nn::nfp::ResultNfcDeviceNotFound();
        mDeviceNum = 0;
        return;
    }

    for (s32 i = 0; i < mMaxDeviceNum; i++) {
        mDeviceInfos[i].npadId = 0;
    }

    mCriticalSection.lock();
    mState = NfpDirectorState_Started;
    mDeviceNum = total < mMaxDeviceNum ? total : mMaxDeviceNum;

    for (s32 i = 0; i < mDeviceNum; i++) {
        mDeviceInfos[i].handle = handles[i];
        mDeviceInfos[i].npadId = npadIds[i];

        if (mDeviceInfos[i].npadId == static_cast<u32>(mRequestNpadId)) {
            mIsActiveDevice[i] = true;
        }

        mControllerThreads[i]->setDeviceHandleAndNpadId(handles[i], npadIds[i]);
    }

    mCriticalSection.unlock();
}

/**
 * Starts the controller threads of the active devices.
 */
void NfpDirector::executeCommandStart() {
    mCriticalSection.lock();
    s32 num = mDeviceNum < mMaxDeviceNum ? mDeviceNum : mMaxDeviceNum;

    for (s32 i = 0; i < num && i < mMaxDeviceNum; i++) {
        if (mIsActiveDevice[i]) {
            mControllerThreads[i]->start();
        }
    }

    mCriticalSection.unlock();
}

/**
 * Stops the controller threads of the active devices and restarts if requested.
 */
void NfpDirector::executeCommandStop() {
    mCriticalSection.lock();
    s32 num = mDeviceNum;
    mCriticalSection.unlock();

    for (s32 i = 0; i < num && i < mMaxDeviceNum; i++) {
        if (mIsActiveDevice[i]) {
            mControllerThreads[i]->stop();
        }
    }

    for (s32 i = 0; i < mDeviceNum; i++) {
        mIsActiveDevice[i] = false;
    }

    if (mIsRestart) {
        mIsRestart = false;
        start();
    }
}

/**
 * Counts down the start delay and starts when it reaches zero.
 */
void NfpDirector::executeCommandDelayStart() {
    mCriticalSection.lock();

    if (mDelayStartCount == 0) {
        start();
    } else {
        mDelayStartCount--;
        enqueueCommand(NfpApplicationCommand::DelayStart);
    }

    mCriticalSection.unlock();
}

/**
 * Resets the NFP infos and starts after a delay.
 * @param delayCount number of delay commands to wait
 */
void NfpDirector::startDelayed(s32 delayCount) {
    s32 size = mControllerThreads.size();

    for (s32 i = 0; i < size; i++) {
        mControllerThreads[i]->resetNfpInfo(mNfpInfos[i]);
    }

    if (delayCount < 1) {
        start();
        return;
    }

    mDelayStartCount = delayCount;
    enqueueCommand(NfpApplicationCommand::DelayStart);
}
}  // namespace al
