#include "Library/SaveData/SaveDataSequence.hpp"

#include <cstring>
#include <filedevice/nin/seadNinSaveFileDeviceNin.h>

#include "Library/SaveData/SaveDataFunction.hpp"
#include "Project/Account/AccountUtil.hpp"

namespace al {
    /**
     * @brief Constructs a save data write sequence.
     * @param unk Unknown value stored at _16.
     */
    SaveDataSequenceWrite::SaveDataSequenceWrite(u8 unk) : _16(unk) {}

    /**
     * @brief Starts writing a save data buffer, filling in its header first.
     * @param pBuffer The save data buffer.
     * @param bufferSize The size of the buffer.
     * @param version Value stored in the save data header.
     * @param isFlush Whether to flush after writing.
     */
    void SaveDataSequenceWrite::start(u8* pBuffer, u32 bufferSize, u32 version, bool isFlush) {
        mIsWrite = true;
        mBuffer = pBuffer;
        mBufferSize = bufferSize;
        mIsFlush = isFlush;
        SaveDataHeader* header = SaveDataFunction::getSaveDataHeader(pBuffer);
        header->_4 = version;
        header->mSize = bufferSize;
        header->mCheckSum = SaveDataFunction::calcSaveDataCheckSum(pBuffer);
    }

    /**
     * @brief Starts a sequence that only flushes the save data.
     */
    void SaveDataSequenceWrite::startFlushOnly() {
        mBuffer = nullptr;
        mBufferSize = 0;
        mIsWrite = false;
        mIsFlush = true;
    }

    /**
     * @brief Thread body of the write sequence (writing is stubbed out in this build).
     * @param pFileName The save file name.
     * @return The sequence result.
     */
    s32 SaveDataSequenceWrite::threadFunc(const char* pFileName) {
        sead::NinSaveFileDevice device("save");
        return 0;
    }

    /**
     * @brief Writes a save file (stubbed out in this build).
     * @param pDevice The file device to write to.
     * @param pFileName The save file name.
     * @return Whether the write succeeded.
     */
    bool SaveDataSequenceWrite::write(sead::FileDevice* pDevice, const char* pFileName) {
        return true;
    }

    /**
     * @brief Constructs a save data format sequence.
     */
    SaveDataSequenceFormat::SaveDataSequenceFormat() = default;

    /**
     * @brief Starts formatting the save data.
     * @param unk1 Unknown value stored at _8.
     * @param unk2 Unknown value stored at _c.
     */
    void SaveDataSequenceFormat::start(s32 unk1, s32 unk2) {
        _8 = unk1;
        _c = unk2;
    }

    /**
     * @brief Thread body of the format sequence (not supported in this build).
     * @param pFileName The save file name.
     * @return Always -1.
     */
    s32 SaveDataSequenceFormat::threadFunc(const char* pFileName) {
        return -1;
    }

    /**
     * @brief Constructs a save directory initialization sequence.
     * @param unk Unknown value stored at _18.
     */
    SaveDataSequenceInitDir::SaveDataSequenceInitDir(u8 unk) : _18(unk) {}

    /**
     * @brief Starts initializing the save directory, clearing the work buffer.
     * @param pBuffer The work buffer.
     * @param bufferSize The size of the work buffer.
     * @param unk Unknown value stored at _14.
     */
    void SaveDataSequenceInitDir::start(u8* pBuffer, u32 bufferSize, u32 unk) {
        mBuffer = pBuffer;
        mBufferSize = bufferSize;
        _14 = unk;
        memset(pBuffer, 0, bufferSize);
    }

    /**
     * @brief Thread body of the directory initialization: makes sure a user account is selected.
     * @param pFileName The save file name.
     * @return The sequence result.
     */
    s32 SaveDataSequenceInitDir::threadFunc(const char* pFileName) {
        tryInitAccount();
        nn::account::Uid uid = getUid();
        if (!uid.IsValid()) {
            nn::account::UserHandle handle;
            if (nn::account::TryOpenPreselectedUser(&handle)) {
                nn::Result result = nn::account::GetUserId(&uid, handle);
                nn::account::CloseUser(handle);
                if (result.IsSuccess()) {
                    return 0;
                }
            }
            s32 userNum;
            nn::account::ListAllUsers(&userNum, &uid, 1);
        }
        return 0;
    }
}  // namespace al
