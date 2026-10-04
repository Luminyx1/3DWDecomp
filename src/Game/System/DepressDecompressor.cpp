#include "System/DepressDecompressor.hpp"
#include <filedevice/seadFileDevice.h>
#include <filedevice/seadFileDeviceMgr.h>
#include <heap/seadHeap.h>
#include <heap/seadHeapMgr.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadRuntimeTypeInfo.h>

/**
 * @brief Calculate the required decompression workspace size.
 * @return Workspace size in bytes, including the streaming buffer and header.
 */
u32 al::DepressDecompressor::GetWorkingSize() {
    return static_cast<u32>(nst::dpr::GetUndepressWorkingDataSize()) + 0x10204;
}

/**
 * @brief Construct a decompressor registered under the "dpr" extension.
 * @param pWorkBuffer Caller-owned workspace, or nullptr to allocate one per load.
 * @param workSize Workspace size in bytes; 0 selects GetWorkingSize().
 */
al::DepressDecompressor::DepressDecompressor(u8* pWorkBuffer, u32 workSize)
    : sead::Decompressor("dpr") {
    if (workSize == 0) {
        workSize = GetWorkingSize();
    }

    mpProvidedWorkBuffer = pWorkBuffer;
    mWorkSize = workSize;
}

/**
 * @brief Open a compressed file and decompress it, streaming when it does not fit the buffer.
 * @param rArg Load parameters (path, device, heap, destination buffer and alignments).
 * @param pResource Resource the data is loaded for, used to query the required alignment.
 * @param pSize Receives the decompressed size; may be nullptr.
 * @param pAllocSize Receives the size of the destination buffer; may be nullptr.
 * @param pAllocated Receives whether the destination buffer was allocated here; may be nullptr.
 * @return Decompressed data, or nullptr on failure.
 */
u8* al::DepressDecompressor::tryDecompFromDevice(const sead::ResourceMgr::LoadArg& rArg,
                                                 sead::Resource* pResource, u32* pSize,
                                                 u32* pAllocSize, bool* pAllocated) {
    mFileOffset = 0;
    mReadOffset = -1;
    mReadSize = 0;

    sead::Heap* heap = rArg.load_data_heap;
    if (heap == nullptr) {
        heap = sead::HeapMgr::instance()->getCurrentHeap();
    }

    sead::FileHandle handle;
    sead::FileDevice* device;
    if (rArg.device != nullptr) {
        device = rArg.device->tryOpen(&handle, rArg.path, sead::FileDevice::cFileOpenFlag_ReadOnly,
                                      rArg.div_size);
    } else {
        device = sead::FileDeviceMgr::instance()->tryOpen(
            &handle, rArg.path, sead::FileDevice::cFileOpenFlag_ReadOnly, rArg.div_size);
    }

    mpFile = &handle;
    if (device == nullptr) {
        return nullptr;
    }

    u8* work = mpProvidedWorkBuffer;
    if (work == nullptr) {
        work = new (heap, -sead::FileDevice::cBufferMinAlignment) u8[mWorkSize];
        if (work == nullptr) {
            return nullptr;
        }
    }

    mWorkBuffer = work;
    mBuffer = work + nst::dpr::GetUndepressWorkingDataSize();
    mBufferSize = mWorkSize - nst::dpr::GetUndepressWorkingDataSize();

    nst::dpr::UndepressHeader header;
    if (ReadData(0, sizeof(header), &header) < sizeof(header)) {
        if (mpProvidedWorkBuffer == nullptr) {
            delete[] work;
        }

        return nullptr;
    }

    u32 decompSize = header.mSize;
    u32 bufferSize = rArg.load_data_buffer_size;
    if (!(decompSize <= bufferSize || bufferSize == 0)) {
        decompSize = bufferSize;
    }

    u32 allocSize = sead::Mathu::roundUpPow2(decompSize, 0x20);
    u8* dst = rArg.load_data_buffer;
    bool allocated = false;
    if (dst == nullptr) {
        s32 decompAlignment = header._10;
        s32 alignment;
        sead::DirectResource* directResource = sead::DynamicCast<sead::DirectResource>(pResource);
        if (directResource != nullptr) {
            if (rArg.load_data_alignment != 0) {
                alignment = sead::Mathi::max(rArg.load_data_alignment, 0x20);
            } else {
                if (decompAlignment == 0) {
                    decompAlignment = directResource->getLoadDataAlignment();
                }

                alignment = (rArg.instance_alignment >= 0 ? 1 : -1) *
                            sead::Mathi::max(decompAlignment, 0x20);
            }
        } else {
            alignment = (rArg.instance_alignment >= 0 ? 1 : -1) * -0x20;
        }

        dst = new (heap, alignment) u8[allocSize];
        if (dst == nullptr) {
            if (mpProvidedWorkBuffer == nullptr) {
                delete[] work;
            }

            return nullptr;
        }

        allocated = true;
    }

    u32 fileSize = handle.getFileSize();
    if (static_cast<s64>(mBufferSize) < fileSize) {
        nst::dpr::GeneralStreamUndepress(dst, this, 0);
    } else {
        ReadData(0, fileSize, mBuffer);
        nst::dpr::GeneralUndepress(mBuffer, dst, 0, mpProvidedWorkBuffer);
    }

    if (mpProvidedWorkBuffer == nullptr) {
        delete[] work;
    }

    if (pSize != nullptr) {
        *pSize = decompSize;
    }

    if (pAllocSize != nullptr) {
        *pAllocSize = allocSize;
    }

    if (pAllocated != nullptr) {
        *pAllocated = allocated;
    }

    return dst;
}

/**
 * @brief Read compressed bytes, seeking only when the requested offset changes.
 * @param offset Byte offset in the compressed file; must fit the file handle's signed 32-bit seek range.
 * @param size Requested byte count; must fit the file handle's unsigned 32-bit read size.
 * @param pBuffer Destination with at least size bytes; nullptr selects the internal stream buffer.
 * @return Number of bytes actually read, including zero on a failed read.
 */
u64 al::DepressDecompressor::ReadData(s64 offset, s64 size, void* pBuffer) {
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
