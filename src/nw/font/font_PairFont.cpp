#include <nn/font/font_PairFont.h>

namespace nn {
namespace font {

/**
 * Constructs an empty pair font.
 */
PairFont::PairFont()
    : m_pFirstFont(nullptr), m_pSecondFont(nullptr), m_IsAlternateCharInFirstFont(true) {}

/**
 * Constructs a pair font from two fonts.
 * @param pFirst primary font
 * @param pSecond secondary font
 */
PairFont::PairFont(Font* pFirst, Font* pSecond)
    : m_pFirstFont(pFirst), m_pSecondFont(pSecond), m_IsAlternateCharInFirstFont(true) {}

/**
 * Sets the two fonts.
 * @param pFirst primary font
 * @param pSecond secondary font
 */
void PairFont::SetFont(Font* pFirst, Font* pSecond) {
    m_pFirstFont = pFirst;
    m_pSecondFont = pSecond;
}

/**
 * Destroys the pair font.
 */
PairFont::~PairFont() = default;

/**
 * Gets the larger width of the two fonts.
 * @return width
 */
int PairFont::GetWidth() const {
    int firstWidth = m_pFirstFont->GetWidth();
    int secondWidth = m_pSecondFont->GetWidth();
    return firstWidth < secondWidth ? secondWidth : firstWidth;
}

/**
 * Gets the larger height of the two fonts.
 * @return height
 */
int PairFont::GetHeight() const {
    int firstHeight = m_pFirstFont->GetHeight();
    int secondHeight = m_pSecondFont->GetHeight();
    return firstHeight < secondHeight ? secondHeight : firstHeight;
}

/**
 * Gets the ascent of the taller font.
 * @return ascent
 */
int PairFont::GetAscent() const {
    if (m_pFirstFont->GetHeight() < m_pSecondFont->GetHeight()) {
        return m_pSecondFont->GetAscent();
    }

    return m_pFirstFont->GetAscent();
}

/**
 * Gets the descent of the taller font.
 * @return descent
 */
int PairFont::GetDescent() const {
    if (m_pFirstFont->GetHeight() < m_pSecondFont->GetHeight()) {
        return m_pSecondFont->GetDescent();
    }

    return m_pFirstFont->GetDescent();
}

/**
 * Gets the baseline position of the taller font.
 * @return baseline position
 */
int PairFont::GetBaselinePos() const {
    if (m_pFirstFont->GetHeight() < m_pSecondFont->GetHeight()) {
        return m_pSecondFont->GetBaselinePos();
    }

    return m_pFirstFont->GetBaselinePos();
}

/**
 * Gets the cell height of the taller font.
 * @return cell height
 */
int PairFont::GetCellHeight() const {
    if (m_pFirstFont->GetHeight() < m_pSecondFont->GetHeight()) {
        return m_pSecondFont->GetCellHeight();
    }

    return m_pFirstFont->GetCellHeight();
}

/**
 * Gets the cell width of the wider font.
 * @return cell width
 */
int PairFont::GetCellWidth() const {
    if (m_pFirstFont->GetWidth() < m_pSecondFont->GetWidth()) {
        return m_pSecondFont->GetCellWidth();
    }

    return m_pFirstFont->GetCellWidth();
}

/**
 * Gets the larger maximum character width of the two fonts.
 * @return maximum character width
 */
int PairFont::GetMaxCharWidth() const {
    int firstWidth = m_pFirstFont->GetMaxCharWidth();
    int secondWidth = m_pSecondFont->GetMaxCharWidth();
    return firstWidth < secondWidth ? secondWidth : firstWidth;
}

/**
 * Gets the font type.
 * @return font type
 */
FontType PairFont::GetType() const {
    return static_cast<FontType>(3);
}

/**
 * Gets the texture format of the primary font.
 * @return texture format
 */
TexFmt PairFont::GetTextureFormat() const {
    return m_pFirstFont->GetTextureFormat();
}

/**
 * Gets the larger line feed of the two fonts.
 * @return line feed
 */
int PairFont::GetLineFeed() const {
    int firstLineFeed = m_pFirstFont->GetLineFeed();
    int secondLineFeed = m_pSecondFont->GetLineFeed();
    return firstLineFeed < secondLineFeed ? secondLineFeed : firstLineFeed;
}

/**
 * Gets the default character widths of the wider font.
 * @return default character widths
 */
const CharWidths PairFont::GetDefaultCharWidths() const {
    if (m_pFirstFont->GetWidth() < m_pSecondFont->GetWidth()) {
        return m_pSecondFont->GetDefaultCharWidths();
    }

    return m_pFirstFont->GetDefaultCharWidths();
}

/**
 * Sets the default character widths of both fonts.
 * @param rWidths character widths
 */
void PairFont::SetDefaultCharWidths(const CharWidths& rWidths) {
    m_pFirstFont->SetDefaultCharWidths(rWidths);
    m_pSecondFont->SetDefaultCharWidths(rWidths);
}

/**
 * Sets the alternate character of both fonts.
 * @param c character code
 * @return whether either font accepted the character
 */
bool PairFont::SetAlternateChar(uint32_t c) {
    bool isFirstSet = m_pFirstFont->SetAlternateChar(c);
    bool isSecondSet = m_pSecondFont->SetAlternateChar(c);

    if (isFirstSet || isSecondSet) {
        m_IsAlternateCharInFirstFont = isFirstSet;
        return true;
    }

    return false;
}

/**
 * Sets the line feed of the taller font.
 * @param linefeed line feed
 */
void PairFont::SetLineFeed(int linefeed) {
    if (m_pFirstFont->GetHeight() < m_pSecondFont->GetHeight()) {
        m_pSecondFont->SetLineFeed(linefeed);
    } else {
        m_pFirstFont->SetLineFeed(linefeed);
    }
}

/**
 * Gets the advance width of a character from the font containing it.
 * @param c character code
 * @return advance width
 */
int PairFont::GetCharWidth(uint32_t c) const {
    if (m_pFirstFont->IsGlyphExistInFont(c) ||
        (!m_pSecondFont->IsGlyphExistInFont(c) && m_IsAlternateCharInFirstFont)) {
        return m_pFirstFont->GetCharWidth(c);
    }

    return m_pSecondFont->GetCharWidth(c);
}

/**
 * Gets the widths of a character from the font containing it.
 * @param c character code
 * @return character widths
 */
const CharWidths PairFont::GetCharWidths(uint32_t c) const {
    if (m_pFirstFont->IsGlyphExistInFont(c) ||
        (!m_pSecondFont->IsGlyphExistInFont(c) && m_IsAlternateCharInFirstFont)) {
        return m_pFirstFont->GetCharWidths(c);
    }

    return m_pSecondFont->GetCharWidths(c);
}

/**
 * Gets the glyph of a character from the font containing it.
 * @param pGlyph destination glyph
 * @param c character code
 * @return result of the underlying font
 */
int PairFont::GetGlyph(Glyph* pGlyph, uint32_t c) const {
    int result;

    if (m_pFirstFont->IsGlyphExistInFont(c) ||
        (!m_pSecondFont->IsGlyphExistInFont(c) && m_IsAlternateCharInFirstFont)) {
        result = m_pFirstFont->GetGlyph(pGlyph, c);
        pGlyph->baselineDifference += m_pFirstFont->GetBaselinePos() - GetBaselinePos();
    } else {
        result = m_pSecondFont->GetGlyph(pGlyph, c);
        pGlyph->baselineDifference += m_pSecondFont->GetBaselinePos() - GetBaselinePos();
    }

    return result;
}

/**
 * Checks whether either font has a glyph.
 * @param c character code
 * @return whether a glyph is available
 */
bool PairFont::HasGlyph(uint32_t c) const {
    if (m_pFirstFont->HasGlyph(c)) {
        return true;
    }

    return m_pSecondFont->HasGlyph(c);
}

/**
 * Checks whether either font contains a glyph.
 * @param c character code
 * @return whether the glyph exists
 */
bool PairFont::IsGlyphExistInFont(uint32_t c) const {
    if (m_pFirstFont->IsGlyphExistInFont(c)) {
        return true;
    }

    return m_pSecondFont->IsGlyphExistInFont(c);
}

/**
 * Gets the kerning from the font containing both characters.
 * @param c0 first character code
 * @param c1 second character code
 * @return kerning
 */
int PairFont::GetKerning(uint32_t c0, uint32_t c1) const {
    if (m_pFirstFont->IsGlyphExistInFont(c0) && m_pFirstFont->IsGlyphExistInFont(c1)) {
        return m_pFirstFont->GetKerning(c0, c1);
    }

    if (m_pSecondFont->IsGlyphExistInFont(c0) && m_pSecondFont->IsGlyphExistInFont(c1)) {
        return m_pSecondFont->GetKerning(c0, c1);
    }

    return 0;
}

/**
 * Gets the character encoding of the primary font.
 * @return character encoding
 */
CharacterCode PairFont::GetCharacterCode() const {
    return m_pFirstFont->GetCharacterCode();
}

/**
 * Sets linear filtering on both fonts.
 * @param atSmall whether to filter minified glyphs
 * @param atLarge whether to filter magnified glyphs
 */
void PairFont::SetLinearFilterEnabled(bool atSmall, bool atLarge) {
    m_pFirstFont->SetLinearFilterEnabled(atSmall, atLarge);
    m_pSecondFont->SetLinearFilterEnabled(atSmall, atLarge);
}

/**
 * Checks minification filtering of the primary font.
 * @return whether linear filtering is used
 */
bool PairFont::IsLinearFilterEnabledAtSmall() const {
    return m_pFirstFont->IsLinearFilterEnabledAtSmall();
}

/**
 * Checks magnification filtering of the primary font.
 * @return whether linear filtering is used
 */
bool PairFont::IsLinearFilterEnabledAtLarge() const {
    return m_pFirstFont->IsLinearFilterEnabledAtLarge();
}

/**
 * Gets the texture wrap and filter value of the primary font.
 * @return wrap and filter value
 */
uint32_t PairFont::GetTextureWrapFilterValue() const {
    return m_pFirstFont->GetTextureWrapFilterValue();
}

/**
 * Checks black/white interpolation of the primary font.
 * @return whether interpolation is enabled
 */
bool PairFont::IsColorBlackWhiteInterpolationEnabled() const {
    return m_pFirstFont->IsColorBlackWhiteInterpolationEnabled();
}

/**
 * Sets black/white interpolation on both fonts.
 * @param isEnabled whether interpolation is enabled
 */
void PairFont::SetColorBlackWhiteInterpolationEnabled(bool isEnabled) {
    m_pFirstFont->SetColorBlackWhiteInterpolationEnabled(isEnabled);
    m_pSecondFont->SetColorBlackWhiteInterpolationEnabled(isEnabled);
}

/**
 * Checks whether the primary font draws borders.
 * @return whether borders are drawn
 */
bool PairFont::IsBorderEffectEnabled() const {
    return m_pFirstFont->IsBorderEffectEnabled();
}

/**
 * Gets the alternate glyph from the font that would draw a character.
 * @param pGlyph destination glyph
 * @param c character code
 */
void PairFont::GetAlternateCharGlyph(Glyph* pGlyph, uint32_t c) const {
    if (m_pFirstFont->IsGlyphExistInFont(c) ||
        (!m_pSecondFont->IsGlyphExistInFont(c) && m_IsAlternateCharInFirstFont)) {
        m_pFirstFont->GetAlternateCharGlyph(pGlyph, c);
        pGlyph->baselineDifference += m_pFirstFont->GetBaselinePos() - GetBaselinePos();
    } else {
        m_pSecondFont->GetAlternateCharGlyph(pGlyph, c);
        pGlyph->baselineDifference += m_pSecondFont->GetBaselinePos() - GetBaselinePos();
    }
}

}  // namespace font
}  // namespace nn
