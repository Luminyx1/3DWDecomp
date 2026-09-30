#include "Library/SaveData/SaveDataFunction.hpp"

#include "Library/Memory/Util.hpp"
#include "Library/System/SystemKit.hpp"
#include "Project/SaveData/SaveDataDirector.hpp"
#include "Project/SaveData/SaveDataSequenceRead.hpp"

namespace al {
inline SaveDataDirector* getSaveDataDirector() {
    return alProjectInterface::getSystemKit()->getSaveDataDirector();
}

/**
 * Gets the save data work buffer.
 * @return The work buffer, after the save data header.
 */
u8* getSaveDataWorkBuffer() {
    return getSaveDataDirector()->getWorkBuffer();
}

/**
 * Checks whether save data initialization was started.
 * @return True if initialized.
 */
bool isInitializedSaveData() {
    return getSaveDataDirector()->isInitialized();
}

/**
 * Requests asynchronous initialization of the save directory.
 * @param pFileName Save file name.
 * @param dirSize Size of the save data.
 * @param version Save data version.
 * @return True if the request was accepted.
 */
bool requestInitSaveDir(const char* pFileName, u32 dirSize, u32 version) {
    return getSaveDataDirector()->requestInitSaveDir(pFileName, dirSize, version);
}

/**
 * Requests asynchronous formatting of the save data.
 * @param a First format parameter.
 * @param b Second format parameter.
 * @return True if the request was accepted.
 */
bool requestFormatSaveData(s32 a, s32 b) {
    return getSaveDataDirector()->requestFormat(a, b);
}

/**
 * Requests an asynchronous save data read.
 * @param pFileName Save file name.
 * @param readSize Number of bytes to read.
 * @param version Save data version.
 * @return True if the request was accepted.
 */
bool requestReadSaveData(const char* pFileName, u32 readSize, u32 version) {
    return getSaveDataDirector()->requestRead(pFileName, readSize, version);
}

/**
 * Requests an asynchronous save data write.
 * @param pFileName Save file name.
 * @param writeSize Number of bytes to write.
 * @param version Save data version.
 * @param isFlushNeeded Whether to commit after writing.
 * @return True if the request was accepted.
 */
bool requestWriteSaveData(const char* pFileName, u32 writeSize, u32 version, bool isFlushNeeded) {
    return getSaveDataDirector()->requestWrite(pFileName, writeSize, version, isFlushNeeded);
}

/**
 * Requests an asynchronous save data flush.
 * @return True if the request was accepted.
 */
bool requestFlushSaveData() {
    return getSaveDataDirector()->requestFlush();
}

/**
 * Initializes the save directory synchronously.
 * @param pFileName Save file name.
 * @param dirSize Size of the save data.
 * @param version Save data version.
 * @return True on success.
 */
bool initSaveDirSync(const char* pFileName, u32 dirSize, u32 version) {
    return getSaveDataDirector()->initSaveDirSync(pFileName, dirSize, version);
}

/**
 * Formats the save data synchronously.
 * @param a First format parameter.
 * @param b Second format parameter.
 * @return True on success.
 */
bool formatSaveDataSync(s32 a, s32 b) {
    return getSaveDataDirector()->formatSync(a, b);
}

/**
 * Reads the save data synchronously.
 * @param pFileName Save file name.
 * @param readSize Number of bytes to read.
 * @param version Save data version.
 * @return True on success.
 */
bool readSaveDataSync(const char* pFileName, u32 readSize, u32 version) {
    return getSaveDataDirector()->readSync(pFileName, readSize, version);
}

/**
 * Writes the save data synchronously.
 * @param pFileName Save file name.
 * @param writeSize Number of bytes to write.
 * @param version Save data version.
 * @return True on success.
 */
bool writeSaveDataSync(const char* pFileName, u32 writeSize, u32 version) {
    return getSaveDataDirector()->writeSync(pFileName, writeSize, version);
}

/**
 * Copies the read save data out of the work buffer.
 * @param pBuffer Destination buffer.
 * @param size Number of bytes to copy.
 */
void copyReadSaveDataFromBuffer(void* pBuffer, u32 size) {
    u8* workBuffer = getSaveDataDirector()->getWorkBuffer();
    copyMemory(pBuffer, workBuffer, size);
}

/**
 * Copies data into the save data work buffer.
 * @param pBuffer Source buffer.
 * @param size Number of bytes to copy.
 */
void copyWriteSaveDataToBuffer(const void* pBuffer, u32 size) {
    u8* workBuffer = getSaveDataDirector()->getWorkBuffer();
    copyMemory(workBuffer, pBuffer, size);
}

/**
 * Updates the running save data sequence.
 * @return True if no sequence is running anymore.
 */
bool updateSaveDataSequence() {
    return getSaveDataDirector()->updateSequence();
}

/**
 * Checks whether the last save data sequence succeeded.
 * @return True if the result is zero.
 */
bool isSuccessSaveDataSequence() {
    return getSaveDataSequenceResult() == 0;
}

/**
 * Checks whether the save data sequence is done.
 * @return True if done.
 */
bool isDoneSaveDataSequence() {
    return getSaveDataDirector()->isDoneSequence();
}

/**
 * Checks whether the read sequence found corrupted data.
 * @return True if corrupted.
 */
bool isCorruptedSaveDataSequenceRead() {
    return getSaveDataDirector()->getReadSequence()->isCorrupted();
}

/**
 * Checks whether the sequence result is a corruption.
 * @return Always false.
 */
bool isCorruptedSaveDataSequenceResult() {
    return false;
}

/**
 * Gets the result of the last save data sequence.
 * @return The result code.
 */
s32 getSaveDataSequenceResult() {
    return getSaveDataDirector()->getResult();
}

/**
 * Gets the last file system error code.
 * @return The error code.
 */
s32 getLastSaveDataFSErrorCode() {
    return getSaveDataDirector()->getFSErrorCode();
}
}  // namespace al
