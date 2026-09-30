#pragma once

#include <basis/seadTypes.h>

namespace sead {
class Heap;
}

namespace al {
void copyMemoryFast(u32* pDst, const u32* pSrc, u32 size);
void copyMemory(void* pDst, const void* pSrc, u32 size);
bool tryCompressByZlib(u8* pDst, u32* pDstSize, const u8* pSrc, u32 srcSize);
bool tryDecompressByZlib(u8* pDst, u32* pDstSize, const u8* pSrc, u32 srcSize);
sead::Heap* getCurrentHeap();
}  // namespace al
