#pragma once

#include <basis/seadTypes.h>

namespace nst::dpr {

/**
 * Header at the start of every compressed blob (0x18 bytes).
 */
struct UndepressHeader {
    u64 mType;  ///< Compression type, see UndepressType.
    s64 mSize;  ///< Size of the decompressed data in bytes.
    u64 _10;
};

static_assert(sizeof(UndepressHeader) == 0x18);

/**
 * Compression types stored in UndepressHeader::mType.
 */
enum UndepressType : u64 {
    UndepressType_None = 0,
    UndepressType_LZ77 = 1,
    UndepressType_HuffmanLZ77 = 2,
    UndepressType_RLE = 3,
};

/**
 * Source of a streamed compressed blob, read piecewise into a buffer.
 */
class DepressStreamContext {
public:
    /**
     * @brief Read a range of the compressed blob.
     * @param offset Byte offset in the compressed blob.
     * @param size Number of bytes to read.
     * @param pBuffer Destination; nullptr reads into mBuffer.
     * @return Number of bytes actually read.
     */
    virtual u64 ReadData(s64 offset, s64 size, void* pBuffer) = 0;

    void* mWorkBuffer;
    u8* mBuffer;
    u64 mBufferSize;
};

static_assert(sizeof(DepressStreamContext) == 0x20);

/**
 * Length or distance code of the Huffman LZ77 format: number of extra bits and base value.
 */
struct DeflateCodeInfo {
    u16 mExtraBits;
    u16 mBase;
};

extern long gUndepressEndianSwap;
extern const DeflateCodeInfo gDeflateLengthInfo[];
extern const DeflateCodeInfo gDeflateDistInfo[];

u64 GetUndepressWorkingDataSize();
void HuffmanUndepressLZ77stream(void* pDst, DepressStreamContext* pContext);
void HuffmanUndepressLZ77(void* pSrc, void* pDst, void* pWork);
void UndepressRLE(void* pSrc, void* pDst);
void UndepressStreamRLE(void* pDst, DepressStreamContext* pContext);
void UndepressLZ77(void* pSrc, void* pDst);
void UndepressStreamLZ77(void* pDst, DepressStreamContext* pContext);
void* GeneralUndepress(void* pSrc, void* pDst, long endianSwap, void* pWork);
void* GeneralStreamUndepress(void* pDst, DepressStreamContext* pContext, long endianSwap);
s64 GetUndepressSize(void* pSrc, long endianSwap);

}  // namespace nst::dpr
