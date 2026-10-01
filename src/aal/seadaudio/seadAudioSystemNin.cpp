#include "audio/seadAudioSystemNin.h"

#include <nn/atk/atk_HardwareManager.h>
#include <nn/atk/atk_SoundThread.h>

#include "audio/seadAudioFx.h"
#include "audio/seadAudioFxBaseNin.h"
#include "audio/seadAudioTaskThreadNin.h"
#include "prim/seadSafeString.h"

namespace sead {
/**
 * Constructs the Nintendo audio system with the default settings.
 */
AudioSystemNin::AudioSystemNin() : mTaskThreadPriority(Thread::cDefaultPriority + 1) {
    mSoundFrameCallbacks.initOffset(offsetof(ISoundFrameCallback, mListNode));
}

/**
 * Creates the task thread and initializes the Nintendo audio library.
 */
void AudioSystemNin::initialize() {
    if (mIsTaskThreadEnabled) {
        mTaskThread = new (mHeap) AudioTaskThreadNin(mTaskThreadPriority, mHeap,
                                                     SafeString("sead::AudioTaskThread"), 0x2000, 0x40);
    }

    if (mIsAtkEnabled) {
        nn::atk::SoundSystem::SoundSystemParam& param = mAtkInitializeParam.mSoundSystemParam;
        param.enableCompatibleDownMixSetting = false;
        param.enableCompatibleBusVolume = true;
        param.enableCircularBufferSinkBufferManagement = true;
        param.enableRecordingFinalOutputs = true;
        param.enableCircularBufferSink = true;
        param.soundThreadCoreNumber = 2;
        param.taskThreadCoreNumber = 2;
        size_t size = nn::atk::SoundSystem::GetRequiredMemSize(param);
        mWorkBuffer = new (mHeap, 0x40) u8[size];
        mAtkInitializeParam.setWorkMemory(mWorkBuffer, size);
    }

    initializeMain_();
    mIsInitialized = true;
}

/**
 * Gets the Nintendo sound system parameters.
 * @return Sound system parameters.
 */
nn::atk::SoundSystem::SoundSystemParam* AudioSystemNin::AtkInitializeParam::getSoundSystemParam() {
    return &mSoundSystemParam;
}

/**
 * Sets the work memory of the Nintendo sound system.
 * @param pWorkMemory Work memory.
 * @param size Size of the work memory.
 */
void AudioSystemNin::AtkInitializeParam::setWorkMemory(u8* pWorkMemory, size_t size) {
    mWorkMemory = pWorkMemory;
    mWorkMemorySize = size;
}

/**
 * Initializes the Nintendo audio library and starts the task thread.
 */
void AudioSystemNin::initializeMain_() {
    initializeAtk_();

    if (mTaskThread != nullptr) {
        mTaskThread->start();
    }
}

/**
 * Stops the task thread, finalizes the Nintendo audio library and frees the work memory.
 */
void AudioSystemNin::finalize() {
    if (!mIsInitialized) {
        return;
    }

    finalizeMain_();

    if (mWorkBuffer) {
        delete[] mWorkBuffer;
    }

    mWorkBuffer = nullptr;

    if (mTaskThread != nullptr) {
        delete mTaskThread;
        mTaskThread = nullptr;
    }

    mIsInitialized = false;
}

/**
 * Stops the task thread and finalizes the Nintendo audio library.
 */
void AudioSystemNin::finalizeMain_() {
    if (mTaskThread != nullptr) {
        mTaskThread->quitAndWaitDoneSingleThread(false);
    }

    if (mIsAtkEnabled) {
        nn::atk::SoundSystem::Finalize();
    }
}

/**
 * Sets the output mode of the main output device.
 * @param mode Output mode.
 * @return True if the mode was set.
 */
bool AudioSystemNin::setOutputMode(AudioGlobal::OutputMode mode) {
    if (mIsAtkEnabled) {
        nn::atk::OutputMode nwMode;

        switch (mode) {
        case AudioGlobal::cOutputMode_Stereo:
            nwMode = nn::atk::OutputMode_Stereo;
            break;
        case AudioGlobal::cOutputMode_Monaural:
            nwMode = nn::atk::OutputMode_Monaural;
            break;
        case AudioGlobal::cOutputMode_Surround:
            nwMode = nn::atk::OutputMode_Surround;
            break;
        default:
            return false;
        }

        nn::atk::detail::driver::HardwareManager::GetInstance().SetOutputMode(nwMode, nn::atk::OutputDevice_Main);
        return true;
    }

    return false;
}

/**
 * Gets the output mode of the main output device.
 * @return Output mode.
 */
AudioGlobal::OutputMode AudioSystemNin::getOutputMode() const {
    if (!mIsAtkEnabled) {
        return AudioGlobal::cOutputMode_Invalid;
    }

    switch (nn::atk::detail::driver::HardwareManager::GetInstance().GetOutputMode(nn::atk::OutputDevice_Main)) {
    case nn::atk::OutputMode_Monaural:
        return AudioGlobal::cOutputMode_Monaural;
    case nn::atk::OutputMode_Stereo:
        return AudioGlobal::cOutputMode_Stereo;
    case nn::atk::OutputMode_Surround:
        return AudioGlobal::cOutputMode_Surround;
    default:
        return AudioGlobal::cOutputMode_Invalid;
    }
}

/**
 * Appends an effect to an aux bus.
 * @param bus Aux bus.
 * @param pFx Effect.
 * @return True if the effect was appended.
 */
bool AudioSystemNin::appendEffect(AudioGlobal::AuxBus bus, AudioFx* pFx) {
    if (!mIsAtkEnabled) {
        return false;
    }

    nn::atk::OutputDevice device;
    nn::atk::AuxBus nwBus = getAuxBusNw_(bus, &device);
    DynamicCast<AudioFxNin>(pFx);
    AudioFxBaseNin* fx = static_cast<AudioFxNin*>(pFx)->getImpl()->getFxNin();
    fx->SetEnabled(true);
    return nn::atk::SoundSystem::AppendEffect(nwBus, fx, fx->getWorkBuffer(), fx->getWorkBufferSize(), device);
}

/**
 * Converts an aux bus to the Nintendo aux bus.
 * @param bus Aux bus.
 * @param pDevice Receives the output device.
 * @return Nintendo aux bus.
 */
nn::atk::AuxBus AudioSystemNin::getAuxBusNw_(AudioGlobal::AuxBus bus, nn::atk::OutputDevice* pDevice) const {
    switch (bus) {
    case AudioGlobal::cAuxBus_A:
        *pDevice = nn::atk::OutputDevice_Main;
        return nn::atk::AuxBus_A;
    case AudioGlobal::cAuxBus_B:
        *pDevice = nn::atk::OutputDevice_Main;
        return nn::atk::AuxBus_B;
    case AudioGlobal::cAuxBus_C:
        *pDevice = nn::atk::OutputDevice_Main;
        return nn::atk::AuxBus_C;
    default:
        return nn::atk::AuxBus_A;
    }
}

/**
 * Appends an effect object to an aux bus.
 * @param bus Aux bus.
 * @param pFxObject Effect object.
 * @return True if the effect was appended.
 */
bool AudioSystemNin::appendFxObject(AudioGlobal::AuxBus bus, AudioFxObject* pFxObject) {
    if (!mIsAtkEnabled) {
        return false;
    }

    nn::atk::OutputDevice device;
    nn::atk::AuxBus nwBus = getAuxBusNw_(bus, &device);
    AudioFxBaseNin* fx = pFxObject->getImpl()->getFxNin();
    fx->SetEnabled(true);
    return nn::atk::SoundSystem::AppendEffect(nwBus, fx, fx->getWorkBuffer(), fx->getWorkBufferSize(), device);
}

/**
 * Removes every effect from an aux bus.
 * @param bus Aux bus.
 * @param fadeFrames Fade length in frames (unused).
 */
void AudioSystemNin::clearEffect(AudioGlobal::AuxBus bus, s32 fadeFrames) {
    if (!mIsAtkEnabled) {
        return;
    }

    nn::atk::OutputDevice device;
    nn::atk::AuxBus nwBus = getAuxBusNw_(bus, &device);
    nn::atk::SoundSystem::ClearEffect(nwBus, device);
}

/**
 * Checks whether the effects of an aux bus were removed.
 * @param bus Aux bus.
 * @return True if the effects were removed.
 */
bool AudioSystemNin::isFinishedClearEffect(AudioGlobal::AuxBus bus) {
    if (!mIsAtkEnabled) {
        return true;
    }

    nn::atk::OutputDevice device;
    nn::atk::AuxBus nwBus = getAuxBusNw_(bus, &device);
    return nn::atk::SoundSystem::IsClearEffectFinished(nwBus, device);
}

/**
 * Sets the heap used for the work memory.
 * @param pHeap Heap.
 */
void AudioSystemNin::setHeap(Heap* pHeap) {
    mHeap = pHeap;
}

/**
 * Sets the priorities of the sound and task threads.
 * @param soundThreadPriority Sound thread priority.
 * @param taskThreadPriority Task thread priority.
 */
void AudioSystemNin::setThreadPriority(s32 soundThreadPriority, s32 taskThreadPriority) {
    mAtkInitializeParam.mSoundSystemParam.soundThreadPriority = soundThreadPriority;
    mAtkInitializeParam.mSoundSystemParam.taskThreadPriority = taskThreadPriority;
}

/**
 * Sets the stack sizes of the sound and task threads.
 * @param soundThreadStackSize Sound thread stack size.
 * @param taskThreadStackSize Task thread stack size.
 */
void AudioSystemNin::setThreadStackSize(s32 soundThreadStackSize, s32 taskThreadStackSize) {
    mAtkInitializeParam.mSoundSystemParam.soundThreadStackSize = soundThreadStackSize;
    mAtkInitializeParam.mSoundSystemParam.taskThreadStackSize = taskThreadStackSize;
}

/**
 * Sets the command buffer sizes of the sound and task threads.
 * @param soundThreadSize Sound thread command buffer size.
 * @param taskThreadSize Task thread command buffer size.
 */
void AudioSystemNin::setCommandBufferSize(s32 soundThreadSize, s32 taskThreadSize) {
    mAtkInitializeParam.mSoundSystemParam.soundThreadCommandBufferSize = soundThreadSize;
    mAtkInitializeParam.mSoundSystemParam.taskThreadCommandBufferSize = taskThreadSize;
}

/**
 * Enables the audio task thread.
 * @param priority Task thread priority.
 */
void AudioSystemNin::enableTaskThread(s32 priority) {
    mIsTaskThreadEnabled = true;
    mTaskThreadPriority = priority;
}

/**
 * Sets the cores of the sound and task threads.
 * @param soundThreadCore Sound thread core number.
 * @param taskThreadCore Task thread core number.
 */
void AudioSystemNin::setThreadCoreNumber(s32 soundThreadCore, s32 taskThreadCore) {
    mAtkInitializeParam.mSoundSystemParam.soundThreadCoreNumber = soundThreadCore;
    mAtkInitializeParam.mSoundSystemParam.taskThreadCoreNumber = taskThreadCore;
}

/**
 * Sets the rendering sample rate.
 * @param sampleRate Sample rate.
 */
void AudioSystemNin::setRenderSampleRate(s32 sampleRate) {
    mAtkInitializeParam.mSoundSystemParam.rendererSampleRate = sampleRate;
}

/**
 * Sets the number of effects.
 * @param count Number of effects.
 */
void AudioSystemNin::setEffectCount(s32 count) {
    mAtkInitializeParam.mSoundSystemParam.effectCount = count;
}

/**
 * Sets the maximum number of voices.
 * @param count Maximum number of voices.
 */
void AudioSystemNin::setVoiceCountMax(s32 count) {
    mAtkInitializeParam.mSoundSystemParam.voiceCountMax = count;
}

/**
 * Sets whether the compatible down mix setting is used.
 * @param enable Whether to use it.
 */
void AudioSystemNin::setEnableCompatibleDownMixSetting(bool enable) {
    mAtkInitializeParam.mSoundSystemParam.enableCompatibleDownMixSetting = enable;
}

/**
 * Sets whether the compatible bus volume is used.
 * @param enable Whether to use it.
 */
void AudioSystemNin::setEnableCompatibleBusVolume(bool enable) {
    mAtkInitializeParam.mSoundSystemParam.enableCompatibleBusVolume = enable;
}

/**
 * Enables the debug task thread (unsupported, does nothing).
 * @param priority Thread priority.
 */
void AudioSystemNin::enableDbgTaskThread(s32 priority) {}

/**
 * Marks the system as finalized without finalizing the audio library.
 */
void AudioSystemNin::forceQuit() {
    if (mIsInitialized) {
        forceQuitMain_();
        mIsInitialized = false;
    }
}

/**
 * Stops the audio library for a forced quit (does nothing).
 */
void AudioSystemNin::forceQuitMain_() {}

/**
 * Initializes the Nintendo sound system.
 */
void AudioSystemNin::initializeAtk_() {
    if (!mIsAtkEnabled) {
        return;
    }

    uintptr_t workMemory = reinterpret_cast<uintptr_t>(mAtkInitializeParam.getWorkMemory());
    size_t workMemorySize = mAtkInitializeParam.getWorkMemorySize();
    nn::atk::SoundSystem::Initialize(*mAtkInitializeParam.getSoundSystemParam(), workMemory, workMemorySize);
}

/**
 * Gets the work memory of the Nintendo sound system.
 * @return Work memory.
 */
u8* AudioSystemNin::AtkInitializeParam::getWorkMemory() const {
    return mWorkMemory;
}

/**
 * Gets the size of the work memory of the Nintendo sound system.
 * @return Size of the work memory.
 */
size_t AudioSystemNin::AtkInitializeParam::getWorkMemorySize() const {
    return mWorkMemorySize;
}

/**
 * Registers a callback that is called every sound frame.
 * @param rCallback Callback.
 */
void AudioSystemNin::appendSoundFrameCallback(ISoundFrameCallback& rCallback) {
    if (!mIsAtkEnabled) {
        return;
    }

    if (mSoundFrameCallbacks.isEmpty()) {
        nn::atk::detail::driver::SoundThread::GetInstance().RegisterSoundFrameUserCallback(
            soundFrameCallback_, reinterpret_cast<uintptr_t>(this));
    }

    mCriticalSection.lock();

    if (mSoundFrameCallbacks.indexOf(&rCallback) == -1) {
        mSoundFrameCallbacks.pushBack(&rCallback);
    }

    mCriticalSection.unlock();
}

/**
 * Calls every sound frame callback.
 * @param arg Audio system.
 */
void AudioSystemNin::soundFrameCallback_(uintptr_t arg) {
    reinterpret_cast<AudioSystemNin*>(arg)->soundFrameProc_();
}

/**
 * Unregisters a sound frame callback.
 * @param rCallback Callback.
 */
void AudioSystemNin::removeSoundFrameCallback(ISoundFrameCallback& rCallback) {
    if (!mIsAtkEnabled) {
        return;
    }

    mCriticalSection.lock();

    if (mSoundFrameCallbacks.indexOf(&rCallback) >= 0) {
        mSoundFrameCallbacks.erase(&rCallback);
    }

    mCriticalSection.unlock();

    if (mSoundFrameCallbacks.isEmpty()) {
        nn::atk::detail::driver::SoundThread::GetInstance().ClearSoundFrameUserCallback();
    }
}

/**
 * Unregisters every sound frame callback.
 */
void AudioSystemNin::clearSoundFrameCallback() {
    if (!mIsAtkEnabled) {
        return;
    }

    mCriticalSection.lock();
    mSoundFrameCallbacks.clear();
    mCriticalSection.unlock();
    nn::atk::detail::driver::SoundThread::GetInstance().ClearSoundFrameUserCallback();
}

/**
 * Calls every sound frame callback.
 */
void AudioSystemNin::soundFrameProc_() {
    mCriticalSection.lock();

    if (!mSoundFrameCallbacks.isEmpty()) {
        for (auto it = mSoundFrameCallbacks.begin(); it != mSoundFrameCallbacks.end(); ++it) {
            it->onSoundFrame();
        }
    }

    mCriticalSection.unlock();
}

/**
 * Gets the number of samples per audio frame.
 * @return Always 0.
 */
s32 AudioSystemNin::GetSamplesPerFrame() {
    return 0;
}

/**
 * Generates the host IO message (stripped in release builds).
 * @param pContext Host IO context.
 */
void AudioSystemNin::genMessage(hostio::Context* pContext) {}

/**
 * Handles a host IO property event (no-op in release builds).
 * @param pEvent Property event.
 */
void AudioSystemNin::listenPropertyEvent(const hostio::PropertyEvent* pEvent) {}
}  // namespace sead
