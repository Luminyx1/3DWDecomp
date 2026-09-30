#include <nn/ui2d/ui2d_TextBox.h>
#include <nn/util.h>
namespace nn::ui2d {
// current is the previous UTF-8 cursor; next must advance within the text buffer.
bool TextBox::ValidateNextPrintableChar(const char* current, const char* next) {
    return (next <= static_cast<const char*>(mTextBuffer) + mTextLength) & (next > current);
}
// current is the previous UTF-16 cursor; next must advance within the text buffer.
bool TextBox::ValidateNextPrintableChar(const u16* current, const u16* next) {
    return (next <= static_cast<const u16*>(mTextBuffer) + mTextLength) & (next > current);
}
// text points to the UTF-8 character to decode.
u32 TextBox::GetCharFromPointer(const char* text) {
    char character[4] = {};
    nn::util::PickOutCharacterFromUtf8String(character, &text);
    u32 value = 0;
    nn::util::ConvertCharacterUtf8ToUtf32(&value, character);
    return value;
}
// text points to a UTF-16 code unit.
u32 TextBox::GetCharFromPointer(const u16* text) { return *text; }
u32 TextBox::GetMaterialCount() const { return mMaterial != nullptr; }
// index zero selects the text material; other indices have no material.
Material* TextBox::GetMaterial(int index) const { GetMaterialCount(); return index == 0 ? mMaterial : nullptr; }
}
