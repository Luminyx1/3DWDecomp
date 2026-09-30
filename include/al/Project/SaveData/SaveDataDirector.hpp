#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <prim/seadSafeString.h>

namespace al {
class SaveDataSequenceBase;
class SaveDataSequenceInitDir;
class SaveDataSequenceFormat;
class SaveDataSequenceRead;
class SaveDataSequenceWrite;
class AsyncFunctorThread;

class SaveDataDirector {
public:
    SaveDataDirector(u32 workBufferSize, s32 threadPriority);

    void threadFunc();
    void initCheckSaveData();
    bool requestInitSaveDir(const char* pFileName, u32 dirSize, u32 version);
    bool initSaveDirSync(const char* pFileName, u32 dirSize, u32 version);
    bool requestFormat(s32 a, s32 b);
    bool formatSync(s32 a, s32 b);
    bool requestRead(const char* pFileName, u32 readSize, u32 version);
    bool readSync(const char* pFileName, u32 readSize, u32 version);
    bool requestWrite(const char* pFileName, u32 writeSize, u32 version, bool isFlushNeeded);
    bool requestFlush();
    bool writeSync(const char* pFileName, u32 writeSize, u32 version);
    bool updateSequence();
    bool isDoneSequence() const;
    u8* getWorkBuffer();
    s32 getResult();

    SaveDataSequenceRead* getReadSequence() const { return mReadSequence; }
    bool isInitialized() const { return mIsInitialized; }
    s32 getFSErrorCode() const { return mFileSystemErrorCode; }

    SaveDataSequenceBase* mRunningSequence = nullptr;
    SaveDataSequenceInitDir* mInitDirSequence = nullptr;
    SaveDataSequenceFormat* mFormatSequence = nullptr;
    SaveDataSequenceRead* mReadSequence = nullptr;
    SaveDataSequenceWrite* mWriteSequence = nullptr;
    bool mIsInitialized = false;
    bool _29 = false;
    sead::Buffer<u8> mBuffer;
    sead::FixedSafeString<64> mCurrentFileName;
    AsyncFunctorThread* mSaveDataThread = nullptr;
    s32 mResult = 0;
    s32 mFileSystemErrorCode = 0;
};

static_assert(sizeof(SaveDataDirector) == 0xa8);
}  // namespace al
