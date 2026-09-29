#pragma once

#include <nn/font/font_CharStrmReader.h>
#include <nn/font/font_Util.h>
#include <nn/gfx/gfx_DescriptorSlot.h>
#include <nn/gfx/gfx_Types.h>
#include <nn/types.h>

namespace nn {
namespace font {

enum FontType {
    FontType_Glyph,
    FontType_Texture,
    FontType_PackedTexture,
};

enum CharacterCode {
    CharacterCode_Unicode = 1,
    CharacterCode_Sjis = 2,
    CharacterCode_Cp1252 = 3,
};

enum FontMapMethod {
    FontMapMethod_Direct,
    FontMapMethod_Table,
    FontMapMethod_Scan,
};

typedef uint32_t TexFmt;

const uint16_t FontSheetFormatMask = 0x3fff;

struct CharWidths {
    int8_t left;
    uint8_t glyphWidth;
    uint8_t charWidth;
};

class TextureObject {
public:
    TextureObject() { Reset(); }
    virtual ~TextureObject() { Reset(); }

    void Set(const void* pImage, uint16_t format, uint16_t width, uint16_t height,
             uint8_t sheetCount, bool isColorBlackWhiteInterpolationEnabled);

    void Reset() {
        Set(nullptr, 12, 0, 0, 0, true);
        m_DescriptorSlot.Invalidate();
        m_IsInitialized = false;
    }

    virtual const nn::gfx::TextureView* GetTextureView() const = 0;
    virtual nn::gfx::TextureView* GetTextureView() = 0;

    const void* GetImage() const { return m_pImage; }
    uint16_t GetFormat() const { return m_Format; }
    uint16_t GetWidth() const { return m_Width; }
    uint16_t GetHeight() const { return m_Height; }
    uint8_t GetSheetCount() const { return m_SheetCount; }
    bool IsColorBlackWhiteInterpolationEnabled() const {
        return m_IsColorBlackWhiteInterpolationEnabled;
    }
    void SetColorBlackWhiteInterpolationEnabled(bool isEnabled) {
        m_IsColorBlackWhiteInterpolationEnabled = isEnabled;
    }
    bool IsInitialized() const { return m_IsInitialized; }
    nn::gfx::DescriptorSlot& GetDescriptorSlot() { return m_DescriptorSlot; }
    const nn::gfx::DescriptorSlot& GetDescriptorSlot() const { return m_DescriptorSlot; }

protected:
    const void* m_pImage;
    uint16_t m_Height;
    uint16_t m_Width;
    uint16_t m_Format;
    uint8_t m_SheetCount;
    bool m_IsColorBlackWhiteInterpolationEnabled;
    bool m_IsInitialized;
    nn::gfx::DescriptorSlot m_DescriptorSlot;
};

struct GlyphWidths {
    int16_t left;
    uint16_t glyphWidth;
    uint16_t charWidth;
    uint16_t rawWidth;
};

struct Glyph {
    const void* pTexture;
    GlyphWidths widths;
    uint16_t height;
    uint16_t rawHeight;
    uint16_t texWidth;
    uint16_t texHeight;
    uint16_t cellX;
    uint16_t cellY;
    uint16_t texFormat;
    uint8_t isSheetUpdated;
    uint8_t sheetIndex;
    const TextureObject* pTextureObject;
    int16_t baselineDifference;
};

typedef bool (*RegisterTextureViewSlot)(nn::gfx::DescriptorSlot* pDstSlot,
                                        const nn::gfx::TextureView& textureView, void* pUserData);
typedef void (*UnregisterTextureViewSlot)(nn::gfx::DescriptorSlot* pDstSlot,
                                          const nn::gfx::TextureView& textureView, void* pUserData);

class Font {
public:
    NN_RUNTIME_TYPEINFO_BASE();

    static const uint32_t InvalidCharCode = 0xffffffff;

    Font();
    virtual ~Font();

    /**
     * Finalizes the font.
     * @param pDevice gfx device
     */
    virtual void Finalize(nn::gfx::Device* pDevice) {}

    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual int GetAscent() const = 0;
    virtual int GetDescent() const = 0;
    virtual int GetMaxCharWidth() const = 0;
    virtual FontType GetType() const = 0;
    virtual TexFmt GetTextureFormat() const = 0;
    virtual int GetLineFeed() const = 0;
    virtual const CharWidths GetDefaultCharWidths() const = 0;
    virtual void SetLineFeed(int linefeed) = 0;
    virtual void SetDefaultCharWidths(const CharWidths& rWidths) = 0;
    virtual bool SetAlternateChar(uint32_t c) = 0;
    virtual int GetCharWidth(uint32_t c) const = 0;
    virtual const CharWidths GetCharWidths(uint32_t c) const = 0;
    virtual int GetGlyph(Glyph* pGlyph, uint32_t c) const = 0;
    virtual bool HasGlyph(uint32_t c) const = 0;

    /**
     * Checks whether a glyph is ready to be drawn.
     * @param c character code
     * @return whether the glyph is ready
     */
    virtual bool IsGlyphReady(uint32_t c) const { return HasGlyph(c); }

    /**
     * Checks whether a glyph exists in the font.
     * @param c character code
     * @return whether the glyph exists
     */
    virtual bool IsGlyphExistInFont(uint32_t c) const { return HasGlyph(c); }

    virtual int GetKerning(uint32_t c0, uint32_t c1) const = 0;
    virtual CharacterCode GetCharacterCode() const = 0;
    virtual int GetBaselinePos() const = 0;
    virtual int GetCellHeight() const = 0;
    virtual int GetCellWidth() const = 0;
    virtual void SetLinearFilterEnabled(bool atSmall, bool atLarge) = 0;
    virtual bool IsLinearFilterEnabledAtSmall() const = 0;
    virtual bool IsLinearFilterEnabledAtLarge() const = 0;
    virtual uint32_t GetTextureWrapFilterValue() const = 0;
    virtual bool IsColorBlackWhiteInterpolationEnabled() const = 0;
    virtual void SetColorBlackWhiteInterpolationEnabled(bool isEnabled) = 0;

    /**
     * Checks whether the font has border glyphs.
     * @return false
     */
    virtual bool IsBorderAvailable() const { return false; }

    virtual bool IsBorderEffectEnabled() const = 0;

    /**
     * Gets the glyph of the alternate character.
     * @param pGlyph destination glyph
     * @param c character code
     */
    virtual void GetAlternateCharGlyph(Glyph* pGlyph, uint32_t c) const {}

    const CharStrmReader GetCharStrmReader(char dummy) const;
    const CharStrmReader GetCharStrmReader(uint16_t dummy) const;

    bool IsKerningEnabled() const { return m_IsKerningEnabled; }
    void SetKerningEnabled(bool isEnabled) { m_IsKerningEnabled = isEnabled; }
    bool IsLinearFilterPaddingEnabled() const { return m_IsLinearFilterPaddingEnabled; }
    void SetLinearFilterPaddingEnabled(bool isEnabled) {
        m_IsLinearFilterPaddingEnabled = isEnabled;
    }

protected:
    bool m_IsKerningEnabled;
    bool m_IsLinearFilterPaddingEnabled;
};

}  // namespace font
}  // namespace nn
