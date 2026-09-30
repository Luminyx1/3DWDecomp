#pragma once

#include <basis/seadTypes.h>

namespace al::SaveDataFunction {
struct SaveDataHeader {
    u32 mCheckSum;
    u32 mVersion;
    u32 mSize;
};

SaveDataHeader* getSaveDataHeader(u8* pBuffer);
const SaveDataHeader* getSaveDataHeader(const u8* pBuffer);
u32 calcSaveDataCheckSum(const u8* pBuffer);
s32 makeInvalidResult();
}  // namespace al::SaveDataFunction
