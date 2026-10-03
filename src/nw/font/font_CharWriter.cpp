// The out-of-line copies of the detail vertex helpers live in this unit.
#define NN_FONT_DETAIL_EMIT_OUT_OF_LINE

#include <nn/font/font_CharWriter.h>

#include <new>

namespace nn {
namespace font {

/**
 * Constructs a writer with default colors and scale.
 */
CharWriter::CharWriter()
    : m_Unknown8(0), m_FixedWidth(0.0f), m_ItalicRatio(0.0f), m_pFont(nullptr),
      m_pDispStringBuffer(nullptr), m_IsWidthFixed(false), m_ShadowAlpha(255) {
    m_TextColors[TextColor_Start].v[0] = 255;
    m_TextColors[TextColor_Start].v[1] = 255;
    m_TextColors[TextColor_Start].v[2] = 255;
    m_TextColors[TextColor_Start].v[3] = 255;
    m_TextColors[TextColor_End].v[0] = 255;
    m_TextColors[TextColor_End].v[1] = 255;
    m_TextColors[TextColor_End].v[2] = 255;
    m_TextColors[TextColor_End].v[3] = 255;
    m_Scale.x = 1.0f;
    m_Scale.y = 1.0f;
    m_CursorPos.x = 0.0f;
    m_CursorPos.y = 0.0f;
    m_CursorPos.z = 0.0f;
}

/**
 * Destroys the writer.
 */
CharWriter::~CharWriter() {}

/**
 * Sets the scale from a font size.
 * @param width font width
 * @param height font height
 */
void CharWriter::SetFontSize(float width, float height) {
    SetScale(width / m_pFont->GetWidth(), height / m_pFont->GetHeight());
}

/**
 * Sets a uniform scale from a font height.
 * @param height font height
 */
void CharWriter::SetFontSize(float height) {
    const float scale = height / m_pFont->GetHeight();
    m_Scale.x = scale;
    m_Scale.y = scale;
}

/**
 * Gets the scaled font width.
 * @return font width
 */
float CharWriter::GetFontWidth() const {
    return m_pFont->GetWidth() * m_Scale.x;
}

/**
 * Gets the scaled font height.
 * @return font height
 */
float CharWriter::GetFontHeight() const {
    return m_pFont->GetHeight() * m_Scale.y;
}

/**
 * Gets the scaled font ascent.
 * @return font ascent
 */
float CharWriter::GetFontAscent() const {
    return m_pFont->GetAscent() * m_Scale.y;
}

/**
 * Gets the scaled font descent.
 * @return font descent
 */
float CharWriter::GetFontDescent() const {
    return m_pFont->GetDescent() * m_Scale.y;
}

/**
 * Prints a glyph at the cursor and advances it.
 * @param rGlyph glyph to print
 * @return advance width
 */
float CharWriter::PrintGlyph(const Glyph& rGlyph) {
    float width;
    float left;

    if (m_IsWidthFixed) {
        const float margin = (m_FixedWidth - rGlyph.widths.charWidth * m_Scale.x) * 0.5f;
        width = m_FixedWidth;
        left = margin + rGlyph.widths.left * m_Scale.x;
    } else {
        width = rGlyph.widths.charWidth * m_Scale.x;
        left = rGlyph.widths.left * m_Scale.x;
    }

    PrintGlyph(left + m_CursorPos.x, rGlyph);
    m_CursorPos.x = width + m_CursorPos.x;
    return width;
}

void CharWriter::PrintGlyph(float x, const Glyph& rGlyph) {
    float u0;
    float u1;
    float v0;
    float v1;
    float width;
    float height;
    float y;

    if (m_pFont->IsLinearFilterPaddingEnabled()) {
        u0 = (rGlyph.cellX - 0.5f) / rGlyph.texWidth;
        u1 = (static_cast<int>(rGlyph.cellX + rGlyph.widths.rawWidth) + 0.5f) / rGlyph.texWidth;
        v0 = (rGlyph.cellY - 0.5f) / rGlyph.texHeight;
        v1 = (static_cast<int>(rGlyph.cellY + rGlyph.rawHeight) + 0.5f) / rGlyph.texHeight;
        x -= m_Scale.x * 0.5f;
        width = m_Scale.x * (rGlyph.widths.glyphWidth + 1.0f);
        height = m_Scale.y * (rGlyph.height + 1.0f);
        y = m_CursorPos.y - m_Scale.y * 0.5f;
    } else {
        u0 = rGlyph.cellX / static_cast<float>(rGlyph.texWidth);
        u1 = static_cast<int>(rGlyph.cellX + rGlyph.widths.rawWidth) /
             static_cast<float>(rGlyph.texWidth);
        v0 = rGlyph.cellY / static_cast<float>(rGlyph.texHeight);
        v1 = static_cast<int>(rGlyph.cellY + rGlyph.rawHeight) /
             static_cast<float>(rGlyph.texHeight);
        width = m_Scale.x * rGlyph.widths.glyphWidth;
        height = m_Scale.y * rGlyph.height;
        y = m_CursorPos.y;
    }

    DispStringBuffer* pBuffer = m_pDispStringBuffer;

    if (pBuffer->m_CharCount >= pBuffer->m_CharCountMax) {
        return;
    }

    const int index = pBuffer->m_CharCount++;
    detail::CharAttribute* pAttr = &m_pDispStringBuffer->m_pCharAttrs[index];

    pAttr->pos.x = width;
    pAttr->pos.y = height;
    pAttr->pos.z = x;
    pAttr->pos.w = y;
    pAttr->color[TextColor_Start] = m_TextColors[TextColor_Start];
    pAttr->color[TextColor_End] = m_TextColors[TextColor_End];
    pAttr->shadowAlpha = m_ShadowAlpha;
    pAttr->tex.x = u0;
    pAttr->tex.y = 1.0f - v0;
    pAttr->tex.z = u1;
    pAttr->tex.w = 1.0f - v1;
    pAttr->italicOffset =
        static_cast<int16_t>(m_ItalicRatio * m_Scale.x * m_pFont->GetWidth());

    const float fontHeight = m_pFont->GetHeight() * m_Scale.y;
    m_pDispStringBuffer->SetFontHeight(m_pFont->IsLinearFilterPaddingEnabled() ?
                                           m_Scale.y + fontHeight :
                                           fontHeight);
    pAttr->sheetIndex = rGlyph.sheetIndex;
    const bool isBorder = m_pFont->IsBorderEffectEnabled();
    pAttr->pTexObjAndFlag = reinterpret_cast<uintptr_t>(rGlyph.pTextureObject) | (isBorder ? 1 : 0);
}

/**
 * Prints a character at the cursor and advances it.
 * @param code character code
 * @return advance width
 */
float CharWriter::Print(uint32_t code) {
    Glyph glyph;
    m_pFont->GetGlyph(&glyph, code);
    return PrintGlyph(glyph);
}

/**
 * Draws a glyph at the cursor and advances it by the glyph width.
 * @param rGlyph glyph to print
 */
void CharWriter::DrawGlyph(const Glyph& rGlyph) {
    PrintGlyph(m_CursorPos.x, rGlyph);
    m_CursorPos.x += rGlyph.widths.glyphWidth * m_Scale.x;
}

/**
 * Starts printing into the display string buffer.
 */
void CharWriter::StartPrint() {
    m_pDispStringBuffer->m_CharCount = 0;
}

/**
 * Ends printing.
 */
void CharWriter::EndPrint() const {}

/**
 * Gets the memory size of a display string buffer.
 * @param charCount maximum number of characters
 * @return required size
 */
size_t CharWriter::GetDispStringBufferSize(uint32_t charCount) {
    DispStringBuffer::InitializeArg arg;
    arg.charCountMax = charCount;
    return DispStringBuffer::GetRequiredDrawBufferSize(arg) + sizeof(DispStringBuffer);
}

/**
 * Gets the constant buffer size of a display string buffer.
 * @param pDevice gfx device
 * @param charCount maximum number of characters
 * @param isShadowEnabled whether the shadow is drawn
 * @return required size
 */
size_t CharWriter::GetConstantBufferSize(nn::gfx::Device* pDevice, int charCount,
                                         bool isShadowEnabled) {
    DispStringBuffer::InitializeArg arg;
    arg.charCountMax = charCount;
    arg.isShadowEnabled = isShadowEnabled;
    return DispStringBuffer::GetRequiredConstantBufferSize(pDevice, arg);
}

DispStringBuffer* CharWriter::InitializeDispStringBuffer(nn::gfx::Device* pDevice, void* pBuffer,
                                                         uint32_t charCount,
                                                         bool isShadowEnabled) {
    DispStringBuffer* pDispStringBuffer = new (pBuffer) DispStringBuffer();
    DispStringBuffer::InitializeArg arg;
    arg.pDrawBuffer = static_cast<uint8_t*>(pBuffer) + sizeof(DispStringBuffer);
    arg.charCountMax = charCount;
    arg.isShadowEnabled = isShadowEnabled;
    pDispStringBuffer->Initialize(pDevice, arg);
    return pDispStringBuffer;
}


}  // namespace font
}  // namespace nn
