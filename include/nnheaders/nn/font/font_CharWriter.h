#pragma once

#include <nn/font/font_DispStringBuffer.h>
#include <nn/font/font_Font.h>
#include <nn/gfx/gfx_Device.h>
#include <nn/types.h>
#include <nn/util/util_MathTypes.h>

namespace nn {
namespace font {

class CharWriter {
public:
    enum TextColor {
        TextColor_Start,
        TextColor_End,
        TextColor_MaxTextColor,
    };

    CharWriter();
    ~CharWriter();

    void SetFontSize(float width, float height);
    void SetFontSize(float height);
    float GetFontWidth() const;
    float GetFontHeight() const;
    float GetFontAscent() const;
    float GetFontDescent() const;

    float PrintGlyph(const Glyph& rGlyph);
    void PrintGlyph(float x, const Glyph& rGlyph);
    float Print(uint32_t code);
    void DrawGlyph(const Glyph& rGlyph);
    void StartPrint();
    void EndPrint() const;

    static size_t GetDispStringBufferSize(uint32_t charCount);
    static size_t GetConstantBufferSize(nn::gfx::Device* pDevice, int charCount,
                                        bool isShadowEnabled);
    static DispStringBuffer* InitializeDispStringBuffer(nn::gfx::Device* pDevice, void* pBuffer,
                                                        uint32_t charCount,
                                                        bool isShadowEnabled);

    void SetTextColor(const nn::util::Unorm8x4& rColor) {
        m_TextColors[TextColor_Start] = rColor;
        m_TextColors[TextColor_End] = rColor;
    }
    void SetGradationColor(const nn::util::Unorm8x4& rStart, const nn::util::Unorm8x4& rEnd) {
        m_TextColors[TextColor_Start] = rStart;
        m_TextColors[TextColor_End] = rEnd;
    }
    const nn::util::Unorm8x4& GetTextColor(TextColor type) const { return m_TextColors[type]; }

    void SetScale(float x, float y) {
        m_Scale.x = x;
        m_Scale.y = y;
    }
    float GetScaleH() const { return m_Scale.x; }
    float GetScaleV() const { return m_Scale.y; }

    void SetCursor(float x, float y) {
        m_CursorPos.x = x;
        m_CursorPos.y = y;
    }
    void SetCursor(float x, float y, float z) {
        m_CursorPos.x = x;
        m_CursorPos.y = y;
        m_CursorPos.z = z;
    }
    void SetCursorX(float x) { m_CursorPos.x = x; }
    void SetCursorY(float y) { m_CursorPos.y = y; }
    void SetCursorZ(float z) { m_CursorPos.z = z; }
    void MoveCursorX(float dx) { m_CursorPos.x += dx; }
    void MoveCursorY(float dy) { m_CursorPos.y += dy; }
    float GetCursorX() const { return m_CursorPos.x; }
    float GetCursorY() const { return m_CursorPos.y; }
    float GetCursorZ() const { return m_CursorPos.z; }

    void SetFixedWidth(float width) { m_FixedWidth = width; }
    float GetFixedWidth() const { return m_FixedWidth; }
    void SetWidthFixed(bool isFixed) { m_IsWidthFixed = isFixed; }
    bool IsWidthFixed() const { return m_IsWidthFixed; }

    void SetItalicRatio(float ratio) { m_ItalicRatio = ratio; }
    float GetItalicRatio() const { return m_ItalicRatio; }

    void SetFont(const Font* pFont) { m_pFont = pFont; }
    const Font* GetFont() const { return m_pFont; }

    void SetDispStringBuffer(DispStringBuffer* pBuffer) { m_pDispStringBuffer = pBuffer; }
    DispStringBuffer* GetDispStringBuffer() const { return m_pDispStringBuffer; }

    void SetShadowAlpha(uint8_t alpha) { m_ShadowAlpha = alpha; }
    uint8_t GetShadowAlpha() const { return m_ShadowAlpha; }

protected:
    nn::util::Unorm8x4 m_TextColors[TextColor_MaxTextColor];
    uint32_t m_Unknown8;
    nn::util::Float2 m_Scale;
    nn::util::Float3 m_CursorPos;
    float m_FixedWidth;
    float m_ItalicRatio;
    const Font* m_pFont;
    DispStringBuffer* m_pDispStringBuffer;
    bool m_IsWidthFixed;
    uint8_t m_ShadowAlpha;
};

}  // namespace font
}  // namespace nn
