#include <nn/font/font_CharStrmReader.h>
#include <nn/util.h>

namespace nn {
namespace font {

namespace {

bool IsSjisLeadByte(uint8_t c) {
    return (c >= 0x81 && c < 0xa0) || c >= 0xe0;
}

}  // namespace

/**
 * Reads the next UTF-8 character of the stream.
 * @return the character as UTF-32
 */
uint32_t CharStrmReader::ReadNextCharUtf8() {
    uint32_t code;
    char buffer[4];
    nn::util::PickOutCharacterFromUtf8String(buffer, reinterpret_cast<const char**>(&m_pCharStrm));
    code = 0;
    nn::util::ConvertCharacterUtf8ToUtf32(&code, buffer);
    return code;
}

/**
 * Reads the next UTF-16 character of the stream.
 * @return the character
 */
uint32_t CharStrmReader::ReadNextCharUtf16() {
    uint32_t code = *GetChar<uint16_t>();
    StepStrm<uint16_t>();
    return code;
}

/**
 * Reads the next CP1252 character of the stream.
 * @return the character
 */
uint32_t CharStrmReader::ReadNextCharCp1252() {
    uint32_t code = *GetChar<uint8_t>();
    StepStrm<uint8_t>();
    return code;
}

/**
 * Reads the next Shift-JIS character of the stream.
 * @return the character
 */
uint32_t CharStrmReader::ReadNextCharSjis() {
    uint32_t code;
    if (IsSjisLeadByte(*GetChar<uint8_t>())) {
        code = (*GetChar<uint8_t>() << 8) | *GetChar<uint8_t>(1);
        StepStrm<uint8_t>(2);
    } else {
        code = *GetChar<uint8_t>();
        StepStrm<uint8_t>(1);
    }
    return code;
}

}  // namespace font
}  // namespace nn
