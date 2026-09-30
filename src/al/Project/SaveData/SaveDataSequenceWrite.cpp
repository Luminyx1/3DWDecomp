#include "Project/SaveData/SaveDataSequenceWrite.hpp"

#include <filedevice/nin/seadNinSaveFileDeviceNin.h>

#include "Project/SaveData/SaveDataFunction.hpp"
#include "Project/SaveData/SaveDataSequenceFormat.hpp"

namespace al {
/**
 * Constructs the write sequence.
 * @param unk Unknown flag.
 */
SaveDataSequenceWrite::SaveDataSequenceWrite(u8 unk) : _16(unk) {}

/**
 * Sets up a write and fills in the save data header.
 * @param pBuffer Save data buffer.
 * @param bufferSize Buffer size.
 * @param version Save data version.
 * @param isFlushNeeded Whether to commit after writing.
 */
void SaveDataSequenceWrite::start(u8* pBuffer, u32 bufferSize, u32 version, bool isFlushNeeded) {
    mIsWriteNeeded = true;
    mBuffer = pBuffer;
    mBufferSize = bufferSize;
    mIsFlushNeeded = isFlushNeeded;

    SaveDataFunction::SaveDataHeader* header = SaveDataFunction::getSaveDataHeader(pBuffer);
    header->mVersion = version;
    header->mSize = bufferSize;
    header->mCheckSum = SaveDataFunction::calcSaveDataCheckSum(pBuffer);
}

/**
 * Sets up a flush without writing.
 */
void SaveDataSequenceWrite::startFlushOnly() {
    mBuffer = nullptr;
    mBufferSize = 0;
    mIsWriteNeeded = false;
    mIsFlushNeeded = true;
}

/**
 * Opens the save file device; writing is stubbed out.
 * @param pFileName Save file name.
 * @return Always 0.
 */
s32 SaveDataSequenceWrite::threadFunc(const char* pFileName) {
    sead::NinSaveFileDevice device("save");
    return 0;
}

/**
 * Stubbed out write.
 * @param pDevice File device.
 * @param pFileName Save file name.
 * @return Always true.
 */
bool SaveDataSequenceWrite::write(sead::FileDevice* pDevice, const char* pFileName) {
    return true;
}

/**
 * Constructs the format sequence.
 */
SaveDataSequenceFormat::SaveDataSequenceFormat() = default;
}  // namespace al
