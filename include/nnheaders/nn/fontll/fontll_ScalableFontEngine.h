#pragma once

#include <nn/types.h>

namespace nn {
namespace fontll {

struct OtfKerningTable;

struct Metrics {
    u16 unitsPerEmUnused;
    u16 reserved0;
    s16 ascender;
    s16 descender;
    u8 reserved1[0x12];
    s16 boundingBoxMinY;
    u8 reserved2[2];
    s16 boundingBoxMaxY;
    u8 reserved3[0x910];
    u16 unitsPerEm;
    u8 reserved4[6];
};
static_assert(sizeof(Metrics) == 0x938);

struct GlyphMap {
    u8 reserved0[0xc];
    s16 left;
    s16 top;
    s16 advanceX;
    s16 advanceY;
    u8 reserved1[8];
    s16 width;
    s16 height;
    u8 reserved2[8];
    u8 bits[1];
};

class ScalableFontEngine {
public:
    enum Flags {
        Flags_NoEffect = 0,
        Flags_OutlinedFilled = 1,
        Flags_Outlined = 2,
    };

    ScalableFontEngine();

    int Initialize(void* pWorkBuffer, u32 workBufferSize);
    int Finalize();
    int GetError();
    int LoadFont(char* pName, const void* pFontData, u32 index, u32 nameBufferLength);
    int SetFont(char* pName);
    int SetBoldWeight(int weight);
    int SetOutlineWidth(u16 width);
    int SetFlags(Flags flags);
    int SetScale(int scaleX00, int scaleX01, int scaleX10, int scaleX11);
    int SetAutoHint(bool isEnabled);
    int GetFontMetrics(Metrics* pMetrics);
    GlyphMap* AcquireGlyphmap(u32 code, u16 type);
    int ReleasesGlyph(void* pGlyph);
    bool CheckGlyphExist(u32 code);
    int GetAdvance(s16* pAdvanceX, s16* pAdvanceY, int* pFixedAdvanceX, int* pFixedAdvanceY,
                   u32 code, u16 type);
    int GetKerning(int* pKerningX, int* pKerningY, u32 code0, u32 code1);
    OtfKerningTable* InitializeOtfKerningTable(void* (*pAllocateFunction)(size_t, size_t, void*),
                                               void* pUserData, bool isIgnored);
    void FinalizeOtfKerningTable(OtfKerningTable* pTable, void (*pFreeFunction)(void*, void*),
                                 void* pUserData);
    int AcquireOtfKerning(const OtfKerningTable* pTable, u32 code0, u32 code1, u32 fontSize);
    int AcquireOtfKerningFirst(const OtfKerningTable* pTable, u32 code, u32 fontSize);
    int AcquireOtfKerningLast(const OtfKerningTable* pTable, u32 code, u32 fontSize);
    void* GetPointerToWorkBuffer();

private:
    u8 m_Data[0xb8];
};
static_assert(sizeof(ScalableFontEngine) == 0xb8);

class ScalableFontEngineHelper {
public:
    static void* Decode(const void* pData, u32 size);
};

}  // namespace fontll
}  // namespace nn
