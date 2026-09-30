#pragma once

#include <prim/seadSafeString.h>
#include <thread/seadCriticalSection.h>

namespace sead {
class ArchiveRes;
class FileDevice;
class Heap;
}  // namespace sead

namespace al {
class ArchiveEntry;
class ArchiveHolder;
class ByamlIter;
class FileLoaderThread;
class IAudioResourceLoader;
class SoundItemEntry;
class SoundItemHolder;

class FileLoader {
public:
    FileLoader(s32 threadPriority, bool unk);

    bool isExistFile(const sead::SafeString& rPath, sead::FileDevice* pDevice) const;
    sead::FileDevice* getFileDevice(const sead::SafeString& rPath, sead::FileDevice* pDevice) const;
    bool isExistArchive(const sead::SafeString& rPath, sead::FileDevice* pDevice) const;
    u32 getFileSize(const sead::SafeString& rPath, sead::FileDevice* pDevice) const;
    u8* loadFile(const sead::SafeString& rPath, s32 alignment, sead::FileDevice* pDevice);
    sead::ArchiveRes* loadArchive(const sead::SafeString& rPath, sead::FileDevice* pDevice);
    sead::ArchiveRes* loadArchiveWithExt(const sead::SafeString& rPath, const char* pExt,
                                         sead::FileDevice* pDevice);
    ArchiveEntry* requestLoadArchive(const sead::SafeString& rPath, sead::Heap* pHeap,
                                     sead::FileDevice* pDevice);
    bool tryRequestLoadArchive(const sead::SafeString& rPath, sead::Heap* pHeap,
                               sead::FileDevice* pDevice);
    bool loadSoundItem(u32 itemId, u32 unk, IAudioResourceLoader* pLoader);
    SoundItemEntry* requestLoadSoundItem(u32 itemId, u32 unk, IAudioResourceLoader* pLoader);
    bool tryRequestLoadSoundItem(u32 itemId, IAudioResourceLoader* pLoader);
    void requestPreLoadFile(const ByamlIter& rPreLoadList, sead::Heap* pHeap,
                            IAudioResourceLoader* pLoader);
    void waitLoadDoneAllFile();
    void clearAllEntry();
    void setThreadPriority(s32 priority);

    FileLoaderThread* mLoaderThread = nullptr;
    ArchiveHolder* mArchiveHolder = nullptr;
    SoundItemHolder* mSoundItemHolder = nullptr;
    bool mIsFilePreloaded = false;
    sead::FileDevice* mFileDevice = nullptr;
    sead::CriticalSection mCriticalSection;
};

static_assert(sizeof(FileLoader) == 0x68);
}  // namespace al
