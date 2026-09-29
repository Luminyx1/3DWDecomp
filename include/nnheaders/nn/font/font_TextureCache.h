#pragma once

#include <nn/font/font_Font.h>
#include <nn/font/font_GlyphTreeMap.h>
#include <nn/fontll/fontll_ScalableFontEngine.h>
#include <nn/gfx/gfx_Device.h>
#include <nn/gfx/gfx_MemoryPool.h>
#include <nn/gfx/gfx_MemoryPoolInfo.h>
#include <nn/gfx/gfx_Texture.h>
#include <nn/types.h>
#include <nn/util/util_IntrusiveList.h>

namespace nn {
namespace font {

class PlacementTextureObject : public TextureObject {
public:
    /**
     * Gets the texture view; the texture cache owns the view instead.
     * @return nullptr
     */
    const nn::gfx::TextureView* GetTextureView() const override { return nullptr; }

    /**
     * Gets the texture view; the texture cache owns the view instead.
     * @return nullptr
     */
    nn::gfx::TextureView* GetTextureView() override { return nullptr; }
};

class TextureCache {
public:
    static const int FontFaceCountMax = 32;
    static const int InnerFontCountMax = 16;
    static const int CharCodeRangeCountMax = 16;
    static const int CoreCountMax = 8;
    static const int LineCountMax = 256;
    static const int FontNameLength = 50;
    static const int WorkMemorySizeDefault = 0x80000;
    static const int NoPlotWorkMemorySizeDefault = 0xc800;
    static const int GlyphNodeCountMaxDefault = 0x400;
    static const int TextureCacheSizeDefault = 0x400;

    typedef void* (*AllocateFunction)(size_t size, size_t alignment, void* pUserData);
    typedef void (*FreeFunction)(void* ptr, void* pUserData);
    typedef u32 (*GetCoreIdFunction)();
    typedef u8 (*CalculateLineKindFunction)(u16 height);
    typedef u32 (*CalculateLineHeightFunction)(u8 kind);

    enum FontDataType {
        FontDataType_Ttf,
        FontDataType_Otf,
        FontDataType_Bfttf,
        FontDataType_Bfotf,
    };

    enum NoSpaceError {
        NoSpaceError_NoError,
        NoSpaceError_GlyphNodeSpaceShortage,
        NoSpaceError_LineCountShortage,
        NoSpaceError_TextureSpaceShortage,
    };

    struct InitializeArg {
        u32 textureCacheWidth;
        u32 textureCacheHeight;
        AllocateFunction pAllocateFunction;
        void* pUserDataForAllocateFunction;
        const void* pFontDatas[FontFaceCountMax][InnerFontCountMax];
        size_t fontDataSizes[FontFaceCountMax][InnerFontCountMax];
        float boldWeights[FontFaceCountMax][InnerFontCountMax];
        u8 borderWidths[FontFaceCountMax][InnerFontCountMax];
        float scaleWidths[FontFaceCountMax][InnerFontCountMax];
        float scaleHeights[FontFaceCountMax][InnerFontCountMax];
        u32 fontDataTypes[FontFaceCountMax][InnerFontCountMax];
        bool ignorePalt[FontFaceCountMax][InnerFontCountMax];
        bool isWidthFromBoundingBox[FontFaceCountMax][InnerFontCountMax];
        s16 letterSpacings[FontFaceCountMax][InnerFontCountMax];
        bool isFixedWidth[FontFaceCountMax][InnerFontCountMax];
        s16 fixedWidths[FontFaceCountMax][InnerFontCountMax];
        int baselineOffsets[FontFaceCountMax][InnerFontCountMax];
        u16 overwrittenAscents[FontFaceCountMax][InnerFontCountMax];
        u16 overwrittenDescents[FontFaceCountMax][InnerFontCountMax];
        u32 charCodeRangeCounts[FontFaceCountMax][InnerFontCountMax];
        u32 charCodeRangeFirsts[FontFaceCountMax][InnerFontCountMax][CharCodeRangeCountMax];
        u32 charCodeRangeLasts[FontFaceCountMax][InnerFontCountMax][CharCodeRangeCountMax];
        u32 fontFaceCount;
        u32 innerFontCounts[FontFaceCountMax];
        size_t workMemorySize;
        size_t noPlotWorkMemorySize;
        bool isNoPlotWorkMemorySizeShared;
        u32 glyphNodeCountMax;
        u32 fontIndexes[FontFaceCountMax][InnerFontCountMax];
        bool isMultiCoreEnabled;
        u32 coreCount;
        GetCoreIdFunction pGetCoreIdFunction;
        CalculateLineKindFunction pCalculateLineKindFunction;
        CalculateLineHeightFunction pCalculateLineHeightFunction;
        bool isAutoHintEnabled;
        bool isDrawingAdvanceWidthUsed;

        void SetDefault();
    };

    struct FontMetrics {
        float ascentRatio;
        float heightRatio;
        float boundingBoxAscentRatio;
        float boundingBoxHeightRatio;
        float scaleWidth;
        float scaleHeight;
        int baselineOffset;
    };

    typedef util::IntrusiveList<GlyphNode,
                                util::IntrusiveListMemberNodeTraits<GlyphNode, &GlyphNode::m_Link>>
        GlyphList;
    typedef util::IntrusiveList<
        GlyphNode, util::IntrusiveListMemberNodeTraits<GlyphNode, &GlyphNode::m_LineLink>>
        GlyphLineList;

    struct LineInfo {
        GlyphLineList list;
        u16 y;
        u16 currentX;
        u8 kind;
        u8 no;
    };

    TextureCache();
    virtual ~TextureCache();

    static void SetMemoryPoolInfo(nn::gfx::MemoryPoolInfo* pInfo);
    static size_t CalculateMemoryPoolAlignment(nn::gfx::Device* pDevice, int width, int height);
    static size_t CalculateMemoryPoolSize(nn::gfx::Device* pDevice, int width, int height);

    void Initialize(nn::gfx::Device* pDevice, const InitializeArg& rArg,
                    nn::gfx::MemoryPool* pMemoryPool, ptrdiff_t memoryPoolOffset,
                    size_t memoryPoolSize);
    int InitializeFontFaceTable(const InitializeArg& rArg);
    void AssertFsError(const char* pApiName) const;
    void AssertFsErrorNoPlot(const char* pApiName, u32 coreId) const;
    void SetFontFace(u32 fontFace);
    void Finalize(nn::gfx::Device* pDevice, FreeFunction pFreeFunction, void* pUserData);
    void RegisterTextureViewToDescriptorPool(RegisterTextureViewSlot pRegisterFunction,
                                             void* pUserData);
    void UnregisterTextureViewFromDescriptorPool(UnregisterTextureViewSlot pUnregisterFunction,
                                                 void* pUserData);
    bool RegisterGlyph(u32 code, u32 fontSize, u16 fontFace, int lockGroup,
                       bool isNoBreakHyphenReplaced);
    bool GetInnerFontFace(u32* pInnerFontFace, u32 fontFace, u32 code);
    u32 EraseNotInFontGlyphs();
    u32 RegisterGlyphsWithLength(const u16* pCodes, u32 codeLength, u32 fontSize, u16 fontFace,
                                 int lockGroup, bool isNoBreakHyphenReplaced);
    u32 RegisterGlyphsWithLengthUtf8(const char* pCodes, u32 codeLength, u32 fontSize,
                                     u16 fontFace, int lockGroup, bool isNoBreakHyphenReplaced);
    void UpdateTextureCache();
    LineInfo* CreateNewLineImpl(u8 lineKind);
    GlyphNode* FindAndReserveEraseGlyph(u8 lineKind, u16 glyphWidth);
    void CompleteTextureCache();
    void ClearLockAllGlyphs(int lockGroup);
    void ClearLockGlyphsWithLength(const u16* pCodes, u32 codeLength, u32 fontSize, u16 fontFace,
                                   int lockGroup, bool isNoBreakHyphenReplaced);
    void ClearLockGlyphsWithLengthUtf8(const char* pCodes, u32 codeLength, u32 fontSize,
                                       u16 fontFace, int lockGroup, bool isNoBreakHyphenReplaced);
    void ResetTextureCache();
    void ChangeFontListOrder(u32 fontFace, u32* pOrders);
    u32 AcquireFontFaceContainingGlyph(u32 code);
    bool IsGlyphExistInFont(u32 code, u16 fontFace);
    bool IsBorderEffectEnabled(u16 fontFace) const;
    u32 CountPlottingGlyph(const u16* pCodes, u32 codeLength, u32 fontSize, u16 fontFace);
    const FontMetrics& GetFontMetrics(u16 fontFace) const;
    int CalculateCharWidth(u32 code, u32 fontSize, u16 fontFace);
    u32 GetCoreId() const;
    void SetFontFaceNoPlot(u32 fontFace, u32 coreId);
    int CalculateKerning(u32 code0, u32 code1, u32 fontSize, u16 fontFace);
    GlyphNode* FindGlyphNode(u32 code, u32 fontSize, u16 fontFace);
    u32 CountUnusableGlyph(const u16* pCodes, u32 codeLength, u32 fontSize, u16 fontFace);
    bool CheckCharCodeRange(u32 innerFontFace, u32 code) const;

    const u8* GetTextureBitMap() const { return m_pTextureBitMap; }
    u32 GetTextureCacheWidth() const { return m_TextureCacheWidth; }
    u32 GetTextureCacheHeight() const { return m_TextureCacheHeight; }
    const TextureObject* GetTextureObject() const { return &m_TextureObject; }

private:
    u32 GetCoreCount() const { return m_IsMultiCoreEnabled ? m_CoreCount : 1; }
    char* GetFontName(u32 index) const { return &m_pFontNameBuffer[index * FontNameLength]; }

    fontll::ScalableFontEngine* m_pFontEngine;
    fontll::ScalableFontEngine* m_pFontEngineNoPlot;
    char* m_pFontNameBuffer;
    u8* m_pTextureBitMap;
    u32 m_TextureCacheWidth;
    u32 m_TextureCacheHeight;
    GlyphTreeMap m_GlyphTreeMap;
    GlyphList m_NeedPlotGlyphList;
    GlyphList m_NeedEraseGlyphList;
    GlyphList m_NotInFontGlyphList;
    LineInfo m_LineInfos[LineCountMax];
    u32 m_LineCurrentPos;
    u32 m_FontFaceCount;
    u8 m_InnerFontCounts[FontFaceCountMax];
    u8* m_pInnerFontFaceTables[FontFaceCountMax];
    u8 m_InnerFontFaceTable[FontFaceCountMax * InnerFontCountMax];
    u32 m_CurrentFontFace;
    u32 m_CurrentFontFacesNoPlot[CoreCountMax];
    u32 m_NoSpaceError;
    bool m_IsFsError;
    bool m_IsMultiCoreEnabled;
    u32 m_CoreCount;
    GetCoreIdFunction m_pGetCoreIdFunction;
    CalculateLineKindFunction m_pCalculateLineKindFunction;
    CalculateLineHeightFunction m_pCalculateLineHeightFunction;
    FontMetrics* m_pFontMetrics;
    int* m_pBoldWeights;
    u8* m_pBorderWidths;
    bool* m_pIsWidthFromBoundingBox;
    s16* m_pLetterSpacings;
    bool* m_pIsFixedWidth;
    s16* m_pFixedWidths;
    int* m_pCharCodeRangeCounts;
    u32 (*m_pCharCodeRangeFirsts)[CharCodeRangeCountMax];
    u32 (*m_pCharCodeRangeLasts)[CharCodeRangeCountMax];
    fontll::OtfKerningTable** m_pOtfKerningTables;
    bool m_IsDrawingAdvanceWidthUsed;
    nn::gfx::Texture m_Texture;
    nn::gfx::TextureView m_TextureView;
    nn::gfx::MemoryPool m_MemoryPool;
    nn::gfx::MemoryPool* m_pActiveMemoryPool;
    ptrdiff_t m_ActiveMemoryPoolOffset;
    PlacementTextureObject m_TextureObject;
};
static_assert(sizeof(TextureCache::InitializeArg) == 0x170e0);
static_assert(sizeof(TextureCache) == 0x1ec8);

}  // namespace font
}  // namespace nn
