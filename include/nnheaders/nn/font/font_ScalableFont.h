#pragma once

#include <nn/font/font_Font.h>
#include <nn/font/font_TextureCache.h>

namespace nn {
namespace font {

class ScalableFont : public Font {
public:
    NN_RUNTIME_TYPEINFO(Font);

    enum AlternateCharMode {
        AlternateCharMode_UseWhiteSpace,
        AlternateCharMode_UseWhiteSpaceWhenNotReady,
        AlternateCharMode_UseAlternateChar,
    };

    struct InitializeArg {
        TextureCache* pTextureCache;
        u32 fontSize;
        u16 fontFace;
        bool isAlternateCharSpaceWithoutOtherGlyph;
        bool isDrawWhiteSpaceWhenGlyphNotReady;
        u32 alternateChar;
        int lineFeedOffset;
        bool isNoBreakHyphenReplaced;

        void SetDefault();
    };

    ScalableFont();
    ~ScalableFont() override;

    void Initialize(const InitializeArg& rArg);
    void RegisterAlternateCharGlyph() const;

    int GetWidth() const override;
    int GetHeight() const override;
    int GetAscent() const override;
    int GetDescent() const override;
    int GetMaxCharWidth() const override;
    FontType GetType() const override;
    TexFmt GetTextureFormat() const override;
    int GetLineFeed() const override;
    const CharWidths GetDefaultCharWidths() const override;
    void SetLineFeed(int linefeed) override;
    void SetDefaultCharWidths(const CharWidths& rWidths) override;
    bool SetAlternateChar(uint32_t c) override;
    int GetCharWidth(uint32_t c) const override;
    const CharWidths GetCharWidths(uint32_t c) const override;
    int GetGlyph(Glyph* pGlyph, uint32_t c) const override;
    bool HasGlyph(uint32_t c) const override;

    /**
     * Checks whether a glyph exists in the fonts of the font face.
     * @param c character code
     * @return whether the glyph exists
     */
    bool IsGlyphExistInFont(uint32_t c) const override {
        return m_pTextureCache->IsGlyphExistInFont(c, m_FontFace);
    }

    int GetKerning(uint32_t c0, uint32_t c1) const override;
    CharacterCode GetCharacterCode() const override;
    int GetBaselinePos() const override;
    int GetCellHeight() const override;
    int GetCellWidth() const override;
    void SetLinearFilterEnabled(bool atSmall, bool atLarge) override;
    bool IsLinearFilterEnabledAtSmall() const override;
    bool IsLinearFilterEnabledAtLarge() const override;
    uint32_t GetTextureWrapFilterValue() const override;
    bool IsColorBlackWhiteInterpolationEnabled() const override;
    void SetColorBlackWhiteInterpolationEnabled(bool isEnabled) override;

    /**
     * Checks whether the font has border glyphs.
     * @return true
     */
    bool IsBorderAvailable() const override { return true; }

    bool IsBorderEffectEnabled() const override;
    void GetAlternateCharGlyph(Glyph* pGlyph, uint32_t c) const override;

    GlyphNode* FindAlternateCharGlyphNode(uint32_t c) const;
    void SetGlyphNodeToGlyph(Glyph* pGlyph, GlyphNode* pNode, int width) const;

private:
    uint32_t ConvertCode(uint32_t c) const {
        return m_IsNoBreakHyphenReplaced && c == 0x2011 ? '-' : c;
    }

    TextureCache* m_pTextureCache;
    u32 m_FontSize;
    int m_Height;
    int m_Ascent;
    int m_BaselinePos;
    int m_LineFeed;
    CharWidths m_DefaultCharWidths;
    u8 m_AlternateCharMode;
    bool m_IsNoBreakHyphenReplaced;
    u16 m_FontFace;
    u32 m_AlternateChar;
};
static_assert(sizeof(ScalableFont) == 0x38);

}  // namespace font
}  // namespace nn
