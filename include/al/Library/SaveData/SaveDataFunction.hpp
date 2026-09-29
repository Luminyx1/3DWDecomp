#pragma once

#include <basis/seadTypes.h>

namespace al {
    /// Header at the start of every save data buffer.
    struct SaveDataHeader {
        u32 mCheckSum;      // _0
        u32 _4;
        u32 mSize;          // _8
    };

    class SaveDataFunction {
    public:
        static SaveDataHeader* getSaveDataHeader(u8* pData);
        static const SaveDataHeader* getSaveDataHeader(const u8* pData);
        static u32 calcSaveDataCheckSum(const u8* pData);
        static s32 makeInvalidResult();
    };
};
