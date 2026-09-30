#pragma once
#include <nn/ui2d/ui2d_TextSearcher.h>
namespace eui {
class MessageSet;
class TagProcessor;
class TextSearcher : public nn::ui2d::TextSearcher {
public:
    TextSearcher(const MessageSet* pMessages, TagProcessor* pProcessor);
    ~TextSearcher() override;
    void SearchText(TextInfo* pInfo, const char* pId, nn::ui2d::Layout* pLayout,
                    nn::ui2d::TextBox* pTextBox, nn::ui2d::Layout* pRootLayout) override;
    const MessageSet* mMessages;
    TagProcessor* mProcessor;
};

static_assert(sizeof(TextSearcher) == 0x18, "TextSearcher size");
}
