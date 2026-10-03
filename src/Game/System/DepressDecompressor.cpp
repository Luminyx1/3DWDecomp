#include "System/DepressDecompressor.hpp"
#include <filedevice/seadFileDevice.h>

/**
 * @brief Calculate the required decompression workspace size.
 * @return Workspace size in bytes, including the streaming buffer and header.
 */
u32 al::DepressDecompressor::GetWorkingSize() {
    return static_cast<u32>(nst::dpr::GetUndepressWorkingDataSize()) + 0x10204;
}

/**
 * @brief Read compressed bytes, seeking only when the requested offset changes.
 * @param offset Byte offset in the compressed file; must fit the file handle's signed 32-bit seek range.
 * @param size Requested byte count; must fit the file handle's unsigned 32-bit read size.
 * @param pBuffer Destination with at least size bytes; nullptr selects the internal stream buffer.
 * @return Number of bytes actually read, including zero on a failed read.
 */
u32 al::DepressDecompressor::ReadData(s64 offset, s64 size, void* pBuffer) {
    u32 readSize = 0;
    if (pBuffer == nullptr) {
        pBuffer = mBuffer;
    }
    if (mFileOffset != offset) {
        mpFile->seek(static_cast<s32>(offset), sead::FileDevice::cSeekOrigin_Begin);
    }
    mpFile->tryRead(&readSize, static_cast<u8*>(pBuffer), static_cast<u32>(size));
    mReadOffset = static_cast<s32>(offset);
    mReadSize = readSize;
    mFileOffset = static_cast<s32>(offset) + readSize;
    return readSize;
}
