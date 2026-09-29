#pragma once

#include <basis/seadTypes.h>

namespace sead {
class FileDevice;
}

namespace al {
class SaveDataSequenceBase {
public:
    SaveDataSequenceBase();

    virtual s32 threadFunc(const char*) = 0;
};

class SaveDataSequenceRead : public SaveDataSequenceBase {
public:
    SaveDataSequenceRead(u8);

    void start(u8*, u32, u32);
    s32 threadFunc(const char*) override;
    bool read(sead::FileDevice*, const char*, u32*);

    u8* mBuffer = nullptr;  // _8
    u32 mBufferSize = 0;    // _10
    u32 _14 = 0;
    bool mIsCorrupted = false;  // _18
    u8 _19;
};

class SaveDataSequenceWrite : public SaveDataSequenceBase {
public:
    SaveDataSequenceWrite(u8);

    void start(u8*, u32, u32, bool);
    void startFlushOnly();
    s32 threadFunc(const char*) override;
    bool write(sead::FileDevice*, const char*);

    u8* mBuffer = nullptr;  // _8
    u32 mBufferSize = 0;    // _10
    bool mIsWrite = false;  // _14
    bool mIsFlush = false;  // _15
    u8 _16;
};

class SaveDataSequenceFormat : public SaveDataSequenceBase {
public:
    SaveDataSequenceFormat();

    void start(s32, s32);
    s32 threadFunc(const char*) override;

    s32 _8 = 0;
    s32 _c = 0;
};

class SaveDataSequenceInitDir : public SaveDataSequenceBase {
public:
    SaveDataSequenceInitDir(u8);

    void start(u8*, u32, u32);
    s32 threadFunc(const char*) override;

    u8* mBuffer = nullptr;  // _8
    u32 mBufferSize = 0;    // _10
    u32 _14 = 0;
    u8 _18;
};
}  // namespace al
