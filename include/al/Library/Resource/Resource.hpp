#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.hpp>
#include <resource/seadArchiveRes.h>
#include "filedevice/seadArchiveFileDevice.h"

namespace nn::g3d {
class ResFile;
}

namespace al {
class Resource {
public:
    Resource(const sead::SafeString &);
    Resource(const sead::SafeString &, sead::ArchiveRes *);

    bool loadPatchData();
    bool isExistFile(const sead::SafeString &) const;
    bool isExistByml(const char *) const;
    u32 getSize() const;
    u32 getEntryNum(const sead::SafeString &) const;
    void getEntryName(sead::BufferedSafeString *, const sead::SafeString &, u32) const;
    u32 getFileSize(const sead::SafeString &) const;
    const u8* getByml(const sead::SafeString &) const;
    const void* getFile(const sead::SafeString &) const;
    const u8* tryGetByml(const sead::SafeString&) const;
    const void* getKcl(const sead::SafeString&) const;
    const void* tryGetKcl(const sead::SafeString&) const;
    const void* getPa(const sead::SafeString&) const;
    void* getOtherFile(const sead::SafeString&, u32*) const;
    const char* getArchiveName() const;
    bool tryCreateResGraphicsFile(const sead::SafeString&, nn::g3d::ResFile*);
    void cleanupResGraphicsFile();

    sead::ArchiveRes* getFileArchive() const { return mArchive; }
    sead::ArchiveFileDevice* getFileDevice() const { return mDevice; }
    const char* getPath() const { return mResName.cstr(); }
    nn::g3d::ResFile* getResFile() const { return mResFile; }

    sead::ArchiveRes* mArchive;                 // 0x00
    sead::ArchiveFileDevice* mDevice;           // 0x08
    sead::FixedSafeString<0x80> mResName;       // 0x10
    sead::Heap* mHeap;                          // 0xA8
    u64 _B0;
    Resource* mPatchRes;                        // 0xB8
    union {
        nn::g3d::ResFile* mResFile;                 // 0xC0
        u64 _C0;
    };
};
}  // namespace al
