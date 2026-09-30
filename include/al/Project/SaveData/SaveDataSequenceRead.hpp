#pragma once

#include "Project/SaveData/SaveDataSequenceBase.hpp"

namespace sead {
class FileDevice;
}

namespace al {
class SaveDataSequenceRead : public SaveDataSequenceBase {
public:
    SaveDataSequenceRead(u8 unk);

    s32 threadFunc(const char* pFileName) override;

    void start(u8* pBuffer, u32 bufferSize, u32 version);
    s32 read(sead::FileDevice* pDevice, const char* pFileName, u32* pReadSize);

    bool isCorrupted() const { return mIsCorrupted; }

    u8* mBuffer = nullptr;
    u32 mBufferSize = 0;
    u32 mVersion = 0;
    bool mIsCorrupted = false;
    u8 _19 = 0;
};

static_assert(sizeof(SaveDataSequenceRead) == 0x20);
}  // namespace al
