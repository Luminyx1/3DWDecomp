#pragma once

#include <resource/seadResourceMgr.h>

#include "Project/File/FileEntryBase.hpp"

namespace sead {
class ArchiveRes;
class FileDevice;
class Heap;
}  // namespace sead

namespace al {
class ArchiveEntry : public FileEntryBase {
public:
    ArchiveEntry();

    void load() override;
    void setLoadRequestInfo(const sead::SafeString& rFileName, sead::Heap* pHeap,
                            sead::FileDevice* pDevice);
    sead::ArchiveRes* getArchiveRes();
    void clear();

    sead::ResourceMgr::LoadArg mLoadArg;
    sead::ArchiveRes* mArchiveRes = nullptr;
};

static_assert(sizeof(ArchiveEntry) == 0x118);
}  // namespace al
