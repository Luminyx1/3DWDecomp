#include "Library/Memory/Util.hpp"

#include <heap/seadHeapMgr.h>

namespace al {
/**
 * Copies memory in blocks of 16 bytes.
 * @param pDst Destination buffer.
 * @param pSrc Source buffer.
 * @param size Number of bytes to copy (multiple of 16).
 */
void copyMemoryFast(u32* pDst, const u32* pSrc, u32 size) {
    for (u32 i = size >> 4; i != 0; i--) {
        u32 a = pSrc[0];
        u32 b = pSrc[1];
        u32 c = pSrc[2];
        u32 d = pSrc[3];
        pDst[0] = a;
        pDst[1] = b;
        pDst[2] = c;
        pDst[3] = d;
        pSrc += 4;
        pDst += 4;
    }
}

void copyMemory(void* pDst, const void* pSrc, u32 size) {
    u32 srcAlign = reinterpret_cast<uintptr_t>(pSrc) & 3;
    u32 dstAlign = reinterpret_cast<uintptr_t>(pDst) & 3;

    if ((size & 0xf) == 0 && srcAlign == dstAlign) {
        copyMemoryFast(static_cast<u32*>(pDst), static_cast<const u32*>(pSrc), size);
        return;
    }

    u8* dst = static_cast<u8*>(pDst);
    const u8* src = static_cast<const u8*>(pSrc);

    if (size < 0x10 || srcAlign != dstAlign) {
        for (; size != 0; size--) {
            *dst++ = *src++;
        }

        return;
    }

    if (srcAlign != 0) {
        for (u32 i = 4 - srcAlign; i != 0; i--) {
            *dst++ = *src++;
        }

        size += srcAlign - 4;
    }

    u32* dst32 = reinterpret_cast<u32*>(dst);
    const u32* src32 = reinterpret_cast<const u32*>(src);
    for (u32 rest = size; rest >= 4; rest -= 4) {
        *dst32++ = *src32++;
    }

    dst = reinterpret_cast<u8*>(dst32);
    src = reinterpret_cast<const u8*>(src32);

    for (size &= 3; size != 0; size--) {
        *dst++ = *src++;
    }
}

/**
 * Stub for zlib compression.
 * @param pDst Destination buffer.
 * @param pDstSize Destination size.
 * @param pSrc Source buffer.
 * @param srcSize Source size.
 * @return Always false.
 */
bool tryCompressByZlib(u8* pDst, u32* pDstSize, const u8* pSrc, u32 srcSize) {
    return false;
}

/**
 * Stub for zlib decompression.
 * @param pDst Destination buffer.
 * @param pDstSize Destination size.
 * @param pSrc Source buffer.
 * @param srcSize Source size.
 * @return Always false.
 */
bool tryDecompressByZlib(u8* pDst, u32* pDstSize, const u8* pSrc, u32 srcSize) {
    return false;
}

/**
 * Gets the current sead heap.
 * @return The current heap.
 */
sead::Heap* getCurrentHeap() {
    return sead::HeapMgr::instance()->getCurrentHeap();
}
}  // namespace al
