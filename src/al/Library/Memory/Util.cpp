#include "Library/Memory/MemoryUtil.hpp"

#include <heap/seadHeapMgr.h>

namespace al {
    /**
     * @brief Copies memory in 16 byte blocks.
     * @param pDst The destination buffer.
     * @param pSrc The source buffer.
     * @param size The number of bytes to copy (only whole 16 byte blocks are copied).
     */
    void copyMemoryFast(u32* pDst, const u32* pSrc, u32 size) {
        for (u32 i = size >> 4; i != 0; i--) {
            u32 word0 = pSrc[0];
            u32 word1 = pSrc[1];
            u32 word2 = pSrc[2];
            u32 word3 = pSrc[3];
            pDst[0] = word0;
            pDst[1] = word1;
            pDst[2] = word2;
            pDst[3] = word3;
            pDst += 4;
            pSrc += 4;
        }
    }

    /**
     * @brief Copies memory, using word copies when both buffers share the same alignment.
     * @param pDst The destination buffer.
     * @param pSrc The source buffer.
     * @param size The number of bytes to copy.
     */
    void copyMemory(void* pDst, const void* pSrc, u32 size) {
        u32 srcAlign = reinterpret_cast<uintptr_t>(pSrc) & 3;
        u32 dstAlign = reinterpret_cast<uintptr_t>(pDst) & 3;

        if ((size & 0xf) == 0 && srcAlign == dstAlign) {
            copyMemoryFast(static_cast<u32*>(pDst), static_cast<const u32*>(pSrc), size);
            return;
        }

        u8* dst = static_cast<u8*>(pDst);
        const u8* src = static_cast<const u8*>(pSrc);

        if (size >= 16 && srcAlign == dstAlign) {
            if (srcAlign != 0) {
                for (u32 i = 0; i < 4 - srcAlign; i++) {
                    *dst++ = *src++;
                }
                size = size + srcAlign - 4;
            }

            u32* dst32 = reinterpret_cast<u32*>(dst);
            const u32* src32 = reinterpret_cast<const u32*>(src);
            while (size >= 4) {
                *dst32++ = *src32++;
                size -= 4;
            }

            dst = reinterpret_cast<u8*>(dst32);
            src = reinterpret_cast<const u8*>(src32);
            while (size != 0) {
                *dst++ = *src++;
                size--;
            }
        } else {
            while (size != 0) {
                *dst++ = *src++;
                size--;
            }
        }
    }

    /**
     * @brief Compresses data with zlib (not supported in this build).
     * @param pDst The destination buffer.
     * @param pDstSize The destination size, updated with the compressed size.
     * @param pSrc The source data.
     * @param srcSize The source size.
     * @return Whether compression succeeded.
     */
    bool tryCompressByZlib(u8* pDst, u32* pDstSize, const u8* pSrc, u32 srcSize) {
        return false;
    }

    /**
     * @brief Decompresses zlib data (not supported in this build).
     * @param pDst The destination buffer.
     * @param pDstSize The destination size, updated with the decompressed size.
     * @param pSrc The compressed data.
     * @param srcSize The compressed size.
     * @return Whether decompression succeeded.
     */
    bool tryDecompressByZlib(u8* pDst, u32* pDstSize, const u8* pSrc, u32 srcSize) {
        return false;
    }

    /**
     * @brief Gets the current heap of the heap manager.
     * @return The current heap.
     */
    sead::Heap* getCurrentHeap() {
        return sead::HeapMgr::instance()->getCurrentHeap();
    }
}  // namespace al
