#include "Library/SaveData/SaveDataSequence.hpp"

#include <filedevice/nin/seadNinSaveFileDeviceNin.h>

namespace al {
    /**
     * @brief Starts reading save data into a buffer.
     * @param pBuffer The buffer to read into.
     * @param bufferSize The size of the buffer.
     * @param unk Unknown value stored at _14.
     */
    void SaveDataSequenceRead::start(u8* pBuffer, u32 bufferSize, u32 unk) {
        mBuffer = pBuffer;
        mBufferSize = bufferSize;
        _14 = unk;
        mIsCorrupted = false;
    }

    /**
     * @brief Thread body of the read sequence (reading is stubbed out in this build).
     * @param pFileName The save file name.
     * @return The sequence result.
     */
    s32 SaveDataSequenceRead::threadFunc(const char* pFileName) {
        sead::NinSaveFileDevice device("save");
        return 0;
    }

    /**
     * @brief Reads a save file (stubbed out in this build).
     * @param pDevice The file device to read from.
     * @param pFileName The save file name.
     * @param pReadSize Output for the number of bytes read.
     * @return Whether the read succeeded.
     */
    bool SaveDataSequenceRead::read(sead::FileDevice* pDevice, const char* pFileName, u32* pReadSize) {
        return false;
    }

    /**
     * @brief Constructs the base of a save data sequence.
     */
    SaveDataSequenceBase::SaveDataSequenceBase() = default;
}  // namespace al
