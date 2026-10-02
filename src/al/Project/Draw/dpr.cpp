#include "Project/Draw/dpr.hpp"

#include <attributes.h>
#include <cstring>

namespace nst::dpr {

namespace {

/**
 * Reads a bit stream LSB first, one byte at a time.
 */
struct BitReader {
    /**
     * Starts reading at the given byte.
     * @param pData First byte of the bit stream.
     */
    void init(const u8* pData) {
        mStart = pData;
        mCurrent = pData;
        mBitsLeft = 8;
        mBitPos = 0;
        mByte = *pData;
    }

    /**
     * Gets the current bit without consuming it.
     * @return The current bit.
     */
    u32 peekBit() const {
        return mByte & 1;
    }

    /**
     * Moves to the next bit, loading the next byte when the current one is used up.
     */
    void skipBit() {
        mBitsLeft--;
        mBitPos++;

        if (mBitsLeft == 0) {
            mBitsLeft = 8;
            mCurrent++;
            mByte = *mCurrent;
        } else {
            mByte >>= 1;
        }
    }

    /**
     * Reads one bit.
     * @return The bit read.
     */
    u32 readBit() {
        u8 byte = mByte;
        skipBit();
        return byte & 1;
    }

    /**
     * Reads a value LSB first.
     * @param bitNum Number of bits to read.
     * @return The value read.
     */
    u64 readBits(u64 bitNum) {
        u64 value = 0;

        for (u64 i = 0; i < bitNum; i++) {
            value |= static_cast<u64>(peekBit()) << i;
            skipBit();
        }

        return value;
    }

    const u8* mStart;
    const u8* mCurrent;
    u8 mByte;
    u8 mBitsLeft;
    u64 mBitPos;
};

/**
 * Node of a canonical Huffman decoding tree.
 */
struct HuffmanNode {
    HuffmanNode* mChildren[2];
    u8 mLength;
    bool mIsLeaf;
    u16 mValue;
    u64 mCode;
};

static_assert(sizeof(HuffmanNode) == 0x20);

constexpr u64 cLiteralSymbolNum = 285;
constexpr u64 cDistanceSymbolNum = 30;

/**
 * Working memory of the Huffman LZ77 decoder.
 */
struct HuffmanWork {
    HuffmanNode mLiteralNodes[cLiteralSymbolNum * 3];
    HuffmanNode mDistanceNodes[cLiteralSymbolNum * 3];
    HuffmanNode* mNextNode;
};

static_assert(sizeof(HuffmanWork) == 0xD5C8);

/**
 * Walks a Huffman tree down to a leaf.
 * @param pRoot Root of the tree.
 * @param pReader Bit stream to read the code from.
 * @return The decoded symbol.
 */
inline u16 decodeSymbol(const HuffmanNode* pRoot, BitReader* pReader) {
    const HuffmanNode* node = pRoot;

    while (!node->mIsLeaf) {
        node = node->mChildren[pReader->readBit()];
    }

    return node->mValue;
}

/**
 * Swaps the byte order of the low 32 bits of a value.
 * @param value Value to swap.
 * @return The swapped value.
 */
inline u64 swapEndian(u64 value) {
    return ((value & 0xff) << 24) | ((value & 0xff00) << 8) | ((value >> 8) & 0xff00) |
           ((value >> 24) & 0xff);
}

/**
 * Converts the header type to native byte order if requested by gUndepressEndianSwap.
 * @param pHeader Header to convert.
 */
inline void swapHeader(UndepressHeader* pHeader) {
    if (gUndepressEndianSwap != 0) {
        pHeader->mType = swapEndian(pHeader->mType);
    }
}

/**
 * Reads the next byte of a streamed blob, refilling the buffer when it runs out.
 * @param pContext Stream to read from.
 * @param rSrc Current read position in the buffer.
 * @param rOffset Offset in the blob of the next buffer refill.
 * @param pBuffer Stream buffer.
 * @param bufferSize Size of the stream buffer.
 * @return The byte read.
 */
inline u8 readStreamByte(DepressStreamContext* pContext, const u8*& rSrc, u64& rOffset,
                         u8* pBuffer, u64 bufferSize) {
    if (rSrc >= pBuffer + bufferSize) {
        pContext->read(rOffset, bufferSize, pBuffer);
        rOffset += bufferSize;
        rSrc = pBuffer;
    }

    return *rSrc++;
}

}  // namespace

static u8* decodeHuffmanBlock(HuffmanWork* pWork, const u8** ppSrc, u8* pDst, u8* pDstEnd);
static HuffmanNode* buildHuffmanTree(HuffmanWork* pWork, BitReader* pReader, HuffmanNode* pNodes,
                                     u64 symbolNum);

static HuffmanWork sHuffmanWork;

/**
 * Gets the size of the working memory needed for streamed Huffman LZ77 decompression.
 * @return The working memory size in bytes.
 */
u64 GetUndepressWorkingDataSize() {
    return sizeof(HuffmanWork);
}

/**
 * Decompresses a streamed Huffman LZ77 blob.
 * @param pDst Destination of the decompressed data.
 * @param pContext Stream to read the compressed blob from.
 */
void HuffmanUndepressLZ77stream(void* pDst, DepressStreamContext* pContext) {
    u8* buffer = pContext->mBuffer;
    HuffmanWork* work = static_cast<HuffmanWork*>(pContext->mWorkBuffer);
    pContext->read(0, sizeof(UndepressHeader), buffer);

    UndepressHeader* header = reinterpret_cast<UndepressHeader*>(buffer);
    swapHeader(header);

    u8* dst = static_cast<u8*>(pDst);
    u8* dstEnd = dst + header->mSize;
    u64 offset = sizeof(UndepressHeader);

    while (dst < dstEnd) {
        const u8* src = buffer;
        pContext->read(offset, 8, buffer);

        BitReader reader;
        reader.init(buffer);
        u32 isCompressed = reader.readBit();
        u64 blockSize = reader.readBits(31);

        if (isCompressed != 0) {
            pContext->read(offset + 8, (blockSize + 7) / 8, buffer + 8);
        } else {
            pContext->read(offset + 8, blockSize, buffer + 8);
        }
        dst = decodeHuffmanBlock(work, &src, dst, dstEnd);
        offset += src - buffer;
    }
}

/**
 * Decompresses one block of Huffman LZ77 data.
 * @param pWork Working memory for the Huffman trees.
 * @param ppSrc Start of the block, advanced past it.
 * @param pDst Destination of the decompressed data.
 * @param pDstEnd End of the destination.
 * @return The destination position after the block.
 */
static u8* decodeHuffmanBlock(HuffmanWork* pWork, const u8** ppSrc, u8* pDst, u8* pDstEnd) {
    const u8* blockStart = *ppSrc;

    BitReader reader;
    reader.init(blockStart);
    u32 isCompressed = reader.readBit();
    u64 blockSize = reader.readBits(31);

    u64 consumedSize;

    if (isCompressed == 0) {
        memcpy(pDst, blockStart + 8, blockSize);
        pDst += blockSize;
        consumedSize = blockSize + 8;
    } else {
        u64 literalTableBits = reader.readBits(16);
        const u8* distanceTable = reader.mCurrent + (literalTableBits + 7) / 8;
        HuffmanNode* literalRoot =
            buildHuffmanTree(pWork, &reader, pWork->mLiteralNodes, cLiteralSymbolNum);

        reader.init(distanceTable);
        reader.mBitPos = (distanceTable - blockStart) * 8;

        u64 distanceTableBits = reader.readBits(16);
        const u8* data = reader.mCurrent + (distanceTableBits + 7) / 8;
        HuffmanNode* distanceRoot =
            buildHuffmanTree(pWork, &reader, pWork->mDistanceNodes, cDistanceSymbolNum);

        reader.init(data);
        reader.mBitPos = (data - blockStart) * 8;

        while (pDst < pDstEnd && reader.mBitPos < blockSize) {
            u64 symbol = decodeSymbol(literalRoot, &reader);

            if (symbol < 256) {
                *pDst++ = symbol;
                continue;
            }

            const DeflateCodeInfo& lengthInfo = gDeflateLengthInfo[symbol - 256];
            u64 length = reader.readBits(lengthInfo.mExtraBits) + lengthInfo.mBase + 3;

            u16 distanceSymbol = decodeSymbol(distanceRoot, &reader);
            const DeflateCodeInfo& distanceInfo = gDeflateDistInfo[distanceSymbol];
            u64 distance = reader.readBits(distanceInfo.mExtraBits) + distanceInfo.mBase;
            u64 i = 0;

            for (; i < length && pDst + i < pDstEnd; i++) {
                pDst[i] = pDst[i - distance - 1];
            }

            pDst += i;
        }

        consumedSize = (blockSize + 7) / 8;
    }

    *ppSrc = blockStart + consumedSize;
    return pDst;
}

/**
 * Decompresses a Huffman LZ77 blob held in memory.
 * @param pSrc Compressed blob.
 * @param pDst Destination of the decompressed data.
 * @param pWork Unused, a static working memory is used instead.
 */
NOINLINE void HuffmanUndepressLZ77(void* pSrc, void* pDst, void* pWork) {
    const UndepressHeader* header = static_cast<const UndepressHeader*>(pSrc);
    const u8* src = reinterpret_cast<const u8*>(header + 1);
    u8* dst = static_cast<u8*>(pDst);
    u8* dstEnd = dst + header->mSize;

    while (dst < dstEnd) {
        dst = decodeHuffmanBlock(&sHuffmanWork, &src, dst, dstEnd);
    }
}

/**
 * Reads the code lengths of a Huffman table and builds its canonical decoding tree.
 * @param pWork Working memory holding the node allocator.
 * @param pReader Bit stream to read the code lengths from.
 * @param pNodes Node storage: one leaf per symbol followed by the inner nodes.
 * @param symbolNum Number of symbols.
 * @return The root of the tree.
 */
static HuffmanNode* buildHuffmanTree(HuffmanWork* pWork, BitReader* pReader, HuffmanNode* pNodes,
                                     u64 symbolNum) {
    memset(pNodes, 0, symbolNum * 2 * sizeof(HuffmanNode));

    u64 lengthCounts[32] = {};
    u64 nextCodes[32] = {};

    for (u64 i = 0; i < symbolNum;) {
        u32 isRun = pReader->readBit();
        u64 count = pReader->readBits(7);

        if (isRun != 0) {
            u8 length = pReader->readBits(5);
            count += 2;

            for (u64 j = 0; j < count; j++) {
                pNodes[i + j].mLength = length;
            }

            lengthCounts[length] += count;
        } else {
            count += 1;

            for (u64 j = 0; j < count; j++) {
                u8 length = pReader->readBits(5);
                pNodes[i + j].mLength = length;
                lengthCounts[length]++;
            }
        }

        i += count;
    }

    lengthCounts[0] = 0;
    u64 code = 0;

    for (s32 i = 1; i < 32; i++) {
        code = (code + lengthCounts[i - 1]) << 1;
        nextCodes[i] = code;
    }

    HuffmanNode* root = &pNodes[symbolNum];
    pWork->mNextNode = root + 1;

    for (u64 i = 0; i < symbolNum; i++) {
        HuffmanNode* leaf = &pNodes[i];
        u8 length = leaf->mLength;
        leaf->mIsLeaf = true;

        if (length == 0) {
            continue;
        }

        leaf->mCode = nextCodes[length];
        leaf->mValue = i;
        nextCodes[length]++;

        HuffmanNode* node = root;

        u64 shift = length - 1;
        u64 bit = (leaf->mCode >> shift) & 1;

        while (shift != 0) {
            if (bit != 0) {
                if (node->mChildren[1] == nullptr) {
                    node->mChildren[1] = pWork->mNextNode++;
                }

                node = node->mChildren[1];
            } else {
                if (node->mChildren[0] == nullptr) {
                    node->mChildren[0] = pWork->mNextNode++;
                }

                node = node->mChildren[0];
            }

            shift--;
            bit = (leaf->mCode >> shift) & 1;
        }

        node->mChildren[bit] = leaf;
    }

    return root;
}

/**
 * Decompresses a run length encoded blob held in memory, writing 16 bits at a time.
 * @param pSrc Compressed blob.
 * @param pDst Destination of the decompressed data.
 */
void UndepressRLE(void* pSrc, void* pDst) {
    const UndepressHeader* header = static_cast<const UndepressHeader*>(pSrc);
    const u8* src = reinterpret_cast<const u8*>(header + 1);
    u16* dst = static_cast<u16*>(pDst);
    u64 size = header->mSize;
    u16 halfword;
    u64 pos = 0;

    while (pos < size) {
        s8 flag = *src++;

        if (flag < 0) {
            u8 count = (flag & 0x7f) + 3;
            u8 value = *src++;

            for (; count != 0; count--) {
                if ((pos & 1) != 0) {
                    *dst++ = halfword | (value << 8);
                    halfword = 0;
                } else {
                    halfword = value;
                }

                pos++;
            }
        } else {
            u8 count = flag + 1;

            for (; count != 0; count--) {
                u8 value = *src++;

                if ((pos & 1) != 0) {
                    *dst++ = halfword | (value << 8);
                    halfword = 0;
                } else {
                    halfword = value;
                }

                pos++;
            }
        }
    }

    if ((pos & 1) != 0) {
        *dst = halfword;
    }
}

/**
 * Decompresses a streamed run length encoded blob, writing 16 bits at a time.
 * @param pDst Destination of the decompressed data.
 * @param pContext Stream to read the compressed blob from.
 */
void UndepressStreamRLE(void* pDst, DepressStreamContext* pContext) {
    u16* dst = static_cast<u16*>(pDst);
    u8* buffer = pContext->mBuffer;
    u64 bufferSize = pContext->mBufferSize;

    UndepressHeader header;
    pContext->read(0, sizeof(UndepressHeader), &header);
    swapHeader(&header);

    u64 size = header.mSize;
    const u8* src = buffer + bufferSize;
    u64 offset = sizeof(UndepressHeader);
    u16 halfword;
    u64 pos = 0;

    while (pos < size) {
        s8 flag = readStreamByte(pContext, src, offset, buffer, bufferSize);

        if (flag < 0) {
            u8 count = (flag & 0x7f) + 3;
            u8 value = readStreamByte(pContext, src, offset, buffer, bufferSize);

            for (; count != 0; count--) {
                if ((pos & 1) != 0) {
                    *dst++ = halfword | (value << 8);
                    halfword = 0;
                } else {
                    halfword = value;
                }

                pos++;
            }
        } else {
            u8 count = flag + 1;

            for (; count != 0; count--) {
                u8 value = readStreamByte(pContext, src, offset, buffer, bufferSize);

                if ((pos & 1) != 0) {
                    *dst++ = halfword | (value << 8);
                    halfword = 0;
                } else {
                    halfword = value;
                }

                pos++;
            }
        }
    }

    if ((pos & 1) != 0) {
        *dst = halfword;
    }
}

/**
 * Decompresses an LZ77 blob held in memory.
 * @param pSrc Compressed blob.
 * @param pDst Destination of the decompressed data.
 */
void UndepressLZ77(void* pSrc, void* pDst) {
    const UndepressHeader* header = static_cast<const UndepressHeader*>(pSrc);
    const u8* src = reinterpret_cast<const u8*>(header + 1);
    u8* dst = static_cast<u8*>(pDst);
    u64 size = header->mSize;
    u64 pos = 0;

    while (pos < size) {
        u8 flags = *src++;

        for (u32 mask = 0x80; mask != 0 && pos < size; mask >>= 1) {
            if ((flags & mask) != 0) {
                u8 byte0 = *src++;
                u8 byte1 = *src++;
                u64 length = (byte0 >> 4) + 3;
                u64 distance = ((byte0 & 0xf) << 8) | byte1;

                for (u64 end = pos + length; pos < end; pos++) {
                    dst[pos] = dst[pos - distance - 1];
                }
            } else {
                dst[pos++] = *src++;
            }
        }
    }
}

/**
 * Decompresses a streamed LZ77 blob.
 * @param pDst Destination of the decompressed data.
 * @param pContext Stream to read the compressed blob from.
 */
void UndepressStreamLZ77(void* pDst, DepressStreamContext* pContext) {
    u8* dst = static_cast<u8*>(pDst);
    u8* buffer = pContext->mBuffer;
    u64 bufferSize = pContext->mBufferSize;

    UndepressHeader header;
    pContext->read(0, sizeof(UndepressHeader), &header);
    swapHeader(&header);

    u64 size = header.mSize;
    const u8* src = buffer + bufferSize;
    u64 offset = sizeof(UndepressHeader);
    u64 pos = 0;

    while (pos < size) {
        u8 flags = readStreamByte(pContext, src, offset, buffer, bufferSize);

        for (u32 mask = 0x80; mask != 0 && pos < size; mask >>= 1) {
            if ((flags & mask) != 0) {
                u8 byte0 = readStreamByte(pContext, src, offset, buffer, bufferSize);
                u8 byte1 = readStreamByte(pContext, src, offset, buffer, bufferSize);
                u64 length = (byte0 >> 4) + 3;
                u64 distance = ((byte0 & 0xf) << 8) | byte1;

                for (u64 end = pos + length; pos < end; pos++) {
                    dst[pos] = dst[pos - distance - 1];
                }
            } else {
                dst[pos++] = readStreamByte(pContext, src, offset, buffer, bufferSize);
            }
        }
    }
}

/**
 * Decompresses a blob held in memory, picking the method from its header.
 * @param pSrc Compressed blob; its header is converted to native byte order in place.
 * @param pDst Destination of the decompressed data.
 * @param endianSwap Whether the header is stored in the opposite byte order.
 * @param pWork Working memory passed to HuffmanUndepressLZ77.
 * @return The destination.
 */
void* GeneralUndepress(void* pSrc, void* pDst, long endianSwap, void* pWork) {
    gUndepressEndianSwap = endianSwap;

    UndepressHeader* header = static_cast<UndepressHeader*>(pSrc);
    swapHeader(header);

    if (header->mType == UndepressType_RLE) {
        UndepressRLE(pSrc, pDst);
    } else if (header->mType == UndepressType_HuffmanLZ77) {
        HuffmanUndepressLZ77(pSrc, pDst, pWork);
    } else if (header->mType == UndepressType_LZ77) {
        UndepressLZ77(pSrc, pDst);
    } else {
        memcpy(pDst, header + 1, header->mSize);
    }

    return pDst;
}

/**
 * Decompresses a streamed blob, picking the method from its header.
 * @param pDst Destination of the decompressed data.
 * @param pContext Stream to read the compressed blob from.
 * @param endianSwap Whether the header is stored in the opposite byte order.
 * @return The destination.
 */
void* GeneralStreamUndepress(void* pDst, DepressStreamContext* pContext, long endianSwap) {
    gUndepressEndianSwap = endianSwap;

    UndepressHeader header;
    pContext->read(0, sizeof(UndepressHeader), &header);
    swapHeader(&header);

    switch (header.mType) {
    case UndepressType_LZ77:
        UndepressStreamLZ77(pDst, pContext);
        break;
    case UndepressType_HuffmanLZ77:
        HuffmanUndepressLZ77stream(pDst, pContext);
        break;
    case UndepressType_RLE:
        UndepressStreamRLE(pDst, pContext);
        break;
    default: {
        u8* dst = static_cast<u8*>(pDst);
        u64 pos = 0;

        while (pos + pContext->mBufferSize <= header.mSize) {
            pContext->read(pos + sizeof(UndepressHeader), pContext->mBufferSize, nullptr);
            memcpy(dst + pos, pContext->mBuffer, pContext->mBufferSize);
            pos += pContext->mBufferSize;
        }

        if (pos < header.mSize) {
            pContext->read(pos + sizeof(UndepressHeader), header.mSize - pos, nullptr);
            memcpy(dst + pos, pContext->mBuffer, header.mSize - pos);
        }

        break;
    }
    }

    return pDst;
}

/**
 * Gets the decompressed size of a blob.
 * @param pSrc Compressed blob.
 * @param endianSwap Unused.
 * @return The decompressed size in bytes.
 */
s64 GetUndepressSize(void* pSrc, long endianSwap) {
    return static_cast<const UndepressHeader*>(pSrc)->mSize;
}

}  // namespace nst::dpr
