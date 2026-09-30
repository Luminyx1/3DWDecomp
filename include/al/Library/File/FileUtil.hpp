#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>
#include <resource/seadArchiveRes.h>

namespace sead {
class Heap;
}

namespace al {
class IAudioResourceLoader;
class Resource;

bool isExistFile(const sead::SafeString& rFileName);
bool isExistArchive(const sead::SafeString& rFileName);
bool isExistArchive(const sead::SafeString& rFileName, const char* pExt);
u32 getFileSize(const sead::SafeString& rFileName);
u32 calcFileAlignment(const sead::SafeString& rFileName);
u32 calcBufferSizeAlignment(const sead::SafeString& rFileName);
u8* loadFile(const sead::SafeString& rFileName, s32 alignment);
sead::ArchiveRes* loadArchive(const sead::SafeString& rFileName);
sead::ArchiveRes* loadArchiveWithExt(const sead::SafeString& rFileName, const char* pExt);
bool tryRequestLoadArchive(const sead::SafeString& rFileName, sead::Heap* pHeap);
void loadSoundItem(u32 itemId, u32 unk, IAudioResourceLoader* pLoader);
bool tryRequestLoadSoundItem(u32 itemId);
bool tryRequestPreLoadFile(const Resource* pResource, s32 id, sead::Heap* pHeap,
                           IAudioResourceLoader* pLoader);
bool tryRequestPreLoadFile(const Resource* pResource, const sead::SafeString& rFileName,
                           sead::Heap* pHeap, IAudioResourceLoader* pLoader);
void waitLoadDoneAllFile();
void clearFileLoaderEntry();
void makeLocalizedArchivePath(sead::BufferedSafeString* pOutPath,
                              const sead::SafeString& rFileName);
void makeLocalizedArchivePathByCountryCode(sead::BufferedSafeString* pOutPath,
                                           const sead::SafeString& rFileName);
void setFileLoaderThreadPriority(s32 priority);
}  // namespace al
