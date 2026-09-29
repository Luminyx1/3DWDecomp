#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.hpp>
#include <thread/seadMessageQueue.h>

namespace al {
class FileEntryBase {
public:
    FileEntryBase();

    virtual void load() = 0;

    void setFileName(const sead::SafeString&);
    const sead::SafeString& getFileName() const;
    void sendMessageDone();
    void waitLoadDone();
    void clear();
    void setLoadStateRequested();

    sead::FixedSafeString<0x40> mFileName;  // _8
    s32 mFileState = 0;                     // _60
    sead::MessageQueue mMessageQueue;       // _68
};

static_assert(sizeof(FileEntryBase) == 0xB8, "FileEntryBase size");
}  // namespace al
