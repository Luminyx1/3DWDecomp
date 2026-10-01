#include <eui/euiTextBoxEx.h>
#include <eui/euiMessageString.h>
#include <eui/euiLayoutEx.h>
#include <nn/ui2d/ui2d_BuildArgSet.h>
#include <nn/ui2d/ui2d_ExtUserData.h>
namespace eui {
TextBoxEx::TextBoxEx() : mLetterAnimControl(nullptr) { mTextFlags &= ~0x40; }
// pResult receives build details; pDevice supplies graphics resources; rArgs and rParams describe the text.
void TextBoxEx::InitializeString(nn::ui2d::BuildResultInformation* pResult, nn::gfx::Device* pDevice,
    const nn::ui2d::BuildArgSet& rArgs, const InitializeStringParam& rParams) {
    nn::ui2d::TextBox::InitializeString(pResult, pDevice, rArgs, rParams);
    adjustText_(static_cast<LayoutEx*>(rArgs.m_pPartsLayout));
}

// rText supplies the message; pUserData is forwarded to preprocessing callbacks.
u16 TextBoxEx::setMessageString(const MessageString& rText, void* pUserData) {
    return doSetString_(rText.getText(), rText.getLength(), nullptr, -1, false, pUserData);
}

// rText supplies the message; pHasNext receives continuation state; page selects the page;
// flag and pUserData are forwarded to preprocessing.
u16 TextBoxEx::setMessageStringWithPage(const MessageString& rText, bool* pHasNext, u32 page, bool flag, void* pUserData) {
    return doSetString_(rText.getText(), rText.getLength(), pHasNext, page, flag, pUserData);
}

// pText is null-terminated text; offset is forwarded to the explicit-length overload.
u16 TextBoxEx::SetString(const u16* pText, u16 offset) {
    size_t length = 0;

    if (*pText) {
        do { ++length; } while (pText[length]);
    }

    return SetString(pText, offset, length);
}

// pText and length describe the text; offset is unused by the preprocessing implementation.
u16 TextBoxEx::SetString(const u16* pText, u16 offset, u16 length) {
    return doSetString_(reinterpret_cast<const char16_t*>(pText), length, nullptr, -1, false, nullptr);
}

// pText and length describe the text; pHasNext, page, flag, and pUserData are forwarded to preprocessing.
u16 TextBoxEx::setStringWithPage(const char16_t* pText, u16 length, bool* pHasNext, u32 page, bool flag, void* pUserData) {
    return doSetString_(pText, length, pHasNext, page, flag, pUserData);
}

// pText and length describe text to copy directly; null clears the string.
u16 TextBoxEx::setStringNoPreproces(const char16_t* pText, u16 length) {
    if (pText != nullptr) return nn::ui2d::TextBox::SetString(reinterpret_cast<const u16*>(pText), 0, length);
    return nn::ui2d::TextBox::SetString(reinterpret_cast<const u16*>(sead::WSafeString::cEmptyString.cstr()), 0, 0);
}

// pScale optionally receives the minimum text scale when the pane defines TextScaleOn.
bool TextBoxEx::getTextAdjustMinScale_(float* pScale) {
    const auto* data = FindExtUserDataByName("TextScaleOn");

    if (data == nullptr) return false;

    if (pScale != nullptr) *pScale = *static_cast<const float*>(data->GetData());
    return true;
}

bool TextBoxEx::isWordwrapOn_() {
    const auto* data = FindExtUserDataByName("WordwrapOn");
    return (data != nullptr) && *static_cast<const s32*>(data->GetData()) != 0;
}

// pSpeed optionally receives the configured letter animation speed.
bool TextBoxEx::getLetterAnimSpeed_(float* pSpeed) {
    const auto* data = FindExtUserDataByName("LetterAnimOn");

    if (data == nullptr) return false;

    if (pSpeed != nullptr) *pSpeed = *static_cast<const float*>(data->GetData());
    return true;
}

}
