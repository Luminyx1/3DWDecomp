#include <nn/font/font_Font.h>

namespace nn {
namespace font {

/**
 * Sets the image and properties of the texture.
 * @param pImage texture image
 * @param format sheet format
 * @param width sheet width
 * @param height sheet height
 * @param sheetCount number of sheets
 * @param isColorBlackWhiteInterpolationEnabled whether black/white color interpolation is enabled
 */
void TextureObject::Set(const void* pImage, uint16_t format, uint16_t width, uint16_t height,
                        uint8_t sheetCount, bool isColorBlackWhiteInterpolationEnabled) {
    m_pImage = pImage;
    m_Format = format;
    m_Width = width;
    m_Height = height;
    m_SheetCount = sheetCount;
    m_IsColorBlackWhiteInterpolationEnabled = isColorBlackWhiteInterpolationEnabled;
    m_IsInitialized = true;
}

/**
 * Constructs a font with kerning enabled.
 */
Font::Font() : m_IsKerningEnabled(true), m_IsLinearFilterPaddingEnabled(true) {}

/**
 * Destroys the font.
 */
Font::~Font() = default;

/**
 * Gets a stream reader matching the character code of the font for char strings.
 * @param dummy selects the char overload
 * @return the stream reader
 */
const CharStrmReader Font::GetCharStrmReader(char dummy) const {
    CharStrmReader::ReadNextCharFunc func = nullptr;

    switch (GetCharacterCode()) {
    case CharacterCode_Unicode:
        func = &CharStrmReader::ReadNextCharUtf8;
        break;
    case CharacterCode_Sjis:
        func = &CharStrmReader::ReadNextCharSjis;
        break;
    case CharacterCode_Cp1252:
        func = &CharStrmReader::ReadNextCharCp1252;
        break;
    }

    return CharStrmReader(func);
}

/**
 * Gets a stream reader matching the character code of the font for UTF-16 strings.
 * @param dummy selects the uint16_t overload
 * @return the stream reader
 */
const CharStrmReader Font::GetCharStrmReader(uint16_t dummy) const {
    CharStrmReader::ReadNextCharFunc func = nullptr;

    switch (GetCharacterCode()) {
    case CharacterCode_Unicode:
        func = &CharStrmReader::ReadNextCharUtf16;
        break;
    default:
        break;
    }

    return CharStrmReader(func);
}

}  // namespace font
}  // namespace nn
