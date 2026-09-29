#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <prim/seadSafeString.h>

namespace sead {
class FileDevice;
class Heap;
}  // namespace sead

namespace al {
class ArchiveEntry;

class ArchiveHolder {
public:
    ArchiveHolder();

    ArchiveEntry* addNewLoadRequestEntry(const sead::SafeString&, sead::Heap*, sead::FileDevice*);
    ArchiveEntry* tryFindEntry(const sead::SafeString&);
    void waitLoadDoneAll();
    void clearEntry();

    sead::Buffer<ArchiveEntry> mEntries;  // _0
    s32 mNumEntries = 0;                  // _10
};
}  // namespace al
