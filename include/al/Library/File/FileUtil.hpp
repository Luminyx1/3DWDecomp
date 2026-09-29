#pragma once

#include <resource/seadArchiveRes.h>

namespace al {
    sead::ArchiveRes* loadArchive(const sead::SafeString &);
    bool isExistArchive(const sead::SafeString &);
    bool isExistArchive(const sead::SafeString &, const char *);
};