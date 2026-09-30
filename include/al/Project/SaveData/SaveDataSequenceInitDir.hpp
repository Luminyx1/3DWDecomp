#pragma once

#include "Project/SaveData/SaveDataSequenceBase.hpp"

namespace al {
class SaveDataSequenceInitDir : public SaveDataSequenceBase {
public:
    SaveDataSequenceInitDir(u8 unk);

    s32 threadFunc(const char* pFileName) override;

    void start(u8* pBuffer, u32 bufferSize, u32 version);

    u8* mBuffer = nullptr;
    u32 mBufferSize = 0;
    u32 mVersion = 0;
    u8 _18;
};

static_assert(sizeof(SaveDataSequenceInitDir) == 0x20);
}  // namespace al
