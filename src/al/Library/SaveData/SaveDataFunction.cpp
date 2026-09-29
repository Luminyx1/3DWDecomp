#include "Library/SaveData/SaveDataFunction.hpp"

#include <codec/seadHashCRC32.h>

namespace al {
    /** @brief Returns the header at the start of a save data buffer. */
    SaveDataHeader* SaveDataFunction::getSaveDataHeader(u8* pData) {
        return reinterpret_cast<SaveDataHeader*>(pData);
    }

    /** @brief Returns the header at the start of a save data buffer. */
    const SaveDataHeader* SaveDataFunction::getSaveDataHeader(const u8* pData) {
        return reinterpret_cast<const SaveDataHeader*>(pData);
    }

    /** @brief Calculates the CRC32 of everything after the checksum field. */
    u32 SaveDataFunction::calcSaveDataCheckSum(const u8* pData) {
        const SaveDataHeader* pHeader = getSaveDataHeader(pData);
        return sead::HashCRC32::calcHash(pData + sizeof(u32), pHeader->mSize - sizeof(u32));
    }

    /** @brief Returns the result code for invalid save data. */
    s32 SaveDataFunction::makeInvalidResult() {
        return 0;
    }
};
