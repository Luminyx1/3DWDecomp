#pragma once

#include "audio/seadAudioResourceLoader.h"
#include "prim/seadSafeString.h"

namespace sead {
class Heap;

class AudioResourceLoaderNin : public AudioResourceLoader {
    SEAD_RTTI_OVERRIDE(AudioResourceLoaderNin, AudioResourceLoader)

public:
    enum ArchiveType {
        cArchiveType_None = 0,
        cArchiveType_Fs = 1,
        cArchiveType_Memory = 2
    };

    AudioResourceLoaderNin();
    ~AudioResourceLoaderNin() override;

    void initialize(AudioMgr& rMgr) override;
    void load() override;
    void finalize() override;

    void setHeap(Heap* pHeap);
    void setStreamBufferSizeMergin(u32 mergin);
    void setStreamBufferSizeRate(f32 rate);
    void setStreamReadCacheSize(u32 size);
    void setUserParamSizePerSound(u32 size);
    void setSoundHeapSize(u32 size);
    void setArchiveOnFs(const SafeString& rPath);
    void setFileAccessInFunction(bool enable);
    void setLoadLabelString(bool load);
    void setFsContentRootPath(const SafeString& rPath);
    void setArchiveOnMemory(const void* pArchive);

private:
    ArchiveType mArchiveType = cArchiveType_None;
    Heap* mHeap = nullptr;
    u32 mSoundHeapSize = 0;
    u32 mStreamBufferSizeMergin = 0;
    f32 mStreamBufferSizeRate = 1.0f;
    u32 mStreamReadCacheSize = 0;
    u32 mUserParamSizePerSound = 0;
    bool mIsLoadLabelString = true;
    SafeString mArchivePath;
    SafeString mFsContentRootPath;
    const void* mArchiveOnMemory = nullptr;
    AudioMgr* mAudioMgr = nullptr;
    bool mIsFileAccessInFunction = false;
};
static_assert(sizeof(AudioResourceLoaderNin) == 0x68);
}  // namespace sead
