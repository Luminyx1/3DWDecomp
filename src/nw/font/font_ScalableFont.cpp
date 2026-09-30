#include <nn/font/font_ScalableFont.h>

namespace nn {
namespace font {

/**
 * Sets the default initialization parameters.
 */
void ScalableFont::InitializeArg::SetDefault() {
    pTextureCache = nullptr;
    fontSize = 40;
    fontFace = 0;
    isAlternateCharSpaceWithoutOtherGlyph = false;
    isDrawWhiteSpaceWhenGlyphNotReady = false;
    alternateChar = '?';
    lineFeedOffset = 0;
    isNoBreakHyphenReplaced = false;
}

/**
 * Constructs a scalable font with default metrics.
 */
ScalableFont::ScalableFont()
    : m_pTextureCache(nullptr), m_FontSize(40), m_Height(50), m_Ascent(40), m_BaselinePos(40),
      m_LineFeed(50), m_AlternateCharMode(AlternateCharMode_UseAlternateChar),
      m_IsNoBreakHyphenReplaced(false), m_FontFace(0), m_AlternateChar('?') {
    m_DefaultCharWidths.left = 0;
    m_DefaultCharWidths.glyphWidth = 40;
    m_DefaultCharWidths.charWidth = 40;
}

/**
 * Destroys the font.
 */
ScalableFont::~ScalableFont() = default;

/**
 * Binds the font to a texture cache font face and computes its metrics.
 * @param rArg initialization parameters
 */
void ScalableFont::Initialize(const InitializeArg& rArg) {
    if (rArg.pTextureCache == nullptr) {
        return;
    }

    m_pTextureCache = rArg.pTextureCache;
    m_FontSize = rArg.fontSize;
    m_FontFace = rArg.fontFace;
    const TextureCache::FontMetrics& metrics = m_pTextureCache->GetFontMetrics(m_FontFace);
    m_Ascent = static_cast<int>(metrics.ascentRatio * m_FontSize + 0.5f);
    m_Height = static_cast<int>(metrics.heightRatio * m_FontSize + 0.5f);
    m_BaselinePos = static_cast<int>(metrics.boundingBoxAscentRatio * m_FontSize + 0.5f);
    m_LineFeed = m_Height + rArg.lineFeedOffset;
    m_DefaultCharWidths.left = 0;
    m_DefaultCharWidths.glyphWidth = m_FontSize;
    m_DefaultCharWidths.charWidth = m_FontSize;

    if (rArg.isAlternateCharSpaceWithoutOtherGlyph) {
        m_AlternateCharMode = AlternateCharMode_UseWhiteSpace;
        m_AlternateChar = ' ';
    } else if (rArg.isDrawWhiteSpaceWhenGlyphNotReady) {
        m_AlternateCharMode = AlternateCharMode_UseWhiteSpaceWhenNotReady;
        m_AlternateChar = rArg.alternateChar;
    } else {
        m_AlternateCharMode = AlternateCharMode_UseAlternateChar;
        m_AlternateChar = rArg.alternateChar;
    }

    m_IsNoBreakHyphenReplaced = rArg.isNoBreakHyphenReplaced;

    RegisterAlternateCharGlyph();
}

/**
 * Registers the alternate character glyphs as system glyphs.
 */
void ScalableFont::RegisterAlternateCharGlyph() const {
    u32 alternateChar = m_AlternateChar;
    m_pTextureCache->RegisterGlyph(alternateChar, m_FontSize, m_FontFace, -1,
                                   m_IsNoBreakHyphenReplaced);
    m_pTextureCache->FindGlyphNode(m_AlternateChar, m_FontSize, m_FontFace)
        ->SetFlag(GlyphNode::FlagBit_System);

    if (m_AlternateCharMode == AlternateCharMode_UseWhiteSpaceWhenNotReady) {
        m_pTextureCache->RegisterGlyph(' ', m_FontSize, m_FontFace, -1,
                                       m_IsNoBreakHyphenReplaced);
        if (m_pTextureCache->FindGlyphNode(' ', m_FontSize, m_FontFace) != nullptr) {
            m_pTextureCache->FindGlyphNode(' ', m_FontSize, m_FontFace)
                ->SetFlag(GlyphNode::FlagBit_System);
        }
    }
}

/**
 * Gets the font width.
 * @return font size
 */
int ScalableFont::GetWidth() const {
    return m_FontSize;
}

/**
 * Gets the font height.
 * @return font height
 */
int ScalableFont::GetHeight() const {
    return m_Height;
}

/**
 * Gets the ascent.
 * @return ascent
 */
int ScalableFont::GetAscent() const {
    return m_Ascent;
}

/**
 * Gets the descent.
 * @return descent
 */
int ScalableFont::GetDescent() const {
    return m_Height - m_Ascent;
}

/**
 * Gets the maximum character width.
 * @return font size
 */
int ScalableFont::GetMaxCharWidth() const {
    return m_FontSize;
}

/**
 * Gets the font type.
 * @return font type
 */
FontType ScalableFont::GetType() const {
    return FontType_PackedTexture;
}

/**
 * Gets the glyph texture format.
 * @return texture format
 */
TexFmt ScalableFont::GetTextureFormat() const {
    return 8;
}

/**
 * Gets the line feed.
 * @return line feed
 */
int ScalableFont::GetLineFeed() const {
    return m_LineFeed;
}

/**
 * Gets the default character widths.
 * @return default character widths
 */
const CharWidths ScalableFont::GetDefaultCharWidths() const {
    return m_DefaultCharWidths;
}

/**
 * Sets the line feed.
 * @param linefeed line feed
 */
void ScalableFont::SetLineFeed(int linefeed) {
    m_LineFeed = linefeed;
}

/**
 * Sets the default character widths.
 * @param rWidths character widths
 */
void ScalableFont::SetDefaultCharWidths(const CharWidths& rWidths) {
    m_DefaultCharWidths = rWidths;
}

/**
 * Sets the alternate character if the font contains it.
 * @param c character code
 * @return whether the alternate character was set
 */
bool ScalableFont::SetAlternateChar(uint32_t c) {
    uint32_t code = ConvertCode(c);
    if (m_AlternateCharMode == AlternateCharMode_UseWhiteSpace) {
        return false;
    }

    if (!m_pTextureCache->IsGlyphExistInFont(code, m_FontFace)) {
        return false;
    }

    m_AlternateChar = code;
    return true;
}

/**
 * Gets the advance width of a character.
 * @param c character code
 * @return advance width
 */
int ScalableFont::GetCharWidth(uint32_t c) const {
    uint32_t code = ConvertCode(c);
    GlyphNode* pNode = m_pTextureCache->FindGlyphNode(code, m_FontSize, m_FontFace);
    if (pNode == nullptr ||
        pNode->IsFlagOn(GlyphNode::FlagBit_NotPlotted | GlyphNode::FlagBit_NotInFont)) {
        if (m_AlternateCharMode != AlternateCharMode_UseAlternateChar &&
            m_pTextureCache->IsGlyphExistInFont(code, m_FontFace)) {
            return m_pTextureCache->CalculateCharWidth(code, m_FontSize, m_FontFace);
        }

        pNode = m_pTextureCache->FindGlyphNode(m_AlternateChar, m_FontSize, m_FontFace);
    }

    return pNode->m_AdvanceX;
}

/**
 * Gets the widths of a character.
 * @param c character code
 * @return character widths
 */
const CharWidths ScalableFont::GetCharWidths(uint32_t c) const {
    uint32_t code = ConvertCode(c);
    GlyphNode* pNode = m_pTextureCache->FindGlyphNode(code, m_FontSize, m_FontFace);
    CharWidths widths;
    if (pNode == nullptr ||
        pNode->IsFlagOn(GlyphNode::FlagBit_NotPlotted | GlyphNode::FlagBit_NotInFont)) {
        if (m_AlternateCharMode != AlternateCharMode_UseAlternateChar &&
            m_pTextureCache->IsGlyphExistInFont(code, m_FontFace)) {
            int width = m_pTextureCache->CalculateCharWidth(code, m_FontSize, m_FontFace);
            widths.left = 0;
            widths.glyphWidth = width;
            widths.charWidth = width;
            return widths;
        }

        pNode = m_pTextureCache->FindGlyphNode(m_AlternateChar, m_FontSize, m_FontFace);
    }

    widths.left = pNode->m_LeftOffset;
    widths.glyphWidth = pNode->m_GlyphWidth;
    widths.charWidth = pNode->m_AdvanceX;
    return widths;
}

/**
 * Gets the glyph of a character, falling back to the alternate character.
 * @param pGlyph destination glyph
 * @param c character code
 * @return 0 if the glyph is cached, 1 if it exists but is not plotted yet, 2 if it does not exist
 */
int ScalableFont::GetGlyph(Glyph* pGlyph, uint32_t c) const {
    uint32_t code = ConvertCode(c);
    GlyphNode* pNode = m_pTextureCache->FindGlyphNode(code, m_FontSize, m_FontFace);
    int result;
    int width;
    if (pNode != nullptr &&
        !pNode->IsFlagOn(GlyphNode::FlagBit_NotPlotted | GlyphNode::FlagBit_NotInFont) &&
        (pNode->IsFlagOn(GlyphNode::FlagBit_Requested | GlyphNode::FlagBit_Used |
                         GlyphNode::FlagBit_UsedInLastFrame | GlyphNode::FlagBit_System) ||
         pNode->m_LockGroup != 0)) {
        result = 0;
        width = pNode->m_AdvanceX;
    } else {
        bool isExist = m_pTextureCache->IsGlyphExistInFont(code, m_FontFace);
        result = isExist ? 1 : 2;
        if (m_AlternateCharMode != AlternateCharMode_UseAlternateChar && isExist) {
            width = m_pTextureCache->CalculateCharWidth(code, m_FontSize, m_FontFace);
            pNode = FindAlternateCharGlyphNode(' ');
        } else {
            pNode = FindAlternateCharGlyphNode(m_AlternateChar);
            width = pNode->m_AdvanceX;
        }
    }

    SetGlyphNodeToGlyph(pGlyph, pNode, width);
    return result;
}

/**
 * Finds the node of a glyph in the font face or in any font face containing it.
 * @param c character code
 * @return the node, or nullptr if not registered
 */
GlyphNode* ScalableFont::FindAlternateCharGlyphNode(uint32_t c) const {
    GlyphNode* pNode = m_pTextureCache->FindGlyphNode(c, m_FontSize, m_FontFace);
    if (pNode != nullptr) {
        return pNode;
    }

    u32 fontFace = m_pTextureCache->AcquireFontFaceContainingGlyph(c);
    if (fontFace != 0xffffffff) {
        pNode = m_pTextureCache->FindGlyphNode(c, m_FontSize, fontFace);
        if (pNode != nullptr) {
            return pNode;
        }
    }

    return nullptr;
}

/**
 * Fills a glyph from a texture cache node.
 * @param pGlyph destination glyph
 * @param pNode texture cache node
 * @param width advance width
 */
void ScalableFont::SetGlyphNodeToGlyph(Glyph* pGlyph, GlyphNode* pNode, int width) const {
    pGlyph->widths.left = pNode->m_LeftOffset;
    pGlyph->widths.glyphWidth = pNode->m_GlyphWidth;
    pGlyph->widths.charWidth = width;
    pGlyph->widths.rawWidth = pNode->m_CacheWidth;
    pGlyph->height = pNode->m_GlyphHeight + 1;
    pGlyph->rawHeight = pNode->m_CacheHeight + 1;
    pGlyph->cellX = pNode->m_CachePosX + 1;
    pGlyph->cellY = pNode->m_CachePosY;
    pGlyph->pTexture = m_pTextureCache->GetTextureBitMap();
    pGlyph->texFormat = 8;
    pGlyph->isSheetUpdated = 0;
    pGlyph->sheetIndex = 0;
    pGlyph->texWidth = m_pTextureCache->GetTextureCacheWidth();
    pGlyph->texHeight = m_pTextureCache->GetTextureCacheHeight();
    pGlyph->pTextureObject = m_pTextureCache->GetTextureObject();
    pGlyph->baselineDifference = pNode->m_BaselineOffset - m_BaselinePos;
    pNode->SetFlag(GlyphNode::FlagBit_Used);
}

/**
 * Checks whether a glyph is ready in the texture cache.
 * @param c character code
 * @return whether the glyph can be drawn
 */
bool ScalableFont::HasGlyph(uint32_t c) const {
    uint32_t code = ConvertCode(c);
    GlyphNode* pNode = m_pTextureCache->FindGlyphNode(code, m_FontSize, m_FontFace);
    if (pNode == nullptr) {
        return false;
    }

    if (pNode->IsFlagOn(GlyphNode::FlagBit_NotPlotted | GlyphNode::FlagBit_NotInFont)) {
        return false;
    }

    if (pNode->IsFlagOn(GlyphNode::FlagBit_Requested | GlyphNode::FlagBit_Used |
                        GlyphNode::FlagBit_UsedInLastFrame | GlyphNode::FlagBit_System)) {
        return true;
    }

    return pNode->m_LockGroup != 0;
}

/**
 * Gets the kerning between two characters.
 * @param c0 first character code
 * @param c1 second character code
 * @return kerning
 */
int ScalableFont::GetKerning(uint32_t c0, uint32_t c1) const {
    if (!IsKerningEnabled()) {
        return 0;
    }

    uint32_t code1 = ConvertCode(c1);
    uint32_t code0 = ConvertCode(c0);
    return m_pTextureCache->CalculateKerning(code0, code1, m_FontSize, m_FontFace);
}

/**
 * Gets the character encoding.
 * @return character encoding
 */
CharacterCode ScalableFont::GetCharacterCode() const {
    return CharacterCode_Unicode;
}

/**
 * Gets the baseline position.
 * @return baseline position
 */
int ScalableFont::GetBaselinePos() const {
    return m_BaselinePos;
}

/**
 * Gets the cell height.
 * @return font height
 */
int ScalableFont::GetCellHeight() const {
    return m_Height;
}

/**
 * Gets the cell width.
 * @return font size
 */
int ScalableFont::GetCellWidth() const {
    return m_FontSize;
}

/**
 * Does nothing; scalable fonts always use linear filtering.
 * @param atSmall ignored
 * @param atLarge ignored
 */
void ScalableFont::SetLinearFilterEnabled(bool atSmall, bool atLarge) {}

/**
 * Checks whether linear filtering is used for minification.
 * @return true
 */
bool ScalableFont::IsLinearFilterEnabledAtSmall() const {
    return true;
}

/**
 * Checks whether linear filtering is used for magnification.
 * @return true
 */
bool ScalableFont::IsLinearFilterEnabledAtLarge() const {
    return true;
}

/**
 * Gets the texture wrap and filter value.
 * @return 0
 */
uint32_t ScalableFont::GetTextureWrapFilterValue() const {
    return 0;
}

/**
 * Checks whether black/white color interpolation is enabled.
 * @return true
 */
bool ScalableFont::IsColorBlackWhiteInterpolationEnabled() const {
    return true;
}

/**
 * Checks whether any font of the font face draws borders.
 * @return whether borders are drawn
 */
bool ScalableFont::IsBorderEffectEnabled() const {
    if (m_pTextureCache == nullptr) {
        return false;
    }

    return m_pTextureCache->IsBorderEffectEnabled(m_FontFace);
}

/**
 * Does nothing; interpolation is always enabled.
 * @param isEnabled ignored
 */
void ScalableFont::SetColorBlackWhiteInterpolationEnabled(bool isEnabled) {}

/**
 * Gets the glyph drawn in place of a character that is not ready.
 * @param pGlyph destination glyph
 * @param c character code
 */
void ScalableFont::GetAlternateCharGlyph(Glyph* pGlyph, uint32_t c) const {
    uint32_t code = ConvertCode(c);
    GlyphNode* pNode;
    int width;
    if (m_AlternateCharMode != AlternateCharMode_UseAlternateChar &&
        m_pTextureCache->IsGlyphExistInFont(code, m_FontFace)) {
        width = m_pTextureCache->CalculateCharWidth(code, m_FontSize, m_FontFace);
        pNode = FindAlternateCharGlyphNode(' ');
    } else {
        pNode = FindAlternateCharGlyphNode(m_AlternateChar);
        width = pNode->m_AdvanceX;
    }

    SetGlyphNodeToGlyph(pGlyph, pNode, width);
}

}  // namespace font
}  // namespace nn
