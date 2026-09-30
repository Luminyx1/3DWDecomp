#pragma once
namespace nn::ui2d {
class Layout;
class TextBox;
class TextSearcher {
public:
    struct TextInfo;
    struct TextInfoUtf8;
    virtual ~TextSearcher() = default;
    virtual void SearchText(TextInfo* pInfo, const char* pId, Layout* pLayout,
                            TextBox* pTextBox, Layout* pRootLayout) = 0;
    // The default searcher supplies no UTF-8 text for any identifier or layout.
    virtual void SearchTextUtf8(TextInfoUtf8* pInfo, const char* pId, Layout* pLayout,
                                TextBox* pTextBox, Layout* pRootLayout) {}
};
}
