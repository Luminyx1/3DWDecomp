#include "Project/SaveData/SaveDataSequenceRead.hpp"

#include <filedevice/nin/seadNinSaveFileDeviceNin.h>

namespace al {
/**
 * Sets up the buffer to read into.
 * @param pBuffer Destination buffer.
 * @param bufferSize Buffer size.
 * @param version Expected save data version.
 */
void SaveDataSequenceRead::start(u8* pBuffer, u32 bufferSize, u32 version) {
    mBuffer = pBuffer;
    mBufferSize = bufferSize;
    mVersion = version;
    mIsCorrupted = false;
}

/**
 * Opens the save file device; reading is stubbed out.
 * @param pFileName Save file name.
 * @return Always 0.
 */
s32 SaveDataSequenceRead::threadFunc(const char* pFileName) {
    sead::NinSaveFileDevice device("save");
    return 0;
}

/**
 * Stubbed out read.
 * @param pDevice File device.
 * @param pFileName Save file name.
 * @param pReadSize Read size output.
 * @return Always 0.
 */
s32 SaveDataSequenceRead::read(sead::FileDevice* pDevice, const char* pFileName, u32* pReadSize) {
    return 0;
}

/**
 * Constructs the save data sequence base.
 */
SaveDataSequenceBase::SaveDataSequenceBase() = default;
}  // namespace al
