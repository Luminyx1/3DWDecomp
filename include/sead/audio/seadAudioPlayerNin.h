#pragma once

#include <nn/atk/atk_SoundArchivePlayer.h>
#include <nn/audio.h>

#include "audio/seadAudioPlayer.h"
#include "container/seadPtrArray.h"
#include "hostio/seadHostIONode.h"
#include "thread/seadCriticalSection.h"

namespace sead {
class AudioSoundDataMgrNin;
class AudioSoundHeapNin;
class Heap;

namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio

class SoundMemoryPoolHandler {
public:
    explicit SoundMemoryPoolHandler(const char* pName);

    static u32 calcRequireBufferSizeFromFileSize(s32 fileSize);

private:
    const char* mName;
    void* _8 = nullptr;
    void* _10 = nullptr;
    s32 _18 = 0;
    void* _20 = nullptr;
    nn::audio::MemoryPoolType* mMemoryPool;
    bool _30;
};

class AudioPlayerNin : public AudioPlayer, public nn::atk::SoundArchivePlayer, public hostio::Node {
    SEAD_RTTI_OVERRIDE(AudioPlayerNin, AudioPlayer)

public:
    struct DataManagementSetupParam {
        f32 mStreamBufferSizeRate;
        u32 mStreamReadCacheSize;
        u32 mUserParamSizePerSound;
        Heap* mHeap;
        u32 mAddonArchiveCount;
    };

    AudioPlayerNin();
    ~AudioPlayerNin() override;

    void initialize() override;
    void finalize() override;
    void calc() override;
    bool startSound(SoundHandle* pHandle, u32 soundId) override;
    bool startSound(SoundHandle* pHandle, const char* pSoundName) override;
    bool holdSound(SoundHandle* pHandle, u32 soundId) override;
    bool holdSound(SoundHandle* pHandle, const char* pSoundName) override;
    u32 getSoundCount() const override;
    const char* getSoundName(u32 soundId) const override;
    u32 getSoundId(const char* pSoundName) const override;

    StartResult detail_SetupSound(nn::atk::SoundHandle* pHandle, u32 soundId, bool holdFlag,
                                  const char* pSoundArchiveName, const StartInfo* pStartInfo) override;

    bool trySetSoundMemoryPoolHandler(SoundMemoryPoolHandler* pHandler);
    void stopAll(s32 fadeFrames);
    void pauseAll(s32 fadeFrames);
    void unpauseAll(s32 fadeFrames);
    u32 getTotalSoundCount() const;
    const char* getAddonSoundName(u32 soundId) const;
    const char* getAddonArchiveName(s32 index) const;
    u32 getAddonSoundId(u32 soundId) const;
    bool areAddonArchivesAdded() const;
    void createSoundHeap(size_t size, Heap* pHeap);
    void destroySoundHeap();
    bool setupDataManagement(u32 streamBufferMargin, u32 streamReadCacheSize, u32 userParamSizePerSound,
                             Heap* pHeap, u32 addonArchiveCount);
    bool setupDataManagement(const DataManagementSetupParam& rParam);
    void shutdownDataManagement();
    void genMessage(hostio::Context* pContext);
    void listenPropertyEvent(const hostio::PropertyEvent* pEvent);

    AudioSoundDataMgrNin* getSoundDataMgr() const { return mSoundDataMgr; }

private:
    bool isAtkEnabled_() const;
    void setPauseAll_(s32 fadeFrames, bool pause);
    bool setupDataManagementInner_(const nn::atk::SoundArchive& rArchive, u32 streamBufferSize,
                                   u32 streamReadCacheSize, u32 userParamSizePerSound, Heap* pHeap,
                                   u32 addonArchiveCount);

    PtrArray<SoundMemoryPoolHandler>* mMemoryPoolHandlers = nullptr;
    u8* mSetupBuffer = nullptr;
    u32 mSetupBufferSize = 0;
    u8* mStreamBuffer = nullptr;
    u32 mStreamBufferSize = 0;
    nn::audio::MemoryPoolType* mStreamMemoryPool = nullptr;
    u32 mRequiredStreamBufferSize = 0;
    u8* mStreamCacheBuffer = nullptr;
    u32 mStreamCacheBufferSize = 0;
    AudioSoundDataMgrNin* mSoundDataMgr = nullptr;
    AudioSoundHeapNin* mSoundHeap = nullptr;
    bool mIsPaused = false;
    bool mIsStartDisabled = false;
    CriticalSection mCriticalSection;
    bool mIsUsingCriticalSection = false;
    u32 mAddonArchiveCount = 0;
};
static_assert(sizeof(AudioPlayerNin) == 0x3c8);
}  // namespace sead
