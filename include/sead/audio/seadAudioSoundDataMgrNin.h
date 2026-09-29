#pragma once

#include <nn/atk/atk_SoundArchive.h>
#include <nn/atk/atk_SoundDataManager.h>

#include "basis/seadTypes.h"
#include "prim/seadRuntimeTypeInfo.h"
#include "prim/seadSafeString.h"

namespace sead {
class AudioSoundHeapNin;
class Heap;

class AudioSoundArchiveBaseNin {
    SEAD_RTTI_BASE(AudioSoundArchiveBaseNin)

public:
    enum Type {
        cType_Fs = 0,
        cType_Memory = 1
    };

    explicit AudioSoundArchiveBaseNin(Type type) : mType(type) {}
    virtual ~AudioSoundArchiveBaseNin() {}

    virtual bool open(const void* pArchive) = 0;
    virtual void close() = 0;

    Type getType() const { return mType; }

private:
    Type mType;
};

class AudioFsSoundArchiveNin : public AudioSoundArchiveBaseNin, public nn::atk::FsSoundArchive {
    SEAD_RTTI_OVERRIDE(AudioFsSoundArchiveNin, AudioSoundArchiveBaseNin)

public:
    explicit AudioFsSoundArchiveNin(Heap* pHeap);
    ~AudioFsSoundArchiveNin() override;

    bool open(const void* pFileName) override;
    void close() override;

    void setLoadLabelString(bool load);
    void setContentRootPath(const char* pPath);
    void setFileAccessInFunction(bool enable);

private:
    Heap* mHeap;
    u8* mHeaderBuffer = nullptr;
    u8* mLabelStringBuffer = nullptr;
    bool mIsLoadLabelString = false;
    const char* mContentRootPath = "content:";
    bool mIsFileAccessInFunction = false;
};
static_assert(sizeof(AudioFsSoundArchiveNin) == 0x650);

class AudioMemorySoundArchiveNin : public AudioSoundArchiveBaseNin, public nn::atk::MemorySoundArchive {
    SEAD_RTTI_OVERRIDE(AudioMemorySoundArchiveNin, AudioSoundArchiveBaseNin)

public:
    explicit AudioMemorySoundArchiveNin(Heap* pHeap);
    ~AudioMemorySoundArchiveNin() override;

    bool open(const void* pArchive) override;
    void close() override;
};
static_assert(sizeof(AudioMemorySoundArchiveNin) == 0x300);

class AudioSoundDataMgrNin : public nn::atk::SoundDataManager {
public:
    AudioSoundDataMgrNin();
    ~AudioSoundDataMgrNin() override;

    void setContentRootPath(const char* pPath);
    void connectSoundHeap(AudioSoundHeapNin* pHeap);
    const nn::atk::SoundArchive* getSoundArchive() const;
    bool mountSoundArchiveFromFs(const SafeString& rPath, Heap* pHeap, bool fileAccessInFunction,
                                 bool loadLabelString);
    bool mountSoundArchiveFromMemory(const void* pArchive, Heap* pHeap);
    void unmountSoundArchive();
    bool loadData(u32 itemId, u32 loadFlag, u32 loadBlockSize, AudioSoundHeapNin* pHeap);
    bool loadData(const char* pItemName, u32 loadFlag, u32 loadBlockSize, AudioSoundHeapNin* pHeap);

private:
    bool isNwEnabled_() const;
    bool setupManager_(Heap* pHeap);
    bool tryGetDefaultSoundHeapAndCheckReady_(AudioSoundHeapNin** ppHeap) const;

    AudioSoundArchiveBaseNin* mSoundArchive = nullptr;
    u8* mWorkBuffer = nullptr;
    AudioSoundHeapNin* mDefaultSoundHeap = nullptr;
    const char* mContentRootPath = nullptr;
    bool mIsSetup = false;
};
static_assert(sizeof(AudioSoundDataMgrNin) == 0x268);
}  // namespace sead
