#pragma once

#include <basis/seadTypes.h>

#include "Library/File/FileEntryBase.hpp"

namespace sead {
class FileDevice;
class Heap;
}  // namespace sead

namespace al {
class ArchiveEntry : public FileEntryBase {
public:
    ArchiveEntry();

    void load() override;
    void setLoadRequestInfo(const sead::SafeString&, sead::Heap*, sead::FileDevice*);
    void clear();

    u8 _B8[0x118 - 0xB8];
};

static_assert(sizeof(ArchiveEntry) == 0x118, "ArchiveEntry size");
}  // namespace al
