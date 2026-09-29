#pragma once

#include <basis/seadTypes.h>

namespace sead {
class DelegateThread;
class Thread;
}  // namespace sead

namespace al {
class FileEntryBase;

class FileLoaderThread {
public:
    FileLoaderThread(s32);

    void threadFunction(sead::Thread*, s64);
    void requestLoadFile(FileEntryBase*);

    sead::DelegateThread* mThread = nullptr;  // _0
};
}  // namespace al
