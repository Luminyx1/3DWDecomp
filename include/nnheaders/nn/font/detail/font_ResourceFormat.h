#pragma once

#include <nn/font/font_Font.h>
#include <nn/types.h>

namespace nn {
namespace font {
namespace detail {

typedef union {
    uint32_t UInt32;
    int32_t SInt32;
    float Float32;
} Type32;

inline uint64_t ByteSwap(uint64_t val) {
    const uint64_t MASK = 0xFF00FF00FF00FF00ULL;
    const uint64_t MASK2 = 0xFFFF0000FFFF0000ULL;
    val = ((val & MASK) >> 8) | ((val << 8) & MASK);
    val = ((val & MASK2) >> 16) | ((val << 16) & MASK2);
    return (val >> 32) | (val << 32);
}

inline uint32_t ByteSwap(uint32_t val) {
    const uint32_t MASK = 0xFF00FF00;
    val = ((val & MASK) >> 8) | ((val << 8) & MASK);
    return (val >> 16) | (val << 16);
}

inline int32_t ByteSwap(int32_t val) {
    Type32 data;
    data.SInt32 = val;
    data.UInt32 = ByteSwap(data.UInt32);
    return data.SInt32;
}

struct BinaryFileHeader {
    uint32_t signature;
    uint16_t byteOrder;
    uint16_t headerSize;
    uint32_t version;
    uint32_t fileSize;
    uint16_t dataBlocks;
    uint16_t reserved;
};

struct BinaryBlockHeader {
    uint32_t kind;
    uint32_t size;
};

}  // namespace detail

const uint32_t BinFileSignatureFont = 0x544e4646;
const uint32_t BinFileSignatureFontUnrelocated = 0x554e4646;
const uint32_t BinBlockSignatureFinf = 0x464e4946;
const uint32_t BinBlockSignatureCglp = 0x504c4743;
const uint32_t BinBlockSignatureTglp = 0x504c4754;
const uint32_t BinBlockSignatureCwdh = 0x48445743;
const uint32_t BinBlockSignatureCmap = 0x50414d43;
const uint32_t BinBlockSignatureKrng = 0x474e524b;
const uint32_t BinBlockSignatureGlgr = 0x52474c47;
const uint32_t BinBlockSignatureBmap = 0x50414d42;

const uint16_t InvalidGlyphIndex = 0xffff;

struct FontTextureGlyph {
    uint8_t cellWidth;
    uint8_t cellHeight;
    uint8_t sheetCount;
    uint8_t maxCharWidth;
    uint32_t sheetSize;
    int16_t baselinePos;
    uint16_t sheetFormat;
    uint16_t sheetRow;
    uint16_t sheetLine;
    uint16_t sheetWidth;
    uint16_t sheetHeight;
    uint32_t sheetImage;
};

struct FontWidth {
    uint16_t indexBegin;
    uint16_t indexEnd;
    uint32_t pNext;
    CharWidths widthTable[1];
};

struct CMapScanEntry {
    uint32_t code;
    uint16_t index;
    uint16_t padding;
};

struct CMapInfoScan {
    uint16_t count;
    uint16_t padding;
    CMapScanEntry entries[1];
};

struct FontCodeMap {
    uint32_t codeBegin;
    uint32_t codeEnd;
    uint16_t mappingMethod;
    uint16_t reserved;
    uint32_t pNext;
    uint16_t mapInfo[1];
};

struct FontInformation {
    uint8_t fontType;
    uint8_t height;
    uint8_t width;
    uint8_t ascent;
    int16_t linefeed;
    uint16_t alterCharIndex;
    CharWidths defaultWidth;
    uint8_t characterCode;
    uint32_t pGlyph;
    uint32_t pWidth;
    uint32_t pMap;
};

struct KerningFirstTableElem {
    uint32_t firstWord;
    uint32_t offset;
};

struct KerningSecondTableElem {
    uint32_t secondWord;
    int16_t kerningValue;
    uint16_t padding;
};

struct KerningSecondTable {
    uint16_t secondWordCount;
    uint16_t padding;
    KerningSecondTableElem elems[1];
};

struct FontKerningTable {
    uint16_t firstWordCount;
    uint16_t padding;
    KerningFirstTableElem firstTable[1];
};

};  // namespace font
};  // namespace nn