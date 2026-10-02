#include <eui/euiTextSearcher.h>
#include <eui/euiMessageSet.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiUtility.h>
#include <nn/ui2d/ui2d_TextBox.h>
#include <nn/ui2d/ui2d_ExtUserData.h>
namespace eui {
// pInfo receives localized text; pId selects a label or layout path. pLayout supplies the
// naming context, pTextBox supplies fallback metadata, and pRootLayout is unused here.
void TextSearcher::SearchText(TextInfo* pInfo, const char* pId, nn::ui2d::Layout* pLayout,
    nn::ui2d::TextBox* pTextBox, nn::ui2d::Layout* pRootLayout) {
    if (mMessages == nullptr) return;
    sead::FixedStringBuilder<256> name;

    if (pId != nullptr) {
        if (*pId != '-' && *pId != '@') {
            if (*pId == '=') CreateLayoutItemUniqueNameByPath(&name, pId + 1, static_cast<LayoutEx*>(pLayout));
            else name.copy(pId);
        }
    } else {
        const auto* data = pTextBox->FindExtUserDataByName("LocalizeReplaceOn");
        const char* item = data != nullptr && data->type == 0 ? static_cast<const char*>(data->GetData()) : pTextBox->mPanelName;
        CreateLayoutItemUniqueName(&name, item, static_cast<LayoutEx*>(pLayout));
    }

    if (name.getLength()) {
        const auto message = mMessages->tryFindMessage(name.cstr());

        if (message.getText() != nullptr) {
            pInfo->pText = message.getText();
            pInfo->length = message.getLength();
        }
    }

    if (pInfo->bufferLengthOverride != -1) pInfo->bufferLength = pInfo->bufferLengthOverride;
}

// pMessages supplies localized strings; pProcessor handles embedded formatting tags.
TextSearcher::TextSearcher(const MessageSet* pMessages, TagProcessor* pProcessor)
    : mMessages(pMessages), mProcessor(pProcessor) {}
}
