#pragma once

#include "Project/SaveData/SaveDataSequenceBase.hpp"

namespace sead {
class FileDevice;
}

namespace al {
class SaveDataSequenceWrite : public SaveDataSequenceBase {
public:
    SaveDataSequenceWrite(u8 unk);

    s32 threadFunc(const char* pFileName) override;

    void start(u8* pBuffer, u32 bufferSize, u32 version, bool isFlushNeeded);
    void startFlushOnly();
    bool write(sead::FileDevice* pDevice, const char* pFileName);

    u8* mBuffer = nullptr;
    u32 mBufferSize = 0;
    bool mIsWriteNeeded = false;
    bool mIsFlushNeeded = false;
    u8 _16 = 0;
};

static_assert(sizeof(SaveDataSequenceWrite) == 0x18);
}  // namespace al
