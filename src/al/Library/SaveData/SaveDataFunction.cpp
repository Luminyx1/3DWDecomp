#include "Library/SaveData/SaveDataFunction.hpp"

#include <codec/seadHashCRC32.h>

namespace al {
    /**
     * @brief Gets the header of a save data buffer.
     * @param pData The save data buffer.
     * @return The header at the start of pData.
     */
    SaveDataHeader* SaveDataFunction::getSaveDataHeader(u8* pData) {
        return reinterpret_cast<SaveDataHeader*>(pData);
    }

    /**
     * @brief Gets the header of a read-only save data buffer.
     * @param pData The save data buffer.
     * @return The header at the start of pData.
     */
    const SaveDataHeader* SaveDataFunction::getSaveDataHeader(const u8* pData) {
        return reinterpret_cast<const SaveDataHeader*>(pData);
    }

    /**
     * @brief Calculates the checksum of a save data buffer.
     * @param pData The save data buffer, starting with its header.
     * @return The CRC32 of everything after the checksum field.
     */
    u32 SaveDataFunction::calcSaveDataCheckSum(const u8* pData) {
        const SaveDataHeader* pHeader = getSaveDataHeader(pData);
        return sead::HashCRC32::calcHash(pData + sizeof(u32), pHeader->mSize - sizeof(u32));
    }

    /**
     * @brief Makes the result code for invalid save data.
     * @return The invalid result code (0).
     */
    s32 SaveDataFunction::makeInvalidResult() {
        return 0;
    }
};
