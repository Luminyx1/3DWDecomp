#pragma once

#include <nn/font/font_TagProcessorBase.h>

namespace nn {
namespace font {

template <typename CharType>
class ExtendedTagProcessorBase : public TagProcessorBase<CharType> {
public:
    NN_RUNTIME_TYPEINFO(TagProcessorBase<CharType>);

    typedef typename TagProcessorBase<CharType>::Operation Operation;

    static const CharType TagBegin = 0x0e;
    static const CharType TagEnd = 0x0f;

    struct TagInfo {
        CharType code;
        uint16_t group;
        uint16_t index;
        uint16_t paramSize;
    };

    struct RubyTagInfo {
        TagInfo header;
        uint16_t baseTextSize;
        uint16_t rubyTextSize;
    };

    /**
     * Processes a control character or tag while printing.
     * @param code character code
     * @param pContext print context
     * @return the operation for the caller
     */
    Operation Process(uint32_t code, PrintContext<CharType>* pContext) override {
        return ProcessTag(code, pContext, nullptr);
    }

    /**
     * Calculates the area covered by a control character or tag.
     * @param pRect destination rectangle
     * @param pContext print context
     * @param code character code
     * @return the operation for the caller
     */
    Operation CalculateRect(Rectangle* pRect, PrintContext<CharType>* pContext,
                            uint32_t code) override {
        return ProcessTag(code, pContext, pRect);
    }

    const CharType* AcquireNextPrintableChar(bool* pIsPrintable, const CharType* pStr) override;

protected:
    Operation ProcessTag(uint32_t code, PrintContext<CharType>* pContext, Rectangle* pRect);
    const CharType* AnalyzeTagHeader(const TagInfo** ppInfo, const CharType* pStr);
    Operation ProcessTagRuby(const TagInfo* pInfo, PrintContext<CharType>* pContext,
                             Rectangle* pRect, const CharType* pBaseText);
};

namespace detail {

uint32_t AcquireNextChar(const char** ppStr);
uint32_t AcquireNextChar(const uint16_t** ppStr);

}  // namespace detail

}  // namespace font
}  // namespace nn
