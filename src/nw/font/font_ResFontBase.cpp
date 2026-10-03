#include <nn/font/font_ResFontBase.h>

#include <nn/util/util_BytePtr.h>

namespace nn {
namespace font {

/**
 * Constructs a resource font without a resource.
 */
ResFontBase::ResFontBase()
    : m_pResource(nullptr),
      m_pResourceBase(nullptr),
      m_pFontInfo(nullptr),
      m_pKerningTable(nullptr),
      m_pMemoryPool(nullptr),
      m_MemoryPoolOffset(0),
      m_MemoryPoolSize(0),
      m_CharCodeRangeCount(0) {
    SetLinearFilterEnabled(true, true);
}

/**
 * Destroys the resource font.
 */
ResFontBase::~ResFontBase() = default;

/**
 * Sets the font resource buffer and its parsed blocks.
 * @param pUserBuffer font resource
 * @param pFontInfo font information block
 * @param pKerningTable kerning block
 * @param pMemoryPool memory pool of the sheet textures
 * @param memoryPoolOffset offset of the resource in the memory pool
 * @param memoryPoolSize size of the resource in the memory pool
 */
void ResFontBase::SetResourceBuffer(void* pUserBuffer, FontInformation* pFontInfo,
                                    FontKerningTable* pKerningTable,
                                    nn::gfx::MemoryPool* pMemoryPool, ptrdiff_t memoryPoolOffset,
                                    size_t memoryPoolSize) {
    m_pResource = pUserBuffer;
    m_pFontInfo = pFontInfo;
    m_pKerningTable = pKerningTable;
    m_pMemoryPool = pMemoryPool;
    m_MemoryPoolOffset = memoryPoolOffset;
    m_MemoryPoolSize = memoryPoolSize;
}

/**
 * Removes the font resource buffer.
 * @return the removed resource
 */
void* ResFontBase::RemoveResourceBuffer() {
    void* pUserData = m_pResource;
    m_pResource = nullptr;
    m_pFontInfo = nullptr;
    m_pMemoryPool = nullptr;
    m_MemoryPoolOffset = 0;
    m_MemoryPoolSize = 0;
    m_pKerningTable = nullptr;
    return pUserData;
}

/**
 * Registers the sheet texture view to a descriptor pool.
 * @param pRegisterFunction registration callback
 * @param pUserData user data passed to the callback
 */
void ResFontBase::RegisterTextureViewToDescriptorPool(RegisterTextureViewSlot pRegisterFunction,
                                                      void* pUserData) {
    pRegisterFunction(&m_TextureObject.GetDescriptorSlot(), *m_TextureObject.GetTextureView(),
                      pUserData);
}

/**
 * Unregisters the sheet texture view from a descriptor pool.
 * @param pUnregisterFunction unregistration callback
 * @param pUserData user data passed to the callback
 */
void ResFontBase::UnregisterTextureViewFromDescriptorPool(
    UnregisterTextureViewSlot pUnregisterFunction, void* pUserData) {
    pUnregisterFunction(&m_TextureObject.GetDescriptorSlot(), *m_TextureObject.GetTextureView(),
                        pUserData);
}

/**
 * Gets the font width.
 * @return the width
 */
int ResFontBase::GetWidth() const {
    return m_pFontInfo->width;
}

/**
 * Gets the font height.
 * @return the height
 */
int ResFontBase::GetHeight() const {
    return m_pFontInfo->height;
}

/**
 * Gets the font ascent.
 * @return the ascent
 */
int ResFontBase::GetAscent() const {
    return m_pFontInfo->ascent;
}

/**
 * Gets the font descent.
 * @return the descent
 */
int ResFontBase::GetDescent() const {
    return m_pFontInfo->height - m_pFontInfo->ascent;
}

/**
 * Gets the baseline position within a cell.
 * @return the baseline position
 */
int ResFontBase::GetBaselinePos() const {
    return GetTextureGlyph()->baselinePos;
}

/**
 * Gets the cell height.
 * @return the cell height
 */
int ResFontBase::GetCellHeight() const {
    return GetTextureGlyph()->cellHeight;
}

/**
 * Gets the cell width.
 * @return the cell width
 */
int ResFontBase::GetCellWidth() const {
    return GetTextureGlyph()->cellWidth;
}

/**
 * Gets the maximum character width.
 * @return the maximum character width
 */
int ResFontBase::GetMaxCharWidth() const {
    return GetTextureGlyph()->maxCharWidth;
}

/**
 * Gets the font type.
 * @return FontType_Texture
 */
FontType ResFontBase::GetType() const {
    return FontType_Texture;
}

/**
 * Gets the sheet texture format.
 * @return the texture format
 */
TexFmt ResFontBase::GetTextureFormat() const {
    return GetTextureGlyph()->sheetFormat & FontSheetFormatMask;
}

/**
 * Gets the line feed height.
 * @return the line feed height
 */
int ResFontBase::GetLineFeed() const {
    return m_pFontInfo->linefeed;
}

/**
 * Gets the default character widths.
 * @return the default character widths
 */
const CharWidths ResFontBase::GetDefaultCharWidths() const {
    return m_pFontInfo->defaultWidth;
}

/**
 * Sets the default character widths.
 * @param rWidths new default character widths
 */
void ResFontBase::SetDefaultCharWidths(const CharWidths& rWidths) {
    m_pFontInfo->defaultWidth = rWidths;
}

/**
 * Sets the character used in place of characters missing from the font.
 * @param c character code
 * @return whether the font has a glyph for the character
 */
bool ResFontBase::SetAlternateChar(uint32_t c) {
    uint16_t index = FindGlyphIndex(c);

    if (index != InvalidGlyphIndex) {
        m_pFontInfo->alterCharIndex = index;
        return true;
    }

    return false;
}

/**
 * Finds the glyph index of a character.
 * @param c character code
 * @return the glyph index, or InvalidGlyphIndex if not found
 */
uint16_t ResFontBase::FindGlyphIndex(uint32_t c) const {
    for (uint32_t offset = m_pFontInfo->pMap; offset != 0;) {
        const FontCodeMap* pMap = GetResourcePtr<FontCodeMap>(offset);

        if (pMap->codeBegin <= c && c <= pMap->codeEnd) {
            return FindGlyphIndex(pMap, c);
        }

        offset = pMap->pNext;
    }

    return InvalidGlyphIndex;
}

/**
 * Sets the line feed height.
 * @param linefeed new line feed height
 */
void ResFontBase::SetLineFeed(int linefeed) {
    m_pFontInfo->linefeed = static_cast<int8_t>(linefeed);
}

/**
 * Gets the advance width of a character.
 * @param c character code
 * @return the advance width
 */
int ResFontBase::GetCharWidth(uint32_t c) const {
    return GetCharWidths(c).charWidth;
}

/**
 * Gets the widths of a character.
 * @param c character code
 * @return the character widths
 */
const CharWidths ResFontBase::GetCharWidths(uint32_t c) const {
    uint16_t index = FindGlyphIndex(c);

    if (index == InvalidGlyphIndex) {
        index = m_pFontInfo->alterCharIndex;
    }

    return *GetCharWidthsFromIndex(index);
}

/**
 * Gets the widths of a glyph.
 * @param index glyph index
 * @return the glyph widths
 */
const CharWidths* ResFontBase::GetCharWidthsFromIndex(uint16_t index) const {
    for (uint32_t offset = m_pFontInfo->pWidth; offset != 0;) {
        const FontWidth* pWidth = GetResourcePtr<FontWidth>(offset);

        if (pWidth->indexBegin <= index && index <= pWidth->indexEnd) {
            return GetCharWidthsFromIndex(pWidth, index);
        }

        offset = pWidth->pNext;
    }

    return &m_pFontInfo->defaultWidth;
}

/**
 * Gets the glyph of a character.
 * @param pGlyph destination glyph
 * @param c character code
 * @return 2 if the alternate character was used, 0 otherwise
 */
int ResFontBase::GetGlyph(Glyph* pGlyph, uint32_t c) const {
    bool isFound;
    uint16_t index = GetGlyphIndex(&isFound, c);
    GetGlyphFromIndex(pGlyph, index);
    return isFound ? 0 : 2;
}

/**
 * Gets the glyph index of a character, falling back to the alternate character.
 * @param pIsFound set to whether the character is in the font
 * @param c character code
 * @return the glyph index
 */
uint16_t ResFontBase::GetGlyphIndex(bool* pIsFound, uint32_t c) const {
    uint16_t index = FindGlyphIndex(c);
    *pIsFound = index != InvalidGlyphIndex;
    if (index == InvalidGlyphIndex) {
        return m_pFontInfo->alterCharIndex;
    }

    return index;
}

/**
 * Gets the glyph at a glyph index.
 * @param pGlyph destination glyph
 * @param index glyph index
 */
void ResFontBase::GetGlyphFromIndex(Glyph* pGlyph, uint16_t index) const {
    const FontTextureGlyph& rTexGlyph = *GetTextureGlyph();
    const uint32_t cellsInASheet = rTexGlyph.sheetRow * rTexGlyph.sheetLine;
    const uint32_t sheetNo = index / cellsInASheet;
    pGlyph->pTexture =
        rTexGlyph.sheetImage != 0 ?
            rTexGlyph.sheetSize * sheetNo + GetResourcePtr<uint8_t>(rTexGlyph.sheetImage) :
            nullptr;

    const CharWidths& rWidths = *GetCharWidthsFromIndex(index);
    pGlyph->widths.left = rWidths.left;
    pGlyph->widths.glyphWidth = rWidths.glyphWidth;
    pGlyph->widths.charWidth = rWidths.charWidth;
    pGlyph->widths.rawWidth = rWidths.glyphWidth;
    pGlyph->pTextureObject = &m_TextureObject;
    pGlyph->sheetIndex = sheetNo;
    pGlyph->isSheetUpdated = false;
    pGlyph->baselineDifference = 0;

    SetGlyphMember(pGlyph, index, rTexGlyph);
}

/**
 * Checks whether the font has a glyph for a character.
 * @param c character code
 * @return whether the glyph exists
 */
bool ResFontBase::HasGlyph(uint32_t c) const {
    return FindGlyphIndex(c) != InvalidGlyphIndex;
}

/**
 * Gets the kerning between two characters.
 * @param c0 first character code
 * @param c1 second character code
 * @return the kerning value
 */
int ResFontBase::GetKerning(uint32_t c0, uint32_t c1) const {
    if (m_pKerningTable == nullptr || !m_IsKerningEnabled) {
        return 0;
    }

    if (!CheckCharCodeRange(c0) || !CheckCharCodeRange(c1)) {
        return 0;
    }

    const FontKerningTable* pTable = m_pKerningTable;
    uint32_t firstLow = 0;
    uint32_t firstHigh = pTable->firstWordCount;
    uint32_t firstMid;

    while (true) {
        firstMid = (firstLow + firstHigh) / 2;
        const uint32_t word = pTable->firstTable[firstMid].firstWord;

        if (word == c0) {
            break;
        }

        if (word < c0) {
            if (firstLow == firstMid) {
                return 0;
            }

            firstLow = firstMid;
        } else {
            if (firstHigh == firstMid) {
                return 0;
            }

            firstHigh = firstMid;
        }
    }

    const KerningSecondTable* pSecond =
        nn::util::ConstBytePtr(pTable, pTable->firstTable[firstMid].offset)
            .Get<KerningSecondTable>();
    uint32_t secondLow = 0;
    uint32_t secondHigh = pSecond->secondWordCount;
    uint32_t secondMid = secondHigh / 2;

    while (pSecond->elems[secondMid].secondWord != c1) {
        if (pSecond->elems[secondMid].secondWord < c1) {
            if (secondLow == secondMid) {
                return 0;
            }

            secondLow = secondMid;
        } else {
            if (secondHigh == secondMid) {
                return 0;
            }

            secondHigh = secondMid;
        }

        secondMid = (secondLow + secondHigh) / 2;
    }

    return pSecond->elems[secondMid].kerningValue;
}

/**
 * Checks whether a character is inside the enabled character code ranges.
 * @param c character code
 * @return whether the character is enabled
 */
bool ResFontBase::CheckCharCodeRange(uint32_t c) const {
    if (m_CharCodeRangeCount == 0) {
        return true;
    }

    for (int i = 0; i < m_CharCodeRangeCount; i++) {
        if (m_CharCodeRangeFirst[i] <= c && c <= m_CharCodeRangeLast[i]) {
            return true;
        }
    }

    return false;
}

/**
 * Gets the character code of the font.
 * @return the character code
 */
CharacterCode ResFontBase::GetCharacterCode() const {
    return static_cast<CharacterCode>(m_pFontInfo->characterCode);
}

/**
 * Finds the glyph index of a character in a code map.
 * @param pMap code map containing the character
 * @param c character code
 * @return the glyph index, or InvalidGlyphIndex if not found
 */
uint16_t ResFontBase::FindGlyphIndex(const FontCodeMap* pMap, uint32_t c) const {
    if (!CheckCharCodeRange(c)) {
        return InvalidGlyphIndex;
    }

    switch (pMap->mappingMethod) {
    case FontMapMethod_Direct:
        return pMap->mapInfo[0] + (c - pMap->codeBegin);
    case FontMapMethod_Table: {
        const int offset = c - pMap->codeBegin;
        return pMap->mapInfo[offset];
    }
    case FontMapMethod_Scan: {
        const CMapInfoScan* pScan = reinterpret_cast<const CMapInfoScan*>(pMap->mapInfo);
        const CMapScanEntry* pFirst = &pScan->entries[0];
        const CMapScanEntry* pLast = &pScan->entries[pScan->count - 1];

        while (pFirst <= pLast) {
            const CMapScanEntry* pMid = pFirst + (pLast - pFirst) / 2;

            if (pMid->code < c) {
                pFirst = pMid + 1;
            } else if (c < pMid->code) {
                pLast = pMid - 1;
            } else {
                return pMid->index;
            }
        }

        return InvalidGlyphIndex;
    }
    default:
        return InvalidGlyphIndex;
    }
}

/**
 * Gets the widths of a glyph in a width block.
 * @param pWidth width block containing the glyph
 * @param index glyph index
 * @return the glyph widths
 */
const CharWidths* ResFontBase::GetCharWidthsFromIndex(const FontWidth* pWidth,
                                                      uint16_t index) const {
    return &pWidth->widthTable[index - pWidth->indexBegin];
}

/**
 * Sets the sheet related members of a glyph.
 * @param pGlyph destination glyph
 * @param index glyph index
 * @param rTexGlyph texture glyph block
 */
void ResFontBase::SetGlyphMember(Glyph* pGlyph, uint16_t index, const FontTextureGlyph& rTexGlyph) {
    const uint32_t cellNo = index % (rTexGlyph.sheetRow * rTexGlyph.sheetLine);
    const uint32_t cellUnitY = cellNo / rTexGlyph.sheetRow;
    const uint32_t cellUnitX = cellNo % rTexGlyph.sheetRow;
    const uint32_t cellPixelX = cellUnitX * (rTexGlyph.cellWidth + 1);
    const uint32_t cellPixelY = cellUnitY * (rTexGlyph.cellHeight + 1);

    pGlyph->height = rTexGlyph.cellHeight;
    pGlyph->rawHeight = rTexGlyph.cellHeight;
    pGlyph->texFormat = rTexGlyph.sheetFormat & FontSheetFormatMask;
    pGlyph->texWidth = rTexGlyph.sheetWidth;
    pGlyph->texHeight = rTexGlyph.sheetHeight;
    pGlyph->cellX = cellPixelX + 1;
    pGlyph->cellY = cellPixelY + 1;
}

/**
 * Finds a block in a font binary.
 * @param pHeader font binary
 * @param signature block signature
 * @return the block contents, or nullptr if not found
 */
void* ResFontBase::FindBlock(detail::BinaryFileHeader* pHeader, uint32_t signature) {
    uint8_t* pPtr = reinterpret_cast<uint8_t*>(pHeader) + pHeader->headerSize;
    void* pResult = nullptr;

    for (int i = 0; i < pHeader->dataBlocks; i++) {
        detail::BinaryBlockHeader* pBlock = reinterpret_cast<detail::BinaryBlockHeader*>(pPtr);

        if (pBlock->kind == signature) {
            pResult = pBlock + 1;
            break;
        }

        pPtr += pBlock->size;
    }

    return pResult;
}

/**
 * Sets whether linear filtering is used when drawing small and large.
 * @param atSmall whether linear filtering is used when minifying
 * @param atLarge whether linear filtering is used when magnifying
 */
void ResFontBase::SetLinearFilterEnabled(bool atSmall, bool atLarge) {
    m_TextureWrapFilter =
        (static_cast<uint32_t>(atLarge) << 1) | (static_cast<uint32_t>(atSmall) << 2);
}

/**
 * Checks whether linear filtering is used when minifying.
 * @return whether linear filtering is used
 */
bool ResFontBase::IsLinearFilterEnabledAtSmall() const {
    return (m_TextureWrapFilter >> 2) & 1;
}

/**
 * Checks whether linear filtering is used when magnifying.
 * @return whether linear filtering is used
 */
bool ResFontBase::IsLinearFilterEnabledAtLarge() const {
    return (m_TextureWrapFilter >> 1) & 1;
}

/**
 * Gets the texture wrap and filter value.
 * @return the texture wrap and filter value
 */
uint32_t ResFontBase::GetTextureWrapFilterValue() const {
    return m_TextureWrapFilter;
}

/**
 * Checks whether black/white color interpolation is enabled.
 * @return whether the interpolation is enabled
 */
bool ResFontBase::IsColorBlackWhiteInterpolationEnabled() const {
    return m_TextureObject.IsColorBlackWhiteInterpolationEnabled();
}

/**
 * Sets whether black/white color interpolation is enabled.
 * @param isEnabled whether the interpolation is enabled
 */
void ResFontBase::SetColorBlackWhiteInterpolationEnabled(bool isEnabled) {
    m_TextureObject.SetColorBlackWhiteInterpolationEnabled(isEnabled);
}

/**
 * Checks whether the border effect is enabled.
 * @return whether the font type is the border type
 */
bool ResFontBase::IsBorderEffectEnabled() const {
    return m_pFontInfo->fontType == 2;
}

/**
 * Sets the enabled character code ranges.
 * @param count number of ranges
 * @param pFirst first character code of each range
 * @param pLast last character code of each range
 */
void ResFontBase::SetCharCodeRange(int count, uint32_t* pFirst, uint32_t* pLast) {
    m_CharCodeRangeCount = count;

    for (int i = 0; i < count; i++) {
        m_CharCodeRangeFirst[i] = pFirst[i];
        m_CharCodeRangeLast[i] = pLast[i];
    }
}

/**
 * Gets the number of sheets in use.
 * @return the number of sheets
 */
int ResFontBase::GetActiveSheetCount() const {
    return GetTextureGlyph()->sheetCount;
}

/**
 * Sets up the sheet texture object and loads the sheet texture.
 * @param pDevice gfx device
 */
void ResFontBase::GenTextureNames(nn::gfx::Device* pDevice) {
    if (m_TextureObject.IsInitialized()) {
        return;
    }

    const FontTextureGlyph& rTexGlyph = *GetTextureGlyph();
    const int sheetCount = GetActiveSheetCount();
    m_TextureObject.Set(GetResourcePtrOrNull<void>(rTexGlyph.sheetImage), rTexGlyph.sheetFormat,
                        rTexGlyph.sheetWidth, rTexGlyph.sheetHeight, sheetCount, true);
    LoadTexture(pDevice, &m_TextureObject);
}

/**
 * Loads the texture of a texture object.
 * @param pDevice gfx device
 * @param pTexObj texture object
 * @return whether the texture format is supported
 */
bool ResFontBase::LoadTexture(nn::gfx::Device* pDevice, ResourceTextureObject* pTexObj) const {
    if ((pTexObj->GetFormat() & FontSheetFormatMask) >= 20) {
        return false;
    }

    switch (pTexObj->GetFormat() & FontSheetFormatMask) {
    case 1:
    case 6:
    case 7:
        return false;
    default:
        break;
    }

    pTexObj->Initialize(
        pDevice, m_pMemoryPool,
        nn::util::BytePtr(m_pResource).Distance(pTexObj->GetImage()) + m_MemoryPoolOffset,
        m_MemoryPoolSize);
    return true;
}

/**
 * Unloads the sheet texture.
 * @param pDevice gfx device
 */
void ResFontBase::DeleteTextureNames(nn::gfx::Device* pDevice) {
    UnloadTexture(pDevice, &m_TextureObject);
}

/**
 * Unloads the texture of a texture object.
 * @param pDevice gfx device
 * @param pTexObj texture object
 */
void ResFontBase::UnloadTexture(nn::gfx::Device* pDevice, ResourceTextureObject* pTexObj) const {
    if (!pTexObj->IsInitialized()) {
        return;
    }

    nn::gfx::ResTextureFile* pFile =
        nn::gfx::ResTextureFile::ResCast(const_cast<void*>(pTexObj->GetImage()));
    nn::gfx::ResTextureContainerData& rContainer = pFile->ToData().textureContainerData;

    if (rContainer.pCurrentMemoryPool.Get() != nullptr) {
        auto* pDeviceImpl = reinterpret_cast<detail::GfxDeviceImpl*>(pDevice);
        nn::gfx::ResTextureData& rData = pTexObj->m_pResTexture->ToData();
        static_cast<detail::GfxTextureImpl*>(rData.pTexture.Get())->Finalize(pDeviceImpl);
        static_cast<detail::GfxTextureViewImpl*>(rData.pTextureView.Get())->Finalize(pDeviceImpl);

        if (rContainer.pCurrentMemoryPool.Get() == rContainer.pTextureMemoryPool.Get()) {
            static_cast<detail::GfxMemoryPoolImpl*>(rContainer.pTextureMemoryPool.Get())
                ->Finalize(pDeviceImpl);
        }

        rContainer.pCurrentMemoryPool.Set(nullptr);
    }

    pTexObj->Reset();
}

/**
 * Gets the glyph of the alternate character.
 * @param pGlyph destination glyph
 * @param c character code
 */
void ResFontBase::GetAlternateCharGlyph(Glyph* pGlyph, uint32_t c) const {
    GetGlyphFromIndex(pGlyph, m_pFontInfo->alterCharIndex);
}

}  // namespace font
}  // namespace nn
