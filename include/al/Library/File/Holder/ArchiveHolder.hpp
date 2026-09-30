#pragma once

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
    ArchiveEntry* addNewLoadRequestEntry(const sead::SafeString& rFileName, sead::Heap* pHeap,
                                         sead::FileDevice* pDevice);
    ArchiveEntry* tryFindEntry(const sead::SafeString& rFileName);
    void waitLoadDoneAll();
    void clearEntry();

    sead::Buffer<ArchiveEntry> mArchiveEntries;
    s32 mSize = 0;
};
}  // namespace al
