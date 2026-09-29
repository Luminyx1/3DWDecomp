#pragma once

#include <nn/atk/atk_SoundSystem.h>

#include "audio/seadAudioSystem.h"
#include "container/seadOffsetList.h"
#include "hostio/seadHostIONode.h"
#include "thread/seadCriticalSection.h"

namespace sead {
class AudioTaskThreadNin;
class Heap;

namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio

class AudioSystemNin : public AudioSystem, public hostio::Node {
    SEAD_RTTI_OVERRIDE(AudioSystemNin, AudioSystem)

public:
    class AtkInitializeParam {
    public:
        AtkInitializeParam() = default;

        nn::atk::SoundSystem::SoundSystemParam* getSoundSystemParam();
        void setWorkMemory(u8* pWorkMemory, size_t size);
        u8* getWorkMemory() const;
        size_t getWorkMemorySize() const;

    private:
        friend class AudioSystemNin;

        u8* mWorkMemory = nullptr;
        size_t mWorkMemorySize = 0;
        nn::atk::SoundSystem::SoundSystemParam mSoundSystemParam;
    };

    AudioSystemNin();
    ~AudioSystemNin() override {}

    void initialize() override;
    void finalize() override;
    bool setOutputMode(AudioGlobal::OutputMode mode) override;
    AudioGlobal::OutputMode getOutputMode() const override;
    bool appendEffect(AudioGlobal::AuxBus bus, AudioFx* pFx) override;
    bool appendFxObject(AudioGlobal::AuxBus bus, AudioFxObject* pFxObject) override;
    void clearEffect(AudioGlobal::AuxBus bus, s32 fadeFrames) override;
    bool isFinishedClearEffect(AudioGlobal::AuxBus bus) override;
    void appendSoundFrameCallback(ISoundFrameCallback& rCallback) override;
    void removeSoundFrameCallback(ISoundFrameCallback& rCallback) override;
    void clearSoundFrameCallback() override;

    void setHeap(Heap* pHeap);
    void setThreadPriority(s32 soundThreadPriority, s32 taskThreadPriority);
    void setThreadStackSize(s32 soundThreadStackSize, s32 taskThreadStackSize);
    void setCommandBufferSize(s32 soundThreadSize, s32 taskThreadSize);
    void enableTaskThread(s32 priority);
    void setThreadCoreNumber(s32 soundThreadCore, s32 taskThreadCore);
    void setRenderSampleRate(s32 sampleRate);
    void setEffectCount(s32 count);
    void setVoiceCountMax(s32 count);
    void setEnableCompatibleDownMixSetting(bool enable);
    void setEnableCompatibleBusVolume(bool enable);
    void enableDbgTaskThread(s32 priority);
    void forceQuit();

    void genMessage(hostio::Context* pContext);
    void listenPropertyEvent(const hostio::PropertyEvent* pEvent);

    static s32 GetSamplesPerFrame();

    bool isAtkEnabled() const { return mIsAtkEnabled; }
    s32 getAddonArchiveCount() const { return mAddonArchiveCount; }

protected:
    virtual void initializeAtk_();

private:
    friend class AudioMgr;
    friend class AudioPlayerNin;
    friend class AudioSoundDataMgrNin;

    void initializeMain_();
    void finalizeMain_();
    void forceQuitMain_();
    nn::atk::AuxBus getAuxBusNw_(AudioGlobal::AuxBus bus, nn::atk::OutputDevice* pDevice) const;
    void soundFrameProc_();

    static void soundFrameCallback_(uintptr_t arg);

    AtkInitializeParam mAtkInitializeParam;
    Heap* mHeap = nullptr;
    u8* mWorkBuffer = nullptr;
    bool mIsInitialized = false;
    s32 mAddonArchiveCount = 0;
    OffsetList<ISoundFrameCallback> mSoundFrameCallbacks;
    CriticalSection mCriticalSection;
    AudioTaskThreadNin* mTaskThread = nullptr;
    bool mIsTaskThreadEnabled = false;
    s32 mTaskThreadPriority;
    bool mIsAtkEnabled = true;
};
static_assert(sizeof(AudioSystemNin) == 0x140);
}  // namespace sead
