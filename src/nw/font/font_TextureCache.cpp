#include <nn/font/font_TextureCache.h>

#include <cstring>
#include <new>
#include <nn/gfx/gfx_Enum.h>
#include <nn/gfx/gfx_TextureInfo.h>
#include <nn/os.h>
#include <nn/util.h>

namespace nn {
namespace font {

namespace {

typedef nn::gfx::detail::TextureImpl<nn::gfx::ApiVariationNvn8> TextureImpl;
typedef nn::gfx::detail::TextureViewImpl<nn::gfx::ApiVariationNvn8> TextureViewImpl;
typedef nn::gfx::detail::MemoryPoolImpl<nn::gfx::ApiVariationNvn8> MemoryPoolImpl;

/**
 * Converts one UTF-8 character to UTF-32.
 * @param pCharacter UTF-8 character
 * @return UTF-32 code
 */
u32 ConvertCharacterUtf8ToUtf32(const char* pCharacter) {
    u32 code = 0;
    nn::util::ConvertCharacterUtf8ToUtf32(&code, pCharacter);
    return code;
}

/**
 * Sets up texture info for the cache texture.
 * @param pInfo texture info to set up
 * @param width texture width
 * @param height texture height
 */
void InitializeTextureInfo(nn::gfx::TextureInfo* pInfo, int width, int height) {
    pInfo->SetDefault();
    pInfo->SetWidth(width);
    pInfo->SetHeight(height);
    pInfo->SetDepth(1);
    pInfo->SetArrayLength(1);
    pInfo->SetImageStorageDimension(nn::gfx::ImageStorageDimension_2d);
    pInfo->SetImageFormat(nn::gfx::ImageFormat_R8_Unorm);
    pInfo->SetGpuAccessFlags(nn::gfx::GpuAccess_Texture);
    pInfo->SetTileMode(nn::gfx::TileMode_Linear);
    pInfo->SetMipCount(1);
}

}  // namespace

/**
 * Sets the default initialization parameters.
 */
void TextureCache::InitializeArg::SetDefault() {
    textureCacheWidth = TextureCacheSizeDefault;
    textureCacheHeight = TextureCacheSizeDefault;
    pAllocateFunction = nullptr;
    pUserDataForAllocateFunction = nullptr;
    for (int i = 0; i < FontFaceCountMax; i++) {
        for (int j = 0; j < InnerFontCountMax; j++) {
            pFontDatas[i][j] = nullptr;
            fontDataSizes[i][j] = 0;
            boldWeights[i][j] = 0.0f;
            borderWidths[i][j] = 0;
            scaleWidths[i][j] = 1.0f;
            scaleHeights[i][j] = 1.0f;
            fontDataTypes[i][j] = FontDataType_Ttf;
            ignorePalt[i][j] = false;
            isWidthFromBoundingBox[i][j] = false;
            letterSpacings[i][j] = 0;
            isFixedWidth[i][j] = false;
            fixedWidths[i][j] = 0;
            baselineOffsets[i][j] = 0;
            overwrittenAscents[i][j] = 0;
            overwrittenDescents[i][j] = 0;
            charCodeRangeCounts[i][j] = 0;
            for (int k = 0; k < CharCodeRangeCountMax; k++) {
                charCodeRangeFirsts[i][j][k] = 0;
                charCodeRangeLasts[i][j][k] = 0;
            }

            fontIndexes[i][j] = 0;
        }

        innerFontCounts[i] = 1;
    }

    fontFaceCount = 1;
    workMemorySize = WorkMemorySizeDefault;
    noPlotWorkMemorySize = NoPlotWorkMemorySizeDefault;
    isNoPlotWorkMemorySizeShared = false;
    glyphNodeCountMax = GlyphNodeCountMaxDefault;
    isMultiCoreEnabled = false;
    coreCount = 1;
    pGetCoreIdFunction = nullptr;
    pCalculateLineKindFunction = nullptr;
    pCalculateLineHeightFunction = nullptr;
    isAutoHintEnabled = false;
    isDrawingAdvanceWidthUsed = true;
}

/**
 * Constructs an uninitialized texture cache.
 */
TextureCache::TextureCache()
    : m_pFontEngine(nullptr), m_pFontEngineNoPlot(nullptr), m_pFontNameBuffer(nullptr),
      m_pTextureBitMap(nullptr), m_TextureCacheWidth(0), m_TextureCacheHeight(0),
      m_LineCurrentPos(0), m_FontFaceCount(0), m_CurrentFontFace(0), m_NoSpaceError(0),
      m_IsFsError(false), m_IsMultiCoreEnabled(false), m_CoreCount(1),
      m_pGetCoreIdFunction(nullptr), m_pCalculateLineKindFunction(nullptr),
      m_pCalculateLineHeightFunction(nullptr), m_pFontMetrics(nullptr), m_pBoldWeights(nullptr),
      m_pBorderWidths(nullptr), m_pIsWidthFromBoundingBox(nullptr), m_pLetterSpacings(nullptr),
      m_pIsFixedWidth(nullptr), m_pFixedWidths(nullptr), m_pCharCodeRangeCounts(nullptr),
      m_pCharCodeRangeFirsts(nullptr), m_pCharCodeRangeLasts(nullptr),
      m_pOtfKerningTables(nullptr), m_IsDrawingAdvanceWidthUsed(true),
      m_pActiveMemoryPool(nullptr), m_ActiveMemoryPoolOffset(0) {
    for (int i = 0; i < FontFaceCountMax; i++) {
        m_InnerFontCounts[i] = 0;
        m_pInnerFontFaceTables[i] = nullptr;
    }

    for (int i = 0; i < FontFaceCountMax * InnerFontCountMax; i++) {
        m_InnerFontFaceTable[i] = 0;
    }
}

/**
 * Destroys the texture cache.
 */
TextureCache::~TextureCache() {}

/**
 * Sets up memory pool info for the cache texture.
 * @param pInfo memory pool info to set up
 */
void TextureCache::SetMemoryPoolInfo(nn::gfx::MemoryPoolInfo* pInfo) {
    pInfo->SetDefault();
    pInfo->SetMemoryPoolProperty(nn::gfx::MemoryPoolProperty_CpuUncached |
                                 nn::gfx::MemoryPoolProperty_GpuCached);
}

/**
 * Calculates the memory pool alignment required by the cache texture.
 * @param pDevice gfx device
 * @param width texture width
 * @param height texture height
 * @return alignment
 */
size_t TextureCache::CalculateMemoryPoolAlignment(nn::gfx::Device* pDevice, int width,
                                                  int height) {
    nn::gfx::TextureInfo info;
    InitializeTextureInfo(&info, width, height);

    nn::gfx::MemoryPoolInfo memoryPoolInfo;
    SetMemoryPoolInfo(&memoryPoolInfo);

    size_t textureAlignment = TextureImpl::CalculateMipDataAlignment(pDevice, info);
    size_t memoryPoolAlignment = MemoryPoolImpl::GetPoolMemoryAlignment(pDevice, memoryPoolInfo);
    return textureAlignment < memoryPoolAlignment ? memoryPoolAlignment : textureAlignment;
}

/**
 * Calculates the memory pool size required by the cache texture.
 * @param pDevice gfx device
 * @param width texture width
 * @param height texture height
 * @return size
 */
size_t TextureCache::CalculateMemoryPoolSize(nn::gfx::Device* pDevice, int width, int height) {
    nn::gfx::MemoryPoolInfo memoryPoolInfo;
    SetMemoryPoolInfo(&memoryPoolInfo);
    int granularity =
        static_cast<int>(MemoryPoolImpl::GetPoolMemorySizeGranularity(pDevice, memoryPoolInfo));
    return (width * height + granularity - 1) & -granularity;
}

void TextureCache::Initialize(nn::gfx::Device* pDevice, const InitializeArg& rArg,
                              nn::gfx::MemoryPool* pMemoryPool, ptrdiff_t memoryPoolOffset,
                              size_t memoryPoolSize) {
    if ((rArg.textureCacheWidth & (rArg.textureCacheWidth - 1)) != 0) {
        return;
    }

    if (rArg.pAllocateFunction == nullptr) {
        return;
    }

    if (rArg.fontFaceCount < 1 || rArg.fontFaceCount > FontFaceCountMax) {
        return;
    }

    int fontCount = InitializeFontFaceTable(rArg);
    m_FontFaceCount = rArg.fontFaceCount;
    m_IsMultiCoreEnabled = rArg.isMultiCoreEnabled;
    m_CoreCount = rArg.coreCount;
    m_pGetCoreIdFunction = rArg.pGetCoreIdFunction == nullptr ? nn::os::GetCurrentCoreNumber :
                                                                 rArg.pGetCoreIdFunction;
    m_pCalculateLineKindFunction = rArg.pCalculateLineKindFunction == nullptr ?
                                       GlyphNode::CalculateLineKind :
                                       rArg.pCalculateLineKindFunction;
    m_pCalculateLineHeightFunction = rArg.pCalculateLineHeightFunction == nullptr ?
                                         GlyphNode::CalculateLineHeight :
                                         rArg.pCalculateLineHeightFunction;

    m_pFontNameBuffer = static_cast<char*>(rArg.pAllocateFunction(
        fontCount * FontNameLength, 4, rArg.pUserDataForAllocateFunction));
    m_pFontMetrics = static_cast<FontMetrics*>(rArg.pAllocateFunction(
        sizeof(FontMetrics) * fontCount, 4, rArg.pUserDataForAllocateFunction));
    m_pBoldWeights = static_cast<int*>(rArg.pAllocateFunction(
        sizeof(int) * fontCount, 4, rArg.pUserDataForAllocateFunction));
    m_pBorderWidths = static_cast<u8*>(rArg.pAllocateFunction(
        sizeof(u8) * fontCount, 4, rArg.pUserDataForAllocateFunction));
    m_pIsWidthFromBoundingBox = static_cast<bool*>(rArg.pAllocateFunction(
        sizeof(bool) * fontCount, 4, rArg.pUserDataForAllocateFunction));
    m_pLetterSpacings = static_cast<s16*>(rArg.pAllocateFunction(
        sizeof(s16) * fontCount, 4, rArg.pUserDataForAllocateFunction));
    m_pIsFixedWidth = static_cast<bool*>(rArg.pAllocateFunction(
        sizeof(bool) * fontCount, 4, rArg.pUserDataForAllocateFunction));
    m_pFixedWidths = static_cast<s16*>(rArg.pAllocateFunction(
        sizeof(s16) * fontCount, 4, rArg.pUserDataForAllocateFunction));
    m_pCharCodeRangeCounts = static_cast<int*>(rArg.pAllocateFunction(
        sizeof(int) * fontCount, 4, rArg.pUserDataForAllocateFunction));
    m_pCharCodeRangeFirsts = static_cast<u32(*)[CharCodeRangeCountMax]>(rArg.pAllocateFunction(
        sizeof(u32) * CharCodeRangeCountMax * fontCount, 4, rArg.pUserDataForAllocateFunction));
    m_pCharCodeRangeLasts = static_cast<u32(*)[CharCodeRangeCountMax]>(rArg.pAllocateFunction(
        sizeof(u32) * CharCodeRangeCountMax * fontCount, 4, rArg.pUserDataForAllocateFunction));
    m_pOtfKerningTables = static_cast<fontll::OtfKerningTable**>(
        rArg.pAllocateFunction(sizeof(fontll::OtfKerningTable*) * fontCount, 4,
                               rArg.pUserDataForAllocateFunction));

    m_TextureCacheWidth = rArg.textureCacheWidth;
    m_TextureCacheHeight = rArg.textureCacheHeight;
    m_CurrentFontFace = 0xffffffff;
    m_LineCurrentPos = 0;
    m_NoSpaceError = NoSpaceError_NoError;
    m_IsFsError = false;
    for (int i = 0; i < CoreCountMax; i++) {
        m_CurrentFontFacesNoPlot[i] = 0xffffffff;
    }

    m_IsDrawingAdvanceWidthUsed = rArg.isDrawingAdvanceWidthUsed;

    m_pFontEngine = static_cast<fontll::ScalableFontEngine*>(rArg.pAllocateFunction(
        sizeof(fontll::ScalableFontEngine), 4, rArg.pUserDataForAllocateFunction));
    new (m_pFontEngine) fontll::ScalableFontEngine();
    m_pFontEngineNoPlot = static_cast<fontll::ScalableFontEngine*>(
        rArg.pAllocateFunction(sizeof(fontll::ScalableFontEngine) * GetCoreCount(), 4,
                               rArg.pUserDataForAllocateFunction));
    for (u32 i = 0; i < GetCoreCount(); i++) {
        new (&m_pFontEngineNoPlot[i]) fontll::ScalableFontEngine();
    }

    m_pFontEngine->Initialize(
        rArg.pAllocateFunction(rArg.workMemorySize, 4, rArg.pUserDataForAllocateFunction),
        rArg.workMemorySize);
    AssertFsError("Initialize");
    for (u32 i = 0; i < GetCoreCount(); i++) {
        if (rArg.isNoPlotWorkMemorySizeShared) {
            m_pFontEngineNoPlot[i].Initialize(
                rArg.pAllocateFunction(rArg.noPlotWorkMemorySize, 4,
                                       rArg.pUserDataForAllocateFunction),
                rArg.noPlotWorkMemorySize);
        } else {
            m_pFontEngineNoPlot[i].Initialize(
                rArg.pAllocateFunction(rArg.noPlotWorkMemorySize * fontCount, 4,
                                       rArg.pUserDataForAllocateFunction),
                rArg.noPlotWorkMemorySize * fontCount);
        }

        AssertFsErrorNoPlot("Initialize", i);
    }

    int fontIndex = 0;
    for (u32 face = 0; face < m_FontFaceCount; face++) {
        for (u32 inner = 0; inner < m_InnerFontCounts[face]; inner++) {
            const void* pFontData;
            if (rArg.fontDataTypes[face][inner] == FontDataType_Bfttf ||
                rArg.fontDataTypes[face][inner] == FontDataType_Bfotf) {
                pFontData = fontll::ScalableFontEngineHelper::Decode(
                    rArg.pFontDatas[face][inner], rArg.fontDataSizes[face][inner]);
            } else {
                pFontData = rArg.pFontDatas[face][inner];
            }

            m_pBoldWeights[fontIndex] = static_cast<int>(rArg.boldWeights[face][inner] * 65536.0f);
            m_pBorderWidths[fontIndex] = rArg.borderWidths[face][inner];
            m_pIsWidthFromBoundingBox[fontIndex] = rArg.isWidthFromBoundingBox[face][inner];
            m_pLetterSpacings[fontIndex] = rArg.letterSpacings[face][inner];
            m_pIsFixedWidth[fontIndex] = rArg.isFixedWidth[face][inner];
            m_pFixedWidths[fontIndex] = rArg.fixedWidths[face][inner];
            m_pCharCodeRangeCounts[fontIndex] = rArg.charCodeRangeCounts[face][inner];
            for (int i = 0; i < CharCodeRangeCountMax; i++) {
                m_pCharCodeRangeFirsts[fontIndex][i] = rArg.charCodeRangeFirsts[face][inner][i];
                m_pCharCodeRangeLasts[fontIndex][i] = rArg.charCodeRangeLasts[face][inner][i];
            }

            m_pFontEngine->LoadFont(GetFontName(fontIndex), pFontData,
                                    rArg.fontIndexes[face][inner], FontNameLength);
            AssertFsError("LoadFont");
            for (u32 i = 0; i < GetCoreCount(); i++) {
                m_pFontEngineNoPlot[i].LoadFont(GetFontName(fontIndex),
                                                pFontData, rArg.fontIndexes[face][inner],
                                                FontNameLength);
                AssertFsErrorNoPlot("LoadFont", i);
            }

            SetFontFace(fontIndex);
            fontll::Metrics metrics;
            m_pFontEngine->GetFontMetrics(&metrics);
            AssertFsError("GetFontMetrics");
            if (rArg.overwrittenAscents[face][inner] != 0) {
                metrics.boundingBoxMaxY = rArg.overwrittenAscents[face][inner];
                metrics.ascender = rArg.overwrittenAscents[face][inner];
            }

            if (rArg.overwrittenDescents[face][inner] != 0) {
                metrics.boundingBoxMinY = -rArg.overwrittenDescents[face][inner];
                metrics.descender = rArg.overwrittenDescents[face][inner];
            }

            m_pFontMetrics[fontIndex].boundingBoxAscentRatio =
                static_cast<float>(metrics.boundingBoxMaxY) / metrics.unitsPerEm;
            m_pFontMetrics[fontIndex].boundingBoxHeightRatio =
                static_cast<float>(metrics.boundingBoxMaxY - metrics.boundingBoxMinY) /
                metrics.unitsPerEm;
            m_pFontMetrics[fontIndex].ascentRatio =
                static_cast<float>(metrics.ascender) / metrics.unitsPerEm;
            m_pFontMetrics[fontIndex].heightRatio =
                static_cast<float>(metrics.ascender + metrics.descender) / metrics.unitsPerEm;
            m_pFontMetrics[fontIndex].scaleWidth = rArg.scaleWidths[face][inner];
            m_pFontMetrics[fontIndex].scaleHeight = rArg.scaleHeights[face][inner];
            m_pFontMetrics[fontIndex].baselineOffset = rArg.baselineOffsets[face][inner];

            fontll::OtfKerningTable* pKerningTable = nullptr;
            if ((rArg.fontDataTypes[face][inner] == FontDataType_Otf ||
                 rArg.fontDataTypes[face][inner] == FontDataType_Bfotf) &&
                !rArg.isWidthFromBoundingBox[face][inner] && !rArg.isFixedWidth[face][inner]) {
                pKerningTable = m_pFontEngine->InitializeOtfKerningTable(
                    rArg.pAllocateFunction, rArg.pUserDataForAllocateFunction,
                    rArg.ignorePalt[face][inner]);
            }

            m_pOtfKerningTables[fontIndex] = pKerningTable;
            fontIndex++;
        }
    }

    int fontIndexBase = 0;
    for (u32 face = 0; face < m_FontFaceCount; face++) {
        float boundingBoxAscentRatio = 0.0f;
        float boundingBoxHeightRatio = 0.0f;
        float ascentRatio = 0.0f;
        float heightRatio = 0.0f;
        for (int inner = 0; inner < m_InnerFontCounts[face]; inner++) {
            const FontMetrics& metrics = m_pFontMetrics[fontIndexBase + inner];
            if (boundingBoxHeightRatio < metrics.boundingBoxHeightRatio) {
                boundingBoxHeightRatio = metrics.boundingBoxHeightRatio;
                boundingBoxAscentRatio = metrics.boundingBoxAscentRatio;
                heightRatio = metrics.heightRatio;
                ascentRatio = metrics.ascentRatio;
            }
        }

        for (int inner = 0; inner < m_InnerFontCounts[face]; inner++) {
            m_pFontMetrics[fontIndexBase + inner].boundingBoxAscentRatio = boundingBoxAscentRatio;
            m_pFontMetrics[fontIndexBase + inner].boundingBoxHeightRatio = boundingBoxHeightRatio;
            m_pFontMetrics[fontIndexBase + inner].ascentRatio = ascentRatio;
            m_pFontMetrics[fontIndexBase + inner].heightRatio = heightRatio;
        }

        fontIndexBase += m_InnerFontCounts[face];
    }

    u32 textureWidth = m_TextureCacheWidth;
    u32 textureHeight = m_TextureCacheHeight;
    m_TextureObject.Set(m_pTextureBitMap, 8, textureWidth, textureHeight, 1, true);

    nn::gfx::TextureInfo info;
    InitializeTextureInfo(&info, m_TextureCacheWidth, m_TextureCacheHeight);

    size_t alignment =
        CalculateMemoryPoolAlignment(pDevice, m_TextureCacheWidth, m_TextureCacheHeight);
    size_t size = CalculateMemoryPoolSize(pDevice, m_TextureCacheWidth, m_TextureCacheHeight);
    if (pMemoryPool != nullptr) {
        m_pTextureBitMap = static_cast<u8*>(static_cast<MemoryPoolImpl*>(pMemoryPool)->Map()) +
                           memoryPoolOffset;
        std::memset(m_pTextureBitMap, 0, size);
        static_cast<MemoryPoolImpl*>(pMemoryPool)->Unmap();
        static_cast<TextureImpl&>(m_Texture).Initialize(pDevice, info, pMemoryPool,
                                                        memoryPoolOffset, memoryPoolSize);
        m_pActiveMemoryPool = pMemoryPool;
        m_ActiveMemoryPoolOffset = memoryPoolOffset;
    } else {
        u32 textureSize = textureWidth * textureHeight;
        m_pTextureBitMap = static_cast<u8*>(
            rArg.pAllocateFunction(size, alignment, rArg.pUserDataForAllocateFunction));
        std::memset(m_pTextureBitMap, 0, size);
        nn::gfx::MemoryPoolInfo memoryPoolInfo;
        memoryPoolInfo.SetDefault();
        memoryPoolInfo.SetMemoryPoolProperty(nn::gfx::MemoryPoolProperty_CpuUncached |
                                             nn::gfx::MemoryPoolProperty_GpuCached);
        memoryPoolInfo.SetPoolMemory(m_pTextureBitMap, size);
        m_MemoryPool.Initialize(pDevice, memoryPoolInfo);
        static_cast<TextureImpl&>(m_Texture).Initialize(pDevice, info, &m_MemoryPool, 0,
                                                        textureSize);
        m_pActiveMemoryPool = &m_MemoryPool;
        m_ActiveMemoryPoolOffset = 0;
    }

    nn::gfx::TextureViewInfo viewInfo;
    viewInfo.SetDefault();
    viewInfo.SetImageFormat(nn::gfx::ImageFormat_R8_Unorm);
    viewInfo.SetChannelMapping(nn::gfx::ChannelMapping_One, nn::gfx::ChannelMapping_One,
                               nn::gfx::ChannelMapping_One, nn::gfx::ChannelMapping_Red);
    viewInfo.SetTexturePtr(&m_Texture);
    viewInfo.SetImageDimension(nn::gfx::ImageDimension_2d);
    static_cast<TextureViewImpl&>(m_TextureView).Initialize(pDevice, viewInfo);

    m_GlyphTreeMap.Initialize(rArg.pAllocateFunction, rArg.pUserDataForAllocateFunction,
                              rArg.glyphNodeCountMax);
    if (rArg.isAutoHintEnabled) {
        m_pFontEngine->SetAutoHint(true);
    }
}

/**
 * Builds the font face to inner font table.
 * @param rArg initialization parameters
 * @return total number of inner fonts, or 0 if a count is invalid
 */
int TextureCache::InitializeFontFaceTable(const InitializeArg& rArg) {
    int fontCount = 0;
    for (u32 i = 0; i < rArg.fontFaceCount; i++) {
        if (rArg.innerFontCounts[i] < 1 || rArg.innerFontCounts[i] > InnerFontCountMax) {
            return 0;
        }

        fontCount += rArg.innerFontCounts[i];
    }

    u8 offset = 0;
    for (u32 i = 0; i < FontFaceCountMax; i++) {
        if (i >= rArg.fontFaceCount) {
            m_InnerFontCounts[i] = 0;
            m_pInnerFontFaceTables[i] = nullptr;
        } else {
            m_InnerFontCounts[i] = rArg.innerFontCounts[i];
            m_pInnerFontFaceTables[i] = &m_InnerFontFaceTable[offset];
            offset += rArg.innerFontCounts[i];
        }
    }

    for (int i = 0; i < FontFaceCountMax * InnerFontCountMax; i++) {
        m_InnerFontFaceTable[i] = i;
    }

    return fontCount;
}

/**
 * Checks the error state of the plotting font engine.
 * @param pApiName name of the checked API
 */
void TextureCache::AssertFsError(const char* pApiName) const {
    m_pFontEngine->GetError();
}

/**
 * Checks the error state of a per-core font engine.
 * @param pApiName name of the checked API
 * @param coreId core index
 */
void TextureCache::AssertFsErrorNoPlot(const char* pApiName, u32 coreId) const {
    m_pFontEngineNoPlot[coreId].GetError();
}

/**
 * Selects an inner font on the plotting font engine.
 * @param fontFace inner font index
 */
void TextureCache::SetFontFace(u32 fontFace) {
    if (m_CurrentFontFace == fontFace) {
        return;
    }

    m_pFontEngine->SetFont(GetFontName(fontFace));
    AssertFsError("SetFont");
    m_pFontEngine->SetBoldWeight(m_pBoldWeights[fontFace]);
    AssertFsError("SetBoldWeight");
    if (m_pBorderWidths[fontFace] != 0) {
        m_pFontEngine->SetOutlineWidth(m_pBorderWidths[fontFace]);
        m_pFontEngine->SetFlags(fontll::ScalableFontEngine::Flags_Outlined);
    } else {
        m_pFontEngine->SetFlags(fontll::ScalableFontEngine::Flags_NoEffect);
    }

    AssertFsError("SetFlags");
    m_CurrentFontFace = fontFace;
}

/**
 * Releases the font engines, kerning tables, buffers and texture.
 * @param pDevice gfx device
 * @param pFreeFunction deallocator
 * @param pUserData user data passed to the deallocator
 */
void TextureCache::Finalize(nn::gfx::Device* pDevice, FreeFunction pFreeFunction,
                            void* pUserData) {
    void* pWorkMemory = m_pFontEngine->GetPointerToWorkBuffer();
    void* pNoPlotWorkMemories[CoreCountMax] = {};
    for (u32 i = 0; i < GetCoreCount(); i++) {
        pNoPlotWorkMemories[i] = m_pFontEngineNoPlot[i].GetPointerToWorkBuffer();
    }

    int fontIndex = 0;
    for (u32 face = 0; face < m_FontFaceCount; face++) {
        for (u32 inner = 0; inner < m_InnerFontCounts[face]; inner++) {
            if (m_pOtfKerningTables[fontIndex] != nullptr) {
                m_pFontEngine->FinalizeOtfKerningTable(m_pOtfKerningTables[fontIndex],
                                                       pFreeFunction, pUserData);
                m_pOtfKerningTables[fontIndex] = nullptr;
            }

            fontIndex++;
        }
    }

    m_pFontEngine->Finalize();
    AssertFsError("Finalize");
    for (u32 i = 0; i < GetCoreCount(); i++) {
        m_pFontEngineNoPlot[i].Finalize();
        AssertFsErrorNoPlot("Finalize", i);
    }

    pFreeFunction(m_pFontEngine, pUserData);
    m_pFontEngine = nullptr;
    pFreeFunction(m_pFontEngineNoPlot, pUserData);
    m_pFontEngineNoPlot = nullptr;
    pFreeFunction(pWorkMemory, pUserData);
    for (u32 i = 0; i < GetCoreCount(); i++) {
        pFreeFunction(pNoPlotWorkMemories[i], pUserData);
    }

    for (int i = 0; i < LineCountMax; i++) {
        m_LineInfos[i].list.clear();
    }

    m_NeedPlotGlyphList.clear();
    m_NeedEraseGlyphList.clear();
    m_NotInFontGlyphList.clear();
    m_GlyphTreeMap.Finalize(pFreeFunction, pUserData);

    pFreeFunction(m_pFontNameBuffer, pUserData);
    m_pFontNameBuffer = nullptr;
    pFreeFunction(m_pFontMetrics, pUserData);
    m_pFontMetrics = nullptr;
    pFreeFunction(m_pBoldWeights, pUserData);
    m_pBoldWeights = nullptr;
    pFreeFunction(m_pBorderWidths, pUserData);
    m_pBorderWidths = nullptr;
    pFreeFunction(m_pIsWidthFromBoundingBox, pUserData);
    m_pIsWidthFromBoundingBox = nullptr;
    pFreeFunction(m_pLetterSpacings, pUserData);
    m_pLetterSpacings = nullptr;
    pFreeFunction(m_pIsFixedWidth, pUserData);
    m_pIsFixedWidth = nullptr;
    pFreeFunction(m_pFixedWidths, pUserData);
    m_pFixedWidths = nullptr;
    pFreeFunction(m_pCharCodeRangeCounts, pUserData);
    m_pCharCodeRangeCounts = nullptr;
    pFreeFunction(m_pCharCodeRangeFirsts, pUserData);
    m_pCharCodeRangeFirsts = nullptr;
    pFreeFunction(m_pCharCodeRangeLasts, pUserData);
    m_pCharCodeRangeLasts = nullptr;
    pFreeFunction(m_pOtfKerningTables, pUserData);
    m_pOtfKerningTables = nullptr;

    if (m_TextureObject.IsInitialized()) {
        static_cast<TextureViewImpl&>(m_TextureView).Finalize(pDevice);
        static_cast<TextureImpl&>(m_Texture).Finalize(pDevice);
        if (m_pActiveMemoryPool == &m_MemoryPool) {
            m_MemoryPool.Finalize(pDevice);
        }
    }

    if (m_pActiveMemoryPool == &m_MemoryPool) {
        pFreeFunction(m_pTextureBitMap, pUserData);
        m_pTextureBitMap = nullptr;
    }
}

/**
 * Registers the cache texture view to a descriptor pool.
 * @param pRegisterFunction registration function
 * @param pUserData user data passed to the function
 */
void TextureCache::RegisterTextureViewToDescriptorPool(RegisterTextureViewSlot pRegisterFunction,
                                                       void* pUserData) {
    pRegisterFunction(&m_TextureObject.GetDescriptorSlot(), m_TextureView, pUserData);
}

/**
 * Unregisters the cache texture view from a descriptor pool.
 * @param pUnregisterFunction unregistration function
 * @param pUserData user data passed to the function
 */
void TextureCache::UnregisterTextureViewFromDescriptorPool(
    UnregisterTextureViewSlot pUnregisterFunction, void* pUserData) {
    pUnregisterFunction(&m_TextureObject.GetDescriptorSlot(), m_TextureView, pUserData);
}

/**
 * Registers a glyph to be plotted into the cache.
 * @param code character code
 * @param fontSize font size
 * @param fontFace font face
 * @param lockGroup lock group index, or a negative value
 * @param isNoBreakHyphenReplaced whether U+2011 is drawn as a hyphen
 * @return whether the glyph still has to be plotted
 */
bool TextureCache::RegisterGlyph(u32 code, u32 fontSize, u16 fontFace, int lockGroup,
                                 bool isNoBreakHyphenReplaced) {
    if ((code == 0x2011) & isNoBreakHyphenReplaced) {
        code = '-';
    }

    u32 innerFontFace;
    if (!GetInnerFontFace(&innerFontFace, fontFace, code)) {
        return false;
    }

    GlyphNode* pNode = m_GlyphTreeMap.Find(code, fontSize, innerFontFace);
    if (pNode != nullptr) {
        pNode->SetFlag(GlyphNode::FlagBit_Requested);
        if (lockGroup >= 0) {
            pNode->m_LockGroup |= 1 << lockGroup;
        }

        return pNode->IsFlagOn(GlyphNode::FlagBit_NotPlotted);
    }

    pNode = m_GlyphTreeMap.Insert(code, fontSize, innerFontFace);
    if (pNode != nullptr) {
        pNode->SetFlag(GlyphNode::FlagBit_Requested | GlyphNode::FlagBit_NotPlotted);
        if (lockGroup >= 0) {
            pNode->m_LockGroup |= 1 << lockGroup;
        }

        m_NeedPlotGlyphList.push_back(*pNode);
        return true;
    }

    if (EraseNotInFontGlyphs() > 0) {
        return RegisterGlyph(code, fontSize, innerFontFace, lockGroup, false);
    }

    m_NoSpaceError = NoSpaceError_GlyphNodeSpaceShortage;
    return false;
}

/**
 * Finds the first inner font of a font face that contains a character.
 * @param pInnerFontFace receives the inner font index
 * @param fontFace font face
 * @param code character code
 * @return whether an inner font was found
 */
bool TextureCache::GetInnerFontFace(u32* pInnerFontFace, u32 fontFace, u32 code) {
    const u8* pInnerFontFaces = m_pInnerFontFaceTables[fontFace];
    for (u32 i = 0; i < m_InnerFontCounts[fontFace]; i++) {
        if (CheckCharCodeRange(pInnerFontFaces[i], code)) {
            u32 coreId = GetCoreId();
            SetFontFaceNoPlot(pInnerFontFaces[i], coreId);
            if (m_pFontEngineNoPlot[coreId].CheckGlyphExist(code)) {
                *pInnerFontFace = pInnerFontFaces[i];
                return true;
            }
        }
    }

    *pInnerFontFace = 0;
    return false;
}

/**
 * Erases unused nodes of glyphs missing from the fonts.
 * @return number of erased nodes
 */
u32 TextureCache::EraseNotInFontGlyphs() {
    u32 count = 0;
    GlyphList::iterator it = m_NotInFontGlyphList.begin();
    while (it != m_NotInFontGlyphList.end()) {
        GlyphList::iterator next = it;
        ++next;
        if (!it->IsFlagOn(GlyphNode::FlagBit_Requested | GlyphNode::FlagBit_UsedInLastFrame)) {
            GlyphNode& node = *it;
            m_NotInFontGlyphList.erase(it);
            m_GlyphTreeMap.Erase(node.GetCode(), node.GetFontSize(), node.GetFontFace());
            count++;
        }

        it = next;
    }

    return count;
}

/**
 * Registers the glyphs of a UTF-16 string.
 * @param pCodes string
 * @param codeLength maximum number of characters
 * @param fontSize font size
 * @param fontFace font face
 * @param lockGroup lock group index, or a negative value
 * @param isNoBreakHyphenReplaced whether U+2011 is drawn as a hyphen
 * @return number of glyphs that still have to be plotted
 */
u32 TextureCache::RegisterGlyphsWithLength(const u16* pCodes, u32 codeLength, u32 fontSize,
                                           u16 fontFace, int lockGroup,
                                           bool isNoBreakHyphenReplaced) {
    u32 count = 0;
    for (u32 i = 0; i < codeLength; i++) {
        if (pCodes[i] == 0) {
            break;
        }

        count += RegisterGlyph(pCodes[i], fontSize, fontFace, lockGroup, isNoBreakHyphenReplaced);
    }

    return count;
}

/**
 * Registers the glyphs of a UTF-8 string.
 * @param pCodes string
 * @param codeLength length in bytes
 * @param fontSize font size
 * @param fontFace font face
 * @param lockGroup lock group index, or a negative value
 * @param isNoBreakHyphenReplaced whether U+2011 is drawn as a hyphen
 * @return number of glyphs that still have to be plotted
 */
u32 TextureCache::RegisterGlyphsWithLengthUtf8(const char* pCodes, u32 codeLength, u32 fontSize,
                                               u16 fontFace, int lockGroup,
                                               bool isNoBreakHyphenReplaced) {
    u32 count = 0;
    const char* pStart = pCodes;
    while (static_cast<u32>(pCodes - pStart) != codeLength) {
        char buffer[4] = {};
        nn::util::PickOutCharacterFromUtf8String(buffer, &pCodes);
        if (buffer[0] == 0) {
            break;
        }

        count += RegisterGlyph(ConvertCharacterUtf8ToUtf32(buffer), fontSize, fontFace, lockGroup,
                               isNoBreakHyphenReplaced);
    }

    return count;
}

void TextureCache::UpdateTextureCache() {
    GlyphList::iterator it = m_NeedPlotGlyphList.begin();
    while (it != m_NeedPlotGlyphList.end()) {
        GlyphList::iterator next = it;
        ++next;
        GlyphNode* pNode = &*it;
        m_NeedPlotGlyphList.erase(it);
        it = next;

        SetFontFace(pNode->GetFontFace());
        if (!m_pFontEngine->CheckGlyphExist(pNode->GetCode())) {
            pNode->SetFlag(GlyphNode::FlagBit_NotInFont);
            m_NotInFontGlyphList.push_back(*pNode);
            continue;
        }

        m_pFontEngine->SetScale(pNode->GetFontSize() << 16, 0, 0, pNode->GetFontSize() << 16);
        AssertFsError("SetScale");
        int baseline = static_cast<int>(
            m_pFontMetrics[pNode->GetFontFace()].boundingBoxAscentRatio * pNode->GetFontSize() +
            0.5f);
        fontll::GlyphMap* pGlyph = m_pFontEngine->AcquireGlyphmap(pNode->GetCode(), 8);
        AssertFsError("AcquireGlyphmap");

        u8 lineKind = m_pCalculateLineKindFunction(static_cast<u16>(pGlyph->height) + baseline -
                                                   static_cast<u16>(pGlyph->top));
        pNode->m_LineKind = lineKind;

        LineInfo* pLine = nullptr;
        for (u32 i = 0; i < m_LineCurrentPos; i++) {
            if (m_LineInfos[i].kind == lineKind &&
                m_LineInfos[i].currentX + pGlyph->width + 2 < m_TextureCacheWidth) {
                pLine = &m_LineInfos[i];
                break;
            }
        }

        if (pLine == nullptr) {
            if (m_LineCurrentPos >= LineCountMax) {
                m_NeedEraseGlyphList.push_back(*pNode);
                m_pFontEngine->ReleasesGlyph(pGlyph);
                AssertFsError("ReleasesGlyph");
                m_NoSpaceError = NoSpaceError_LineCountShortage;
                continue;
            }

            pLine = CreateNewLineImpl(lineKind);
        }

        if (pLine != nullptr) {
            pNode->m_CachePosX = pLine->currentX;
            pNode->m_CachePosY = pLine->y;
            pNode->m_LineNo = pLine->no;
            pLine->list.push_back(*pNode);
            pLine->currentX += pGlyph->width + 2;
        } else {
            GlyphNode* pEraseNode = FindAndReserveEraseGlyph(pNode->m_LineKind, pGlyph->width);
            if (pEraseNode == nullptr) {
                m_NeedEraseGlyphList.push_back(*pNode);
                m_pFontEngine->ReleasesGlyph(pGlyph);
                AssertFsError("ReleasesGlyph");
                m_NoSpaceError = NoSpaceError_TextureSpaceShortage;
                continue;
            }

            pNode->m_CachePosX = pEraseNode->m_CachePosX;
            pNode->m_CachePosY = pEraseNode->m_CachePosY;
            pNode->m_LineNo = pEraseNode->m_LineNo;
            pEraseNode->m_LineLink.LinkPrev(&pNode->m_LineLink);
        }

        int top = pGlyph->top;
        int offsetY = baseline - top;
        const FontMetrics& metrics = m_pFontMetrics[pNode->GetFontFace()];
        int posY = pNode->m_CachePosY;
        int startY = offsetY + posY;
        float scaleWidth = metrics.scaleWidth;
        float scaleHeight = metrics.scaleHeight;
        pNode->m_GlyphWidth = static_cast<int>(scaleWidth * pGlyph->width);
        pNode->m_GlyphHeight = static_cast<int>(scaleHeight * (offsetY + pGlyph->height));
        pNode->m_CacheWidth = pGlyph->width;
        pNode->m_CacheHeight = pGlyph->height + offsetY;

        int left;
        if (m_pIsWidthFromBoundingBox[pNode->GetFontFace()] && pGlyph->width != 0) {
            pNode->m_AdvanceX =
                static_cast<int>(scaleWidth * (m_pLetterSpacings[pNode->GetFontFace()] + pGlyph->width));
            left = m_pLetterSpacings[pNode->GetFontFace()] / 2;
        } else if (m_pIsFixedWidth[pNode->GetFontFace()]) {
            pNode->m_AdvanceX = static_cast<int>(scaleWidth * m_pFixedWidths[pNode->GetFontFace()]);
            left = (m_pFixedWidths[pNode->GetFontFace()] - pGlyph->width) / 2;
        } else {
            if (m_IsDrawingAdvanceWidthUsed) {
                pNode->m_AdvanceX = static_cast<int>(
                    scaleWidth * (pGlyph->advanceX + m_pLetterSpacings[pNode->GetFontFace()]));
            } else {
                s16 advanceX;
                s16 advanceY;
                int fixedAdvanceX;
                int fixedAdvanceY;
                m_pFontEngine->GetAdvance(&advanceX, &advanceY, &fixedAdvanceX, &fixedAdvanceY,
                                          pNode->GetCode(), 8);
                pNode->m_AdvanceX = static_cast<int>(
                    scaleWidth * (advanceX + m_pLetterSpacings[pNode->GetFontFace()]));
            }

            left = m_pLetterSpacings[pNode->GetFontFace()] / 2 + pGlyph->left;
        }

        pNode->m_LeftOffset = static_cast<int>(scaleWidth * left);
        pNode->m_BaselineOffset = static_cast<int>(
            scaleHeight * baseline +
            static_cast<float>(m_pFontMetrics[pNode->GetFontFace()].baselineOffset));

        u32 lineBytes = pGlyph->width + 2;
        u8* pBitMap = static_cast<u8*>(static_cast<MemoryPoolImpl*>(m_pActiveMemoryPool)->Map()) +
                      m_ActiveMemoryPoolOffset;
        for (int y = pNode->m_CachePosY; y < startY; y++) {
            std::memset(&pBitMap[(m_TextureCacheHeight - 1 - y) * m_TextureCacheWidth +
                                 pNode->m_CachePosX],
                        0, lineBytes);
        }

        int row = -startY > 0 ? -startY : 0;
        if (m_pBorderWidths[pNode->GetFontFace()] != 0) {
            m_pFontEngine->SetFlags(fontll::ScalableFontEngine::Flags_OutlinedFilled);
            fontll::GlyphMap* pBorderGlyph = m_pFontEngine->AcquireGlyphmap(pNode->GetCode(), 8);
            AssertFsError("AcquireGlyphmap");
            m_pFontEngine->SetFlags(fontll::ScalableFontEngine::Flags_Outlined);
            int height = pGlyph->height > pBorderGlyph->height ? pGlyph->height :
                                                                 pBorderGlyph->height;
            for (; row < height; row++) {
                u8* pLine = &pBitMap[(m_TextureCacheHeight - 1 - (row + startY)) *
                                         m_TextureCacheWidth +
                                     pNode->m_CachePosX];
                for (int x = 0; x < pGlyph->width; x++) {
                    int value =
                        row < pGlyph->height ? pGlyph->bits[row * pGlyph->width + x] : 0;
                    int borderValue =
                        row < pBorderGlyph->height ?
                            pBorderGlyph->bits[row * pBorderGlyph->width + x] >> 1 :
                            0;
                    pLine[x + 1] = value - borderValue;
                }

                pLine[0] = 0;
                pLine[pGlyph->width + 1] = 0;
            }

            m_pFontEngine->ReleasesGlyph(pGlyph);
            pGlyph = pBorderGlyph;
        } else {
            for (; row < pGlyph->height; row++) {
                u8* pLine = &pBitMap[(m_TextureCacheHeight - 1 - (row + startY)) *
                                         m_TextureCacheWidth +
                                     pNode->m_CachePosX];
                std::memcpy(&pLine[1], &pGlyph->bits[row * pGlyph->width], pGlyph->width);
                pLine[0] = 0;
                pLine[pGlyph->width + 1] = 0;
            }
        }

        u32 lineHeight = m_pCalculateLineHeightFunction(pNode->m_LineKind);
        for (u32 y = pGlyph->height; y < pGlyph->height + 2u && y < lineHeight; y++) {
            std::memset(&pBitMap[(m_TextureCacheHeight - 1 - (y + startY)) * m_TextureCacheWidth +
                                 pNode->m_CachePosX],
                        0, lineBytes);
        }

        static_cast<MemoryPoolImpl*>(m_pActiveMemoryPool)->Unmap();
        m_pFontEngine->ReleasesGlyph(pGlyph);
        AssertFsError("ReleasesGlyph");
    }
}

/**
 * Adds a line of the given kind below the last line.
 * @param lineKind line kind
 * @return the new line, or nullptr if the texture is full
 */
TextureCache::LineInfo* TextureCache::CreateNewLineImpl(u8 lineKind) {
    u32 y;
    if (m_LineCurrentPos == 0) {
        y = 0;
    } else {
        LineInfo& lastLine = m_LineInfos[m_LineCurrentPos - 1];
        y = lastLine.y + m_pCalculateLineHeightFunction(lastLine.kind) + 2;
    }

    if (m_pCalculateLineHeightFunction(lineKind) + y >= m_TextureCacheHeight) {
        return nullptr;
    }

    LineInfo* pLine = &m_LineInfos[m_LineCurrentPos];
    pLine->currentX = 0;
    pLine->y = y;
    pLine->kind = lineKind;
    pLine->no = m_LineCurrentPos;
    m_LineCurrentPos++;
    return pLine;
}

GlyphNode* TextureCache::FindAndReserveEraseGlyph(u8 lineKind, u16 glyphWidth) {
    for (u32 i = 0; i < m_LineCurrentPos; i++) {
        LineInfo& line = m_LineInfos[i];
        if (line.kind != lineKind) {
            continue;
        }

        GlyphLineList::iterator begin = line.list.begin();
        GlyphLineList::iterator end = line.list.end();
        int count = 0;
        GlyphLineList::iterator first = begin;
        for (GlyphLineList::iterator it = begin; it != end; ++it) {
            if (it->m_Flag != 0 || it->m_LockGroup != 0) {
                count = 0;
                continue;
            }

            u32 width = it->m_CacheWidth;
            if (count != 0) {
                width = it->m_CachePosX + width - first->m_CachePosX;
            } else {
                first = it;
            }

            int space = 0;
            if (first != begin) {
                GlyphLineList::iterator prev = first;
                do {
                    prev = GlyphLineList::iterator(prev.GetNode()->GetPrev());
                } while (prev->IsFlagOn(GlyphNode::FlagBit_Erase) && begin != prev);
                space = first->m_CachePosX - 2 - prev->m_CachePosX - prev->m_CacheWidth;
                width += space > 0 ? space : 0;
            }

            if (width >= glyphWidth) {
                GlyphLineList::iterator last = it;
                ++last;
                for (GlyphLineList::iterator eraseIt = first; eraseIt != last; ++eraseIt) {
                    if (!eraseIt->IsFlagOn(GlyphNode::FlagBit_Erase)) {
                        eraseIt->SetFlag(GlyphNode::FlagBit_Erase);
                        m_NeedEraseGlyphList.push_back(*eraseIt);
                    }
                }

                if (space > 0) {
                    first->m_CachePosX -= space;
                }

                return &*first;
            }

            count++;
        }
    }

    return nullptr;
}

/**
 * Erases the glyphs reserved for erasing and advances the usage flags.
 */
void TextureCache::CompleteTextureCache() {
    GlyphList::iterator it = m_NeedEraseGlyphList.begin();
    while (it != m_NeedEraseGlyphList.end()) {
        GlyphList::iterator next = it;
        ++next;
        GlyphNode* pNode = &*it;
        m_NeedEraseGlyphList.erase(it);
        if (pNode->IsFlagOn(GlyphNode::FlagBit_Erase)) {
            GlyphLineList& list = m_LineInfos[pNode->m_LineNo].list;
            list.erase(list.iterator_to(*pNode));
            u16 currentX;
            if (list.size() == 0) {
                currentX = 0;
            } else {
                currentX = list.back().m_CachePosX + list.back().m_CacheWidth + 2;
            }

            m_LineInfos[pNode->m_LineNo].currentX = currentX;
        }

        m_GlyphTreeMap.Erase(pNode->GetCode(), pNode->GetFontSize(), pNode->GetFontFace());
        it = next;
    }

    m_GlyphTreeMap.UpdateFlagsForCompleteTextureCache();
}

/**
 * Clears a lock group from every glyph.
 * @param lockGroup lock group index
 */
void TextureCache::ClearLockAllGlyphs(int lockGroup) {
    m_GlyphTreeMap.ClearLockGroup(1 << lockGroup);
}

/**
 * Clears a lock group from the glyphs of a UTF-16 string.
 * @param pCodes string
 * @param codeLength maximum number of characters
 * @param fontSize font size
 * @param fontFace font face
 * @param lockGroup lock group index
 * @param isNoBreakHyphenReplaced whether U+2011 is drawn as a hyphen
 */
void TextureCache::ClearLockGlyphsWithLength(const u16* pCodes, u32 codeLength, u32 fontSize,
                                             u16 fontFace, int lockGroup,
                                             bool isNoBreakHyphenReplaced) {
    for (u32 i = 0; i < codeLength; i++) {
        u32 code = pCodes[i];
        if (isNoBreakHyphenReplaced && code == 0x2011) {
            code = '-';
        }

        if (code == 0) {
            return;
        }

        u32 innerFontFace;
        if (!GetInnerFontFace(&innerFontFace, fontFace, code)) {
            return;
        }

        GlyphNode* pNode = m_GlyphTreeMap.Find(code, fontSize, innerFontFace);
        if (pNode != nullptr) {
            pNode->m_LockGroup &= ~(1 << lockGroup);
        }
    }
}

/**
 * Clears a lock group from the glyphs of a UTF-8 string.
 * @param pCodes string
 * @param codeLength length in bytes
 * @param fontSize font size
 * @param fontFace font face
 * @param lockGroup lock group index
 * @param isNoBreakHyphenReplaced whether U+2011 is drawn as a hyphen
 */
void TextureCache::ClearLockGlyphsWithLengthUtf8(const char* pCodes, u32 codeLength,
                                                 u32 fontSize, u16 fontFace, int lockGroup,
                                                 bool isNoBreakHyphenReplaced) {
    const char* pCurrent = pCodes;
    while (static_cast<u32>(pCurrent - pCodes) != codeLength) {
        char buffer[4] = {};
        nn::util::PickOutCharacterFromUtf8String(buffer, &pCurrent);
        if (buffer[0] == 0) {
            return;
        }

        u32 code = ConvertCharacterUtf8ToUtf32(buffer);
        u32 convertedCode = isNoBreakHyphenReplaced && code == 0x2011 ? '-' : code;
        u32 innerFontFace;
        if (!GetInnerFontFace(&innerFontFace, fontFace, convertedCode)) {
            return;
        }

        GlyphNode* pNode = m_GlyphTreeMap.Find(convertedCode, fontSize, innerFontFace);
        if (pNode != nullptr) {
            pNode->m_LockGroup &= ~(1 << lockGroup);
        }
    }
}

/**
 * Clears the cache texture and removes every glyph.
 */
void TextureCache::ResetTextureCache() {
    u8* pBitMap = static_cast<u8*>(static_cast<MemoryPoolImpl*>(m_pActiveMemoryPool)->Map()) +
                  m_ActiveMemoryPoolOffset;
    std::memset(pBitMap, 0, m_TextureCacheWidth * m_TextureCacheHeight);
    static_cast<MemoryPoolImpl*>(m_pActiveMemoryPool)->Unmap();

    m_NeedPlotGlyphList.clear();
    m_NeedEraseGlyphList.clear();
    m_NotInFontGlyphList.clear();
    for (u32 i = 0; i < m_LineCurrentPos; i++) {
        m_LineInfos[i].list.clear();
    }

    m_LineCurrentPos = 0;
    m_GlyphTreeMap.Reset();
}

/**
 * Changes the search order of the inner fonts of a font face.
 * @param fontFace font face
 * @param pOrders new order as indices relative to the font face
 */
void TextureCache::ChangeFontListOrder(u32 fontFace, u32* pOrders) {
    u8* pInnerFontFaces = m_pInnerFontFaceTables[fontFace];
    for (u32 i = 0; i < m_InnerFontCounts[fontFace]; i++) {
        pInnerFontFaces[i] = (pInnerFontFaces - m_InnerFontFaceTable) + pOrders[i];
    }
}

/**
 * Finds the first font face containing a character.
 * @param code character code
 * @return the font face, or 0xffffffff if none contains it
 */
u32 TextureCache::AcquireFontFaceContainingGlyph(u32 code) {
    for (u32 face = 0; face < m_FontFaceCount; face++) {
        if (IsGlyphExistInFont(code, face)) {
            return face;
        }
    }

    return 0xffffffff;
}

/**
 * Checks whether a font face contains a character.
 * @param code character code
 * @param fontFace font face
 * @return whether the character exists
 */
bool TextureCache::IsGlyphExistInFont(u32 code, u16 fontFace) {
    u32 innerFontFace;
    return GetInnerFontFace(&innerFontFace, fontFace, code);
}

/**
 * Checks whether any inner font of a font face draws borders.
 * @param fontFace font face
 * @return whether borders are drawn
 */
bool TextureCache::IsBorderEffectEnabled(u16 fontFace) const {
    u8 innerFontCount = m_InnerFontCounts[fontFace];
    const u8* pInnerFontFaces = m_pInnerFontFaceTables[fontFace];
    for (u32 i = 0; i < innerFontCount; i++) {
        if (m_pBorderWidths[pInnerFontFaces[i]] != 0) {
            return true;
        }
    }

    return false;
}

/**
 * Counts the glyphs of a string that are not ready yet.
 * @param pCodes UTF-16 string
 * @param codeLength maximum number of characters
 * @param fontSize font size
 * @param fontFace font face
 * @return number of glyphs not ready
 */
u32 TextureCache::CountPlottingGlyph(const u16* pCodes, u32 codeLength, u32 fontSize,
                                     u16 fontFace) {
    u32 count = 0;
    for (u32 i = 0; i < codeLength; i++) {
        u32 code = pCodes[i];
        if (code == 0) {
            break;
        }

        u32 innerFontFace;
        if (!GetInnerFontFace(&innerFontFace, fontFace, code)) {
            continue;
        }

        GlyphNode* pNode = m_GlyphTreeMap.Find(code, fontSize, innerFontFace);
        if (pNode == nullptr ||
            pNode->IsFlagOn(GlyphNode::FlagBit_NotPlotted | GlyphNode::FlagBit_NotInFont) ||
            (!pNode->IsFlagOn(GlyphNode::FlagBit_Requested | GlyphNode::FlagBit_Used |
                              GlyphNode::FlagBit_UsedInLastFrame | GlyphNode::FlagBit_System) &&
             pNode->m_LockGroup == 0)) {
            count++;
        }
    }

    return count;
}

const TextureCache::FontMetrics& TextureCache::GetFontMetrics(u16 fontFace) const {
    const u8* pInnerFontFaces = m_pInnerFontFaceTables[fontFace];
    u32 innerFontCount = m_InnerFontCounts[fontFace];
    const FontMetrics* pFontMetrics = m_pFontMetrics;
    u32 index = pInnerFontFaces[0];
    if (innerFontCount >= 2) {
        float maxHeight = pFontMetrics[index].boundingBoxHeightRatio;
        for (u32 i = 1; i < innerFontCount; i++) {
            if (maxHeight < pFontMetrics[pInnerFontFaces[i]].boundingBoxHeightRatio) {
                maxHeight = pFontMetrics[pInnerFontFaces[i]].boundingBoxHeightRatio;
                index = pInnerFontFaces[i];
            }
        }
    }

    return pFontMetrics[index];
}

/**
 * Calculates the advance width of a character.
 * @param code character code
 * @param fontSize font size
 * @param fontFace font face
 * @return advance width, or 0 on failure
 */
int TextureCache::CalculateCharWidth(u32 code, u32 fontSize, u16 fontFace) {
    u32 innerFontFace;
    if (!GetInnerFontFace(&innerFontFace, fontFace, code)) {
        return 0;
    }

    u32 coreId = GetCoreId();
    SetFontFaceNoPlot(innerFontFace, coreId);
    m_pFontEngineNoPlot[coreId].SetScale(fontSize << 16, 0, 0, fontSize << 16);
    AssertFsErrorNoPlot("SetScale", coreId);
    s16 advanceX;
    s16 advanceY;
    int fixedAdvanceX;
    int fixedAdvanceY;
    int result = m_pFontEngineNoPlot[coreId].GetAdvance(&advanceX, &advanceY, &fixedAdvanceX,
                                                        &fixedAdvanceY, code, 8);
    int width = 0;
    if (result == 0) {
        width = static_cast<int>(m_pFontMetrics[innerFontFace].scaleWidth *
                                 (advanceX + m_pLetterSpacings[innerFontFace]));
    } else if (result == 0xc9) {
        m_IsFsError = true;
    }

    return width;
}

/**
 * Gets the index of the per-core font engine to use.
 * @return core index
 */
u32 TextureCache::GetCoreId() const {
    return m_IsMultiCoreEnabled ? m_pGetCoreIdFunction() : 0;
}

/**
 * Selects an inner font on a per-core font engine.
 * @param fontFace inner font index
 * @param coreId core index
 */
void TextureCache::SetFontFaceNoPlot(u32 fontFace, u32 coreId) {
    if (m_CurrentFontFacesNoPlot[coreId] == fontFace) {
        return;
    }

    fontll::ScalableFontEngine* pEngine = &m_pFontEngineNoPlot[coreId];
    pEngine->SetFont(GetFontName(fontFace));
    AssertFsErrorNoPlot("SetFont", coreId);
    m_pFontEngineNoPlot[coreId].SetBoldWeight(m_pBoldWeights[fontFace]);
    AssertFsErrorNoPlot("SetBoldWeight", coreId);
    if (m_pBorderWidths[fontFace] != 0) {
        m_pFontEngineNoPlot[coreId].SetOutlineWidth(m_pBorderWidths[fontFace]);
        m_pFontEngineNoPlot[coreId].SetFlags(fontll::ScalableFontEngine::Flags_Outlined);
    } else {
        m_pFontEngineNoPlot[coreId].SetFlags(fontll::ScalableFontEngine::Flags_NoEffect);
    }

    AssertFsErrorNoPlot("SetFlags", coreId);
    m_CurrentFontFacesNoPlot[coreId] = fontFace;
}

int TextureCache::CalculateKerning(u32 code0, u32 code1, u32 fontSize, u16 fontFace) {
    u32 innerFontFace;
    if (code0 == 0) {
        if (!GetInnerFontFace(&innerFontFace, fontFace, code1)) {
            return 0;
        }

        if (m_pIsWidthFromBoundingBox[innerFontFace] || m_pIsFixedWidth[innerFontFace]) {
            return 0;
        }

        u32 coreId = GetCoreId();
        SetFontFaceNoPlot(innerFontFace, coreId);
        m_pFontEngineNoPlot[coreId].SetScale(fontSize << 16, 0, 0, fontSize << 16);
        AssertFsErrorNoPlot("SetScale", coreId);
        if (m_pOtfKerningTables[innerFontFace] == nullptr) {
            return 0;
        }

        return static_cast<int>(m_pFontMetrics[innerFontFace].scaleWidth *
                                m_pFontEngineNoPlot[coreId].AcquireOtfKerningFirst(
                                    m_pOtfKerningTables[innerFontFace], code1, fontSize));
    }

    if (code1 == 0) {
        if (!GetInnerFontFace(&innerFontFace, fontFace, code0)) {
            return 0;
        }

        if (m_pIsWidthFromBoundingBox[innerFontFace] || m_pIsFixedWidth[innerFontFace]) {
            return 0;
        }

        u32 coreId = GetCoreId();
        SetFontFaceNoPlot(innerFontFace, coreId);
        m_pFontEngineNoPlot[coreId].SetScale(fontSize << 16, 0, 0, fontSize << 16);
        AssertFsErrorNoPlot("SetScale", coreId);
        if (m_pOtfKerningTables[innerFontFace] == nullptr) {
            return 0;
        }

        return static_cast<int>(m_pFontMetrics[innerFontFace].scaleWidth *
                                m_pFontEngineNoPlot[coreId].AcquireOtfKerningLast(
                                    m_pOtfKerningTables[innerFontFace], code0, fontSize));
    }

    u32 innerFontFace1;
    if (!GetInnerFontFace(&innerFontFace, fontFace, code0)) {
        return 0;
    }

    if (!GetInnerFontFace(&innerFontFace1, fontFace, code1)) {
        return 0;
    }

    if (innerFontFace != innerFontFace1) {
        return 0;
    }

    if (m_pIsWidthFromBoundingBox[innerFontFace] || m_pIsFixedWidth[innerFontFace]) {
        return 0;
    }

    u32 coreId = GetCoreId();
    SetFontFaceNoPlot(innerFontFace, coreId);
    m_pFontEngineNoPlot[coreId].SetScale(fontSize << 16, 0, 0, fontSize << 16);
    AssertFsErrorNoPlot("SetScale", coreId);

    bool isError = false;
    int kerning;
    int otfKerning = 0;
    if (m_pOtfKerningTables[innerFontFace] != nullptr) {
        otfKerning = m_pFontEngineNoPlot[coreId].AcquireOtfKerning(
            m_pOtfKerningTables[innerFontFace], code0, code1, fontSize);
    }

    if (otfKerning != 0) {
        kerning = static_cast<int>(m_pFontMetrics[innerFontFace].scaleWidth * otfKerning);
    } else {
        int kerningX;
        int kerningY;
        if (m_pFontEngineNoPlot[coreId].GetKerning(&kerningX, &kerningY, code0, code1) != 0) {
            isError = true;
        } else {
            kerning = static_cast<int>(m_pFontMetrics[innerFontFace].scaleWidth * kerningX) >> 16;
        }
    }

    if (isError) {
        return 0;
    }

    return kerning;
}

/**
 * Finds the node of a registered glyph.
 * @param code character code
 * @param fontSize font size
 * @param fontFace font face
 * @return the node, or nullptr if not registered
 */
GlyphNode* TextureCache::FindGlyphNode(u32 code, u32 fontSize, u16 fontFace) {
    u32 innerFontFace;
    if (!GetInnerFontFace(&innerFontFace, fontFace, code)) {
        return nullptr;
    }

    return m_GlyphTreeMap.Find(code, fontSize, innerFontFace);
}

/**
 * Counts the glyphs of a string that cannot be drawn yet.
 * @param pCodes UTF-16 string
 * @param codeLength maximum number of characters
 * @param fontSize font size
 * @param fontFace font face
 * @return number of unusable glyphs
 */
u32 TextureCache::CountUnusableGlyph(const u16* pCodes, u32 codeLength, u32 fontSize,
                                     u16 fontFace) {
    u32 count = 0;
    for (u32 i = 0; i < codeLength; i++) {
        u32 code = pCodes[i];
        if (code == 0) {
            break;
        }

        u32 innerFontFace;
        if (!GetInnerFontFace(&innerFontFace, fontFace, code)) {
            continue;
        }

        GlyphNode* pNode = m_GlyphTreeMap.Find(code, fontSize, innerFontFace);
        if (pNode == nullptr ||
            pNode->IsFlagOn(GlyphNode::FlagBit_NotPlotted | GlyphNode::FlagBit_NotInFont) ||
            (!pNode->IsFlagOn(GlyphNode::FlagBit_Requested | GlyphNode::FlagBit_Used |
                              GlyphNode::FlagBit_UsedInLastFrame | GlyphNode::FlagBit_System) &&
             pNode->m_LockGroup == 0)) {
            count++;
        }
    }

    return count;
}

/**
 * Checks whether a character is inside the code ranges of an inner font.
 * @param innerFontFace inner font index
 * @param code character code
 * @return whether the character may be taken from the font
 */
bool TextureCache::CheckCharCodeRange(u32 innerFontFace, u32 code) const {
    int rangeCount = m_pCharCodeRangeCounts[innerFontFace];
    if (rangeCount == 0) {
        return true;
    }

    for (int i = 0; i < rangeCount; i++) {
        u32 first = m_pCharCodeRangeFirsts[innerFontFace][i];
        if ((first & 0x7fffffff) <= code && code <= m_pCharCodeRangeLasts[innerFontFace][i]) {
            return (first & 0x80000000) == 0;
        }
    }

    return false;
}

}  // namespace font
}  // namespace nn
