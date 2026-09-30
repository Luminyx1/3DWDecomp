#include "Project/SaveData/SaveDataFunction.hpp"

#include <codec/seadHashCRC32.h>

namespace al::SaveDataFunction {
/**
 * Gets the header at the start of a save data buffer.
 * @param pBuffer Save data buffer.
 * @return The header.
 */
SaveDataHeader* getSaveDataHeader(u8* pBuffer) {
    return reinterpret_cast<SaveDataHeader*>(pBuffer);
}

/**
 * Gets the header at the start of a save data buffer.
 * @param pBuffer Save data buffer.
 * @return The header.
 */
const SaveDataHeader* getSaveDataHeader(const u8* pBuffer) {
    return reinterpret_cast<const SaveDataHeader*>(pBuffer);
}

/**
 * Calculates the CRC32 of a save data buffer, excluding the checksum field.
 * @param pBuffer Save data buffer.
 * @return The checksum.
 */
u32 calcSaveDataCheckSum(const u8* pBuffer) {
    const SaveDataHeader* header = getSaveDataHeader(pBuffer);
    return sead::HashCRC32::calcHash(pBuffer + sizeof(u32), header->mSize - sizeof(u32));
}

/**
 * Makes the result value used for a sequence that has not finished.
 * @return The invalid result.
 */
s32 makeInvalidResult() {
    return 0;
}
}  // namespace al::SaveDataFunction
