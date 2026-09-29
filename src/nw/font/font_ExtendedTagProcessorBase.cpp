#include <nn/font/font_ExtendedTagProcessorBase.h>

#include <nn/font/font_TextWriterBase.h>
#include <nn/util.h>

namespace nn {
namespace font {

/**
 * Processes a tag, or forwards other control characters to the base processor.
 * @param code character code
 * @param pContext print context
 * @param pRect destination rectangle when calculating, nullptr when printing
 * @return the operation for the caller
 */
template <typename CharType>
typename ExtendedTagProcessorBase<CharType>::Operation
ExtendedTagProcessorBase<CharType>::ProcessTag(uint32_t code, PrintContext<CharType>* pContext,
                                               Rectangle* pRect) {
    if (code != TagBegin && code != TagEnd) {
        if (pRect != nullptr) {
            return TagProcessorBase<CharType>::CalculateRect(pRect, pContext, code);
        }
        return TagProcessorBase<CharType>::Process(code, pContext);
    }

    const TagInfo* pInfo;
    const CharType* pNext = AnalyzeTagHeader(&pInfo, pContext->str - 1);
    if (pInfo == nullptr || pNext > pContext->strEnd) {
        return TagProcessorBase<CharType>::Operation_EndDraw;
    }
    if (pInfo->group == 0 && pInfo->index == 0) {
        return ProcessTagRuby(pInfo, pContext, pRect, pNext);
    }
    return TagProcessorBase<CharType>::Operation_Default;
}

/**
 * Skips to the next printable character, skipping over tag headers.
 * @param pIsPrintable set to whether the character is printable
 * @param pStr current position in the string
 * @return the position after the character
 */
template <typename CharType>
const CharType* ExtendedTagProcessorBase<CharType>::AcquireNextPrintableChar(bool* pIsPrintable,
                                                                             const CharType* pStr) {
    const CharType* pNext = pStr;
    const uint32_t code = detail::AcquireNextChar(&pNext);
    if (code == TagBegin || code == TagEnd) {
        const TagInfo* pInfo;
        AnalyzeTagHeader(&pInfo, pStr);
        if (pInfo != nullptr) {
            if (pInfo->index == 0) {
                pNext = reinterpret_cast<const CharType*>(
                    reinterpret_cast<const RubyTagInfo*>(pInfo) + 1);
            }
            *pIsPrintable = false;
        }
    } else {
        *pIsPrintable = true;
    }
    return pNext;
}

namespace detail {

/**
 * Reads the next UTF-8 character of a string.
 * @param ppStr current position, advanced past the character
 * @return the character as UTF-32
 */
uint32_t AcquireNextChar(const char** ppStr) {
    char buffer[4] = {};
    nn::util::PickOutCharacterFromUtf8String(buffer, ppStr);
    uint32_t code = 0;
    nn::util::ConvertCharacterUtf8ToUtf32(&code, buffer);
    return code;
}

/**
 * Reads the next UTF-16 character of a string.
 * @param ppStr current position, advanced past the character
 * @return the character
 */
uint32_t AcquireNextChar(const uint16_t** ppStr) {
    const uint32_t code = **ppStr;
    (*ppStr)++;
    return code;
}

}  // namespace detail

/**
 * Parses the header of a tag.
 * @param ppInfo set to the tag header, or nullptr if the string is not at a tag
 * @param pStr position of the tag
 * @return the position after the tag
 */
template <typename CharType>
const CharType* ExtendedTagProcessorBase<CharType>::AnalyzeTagHeader(const TagInfo** ppInfo,
                                                                     const CharType* pStr) {
    if (*pStr == TagBegin) {
        *ppInfo = reinterpret_cast<const TagInfo*>(pStr);
        return reinterpret_cast<const CharType*>(reinterpret_cast<const uint8_t*>(pStr) +
                                                 (*ppInfo)->paramSize + sizeof(TagInfo));
    }
    if (*pStr == TagEnd) {
        *ppInfo = reinterpret_cast<const TagInfo*>(pStr);
        return reinterpret_cast<const CharType*>(&(*ppInfo)->paramSize);
    }
    *ppInfo = nullptr;
    return pStr;
}

/**
 * Prints ruby text above the base text of a ruby tag.
 * @param pInfo ruby tag header
 * @param pContext print context
 * @param pRect destination rectangle when calculating, nullptr when printing
 * @param pBaseText base text following the tag
 * @return the operation for the caller
 */
template <typename CharType>
typename ExtendedTagProcessorBase<CharType>::Operation
ExtendedTagProcessorBase<CharType>::ProcessTagRuby(const TagInfo* pInfo,
                                                   PrintContext<CharType>* pContext,
                                                   Rectangle* pRect, const CharType* pBaseText) {
    pContext->str = pBaseText;
    if (pRect != nullptr) {
        return TagProcessorBase<CharType>::Operation_Default;
    }

    const RubyTagInfo* pRuby = reinterpret_cast<const RubyTagInfo*>(pInfo);
    const int baseLength = pRuby->baseTextSize / sizeof(CharType);
    const int rubyLength = pRuby->rubyTextSize / sizeof(CharType);
    const CharType* pRubyText = reinterpret_cast<const CharType*>(pRuby + 1);

    float charSpace = 0.0f;
    TextWriterBase<CharType> writer(*pContext->writer);
    writer.SetDrawFlag(0x300);
    writer.SetCharSpace(charSpace);
    writer.SetLineSpace(0.0f);
    writer.SetItalicRatio(0.0f);
    writer.ResetTagProcessor();
    const float scale = pContext->writer->GetFontWidth() * 0.4f / writer.GetFont()->GetWidth();
    writer.SetScale(scale, scale);

    float baseWidth;
    float rubyWidth;
    {
        TagProcessorBase<CharType>* pTagProcessor = &pContext->writer->GetTagProcessor();
        ExtendedTagProcessorBase<CharType> tagProcessor;
        pContext->writer->SetTagProcessor(&tagProcessor);
        baseWidth = pContext->writer->CalculateStringWidth(pBaseText, baseLength);
        pContext->writer->SetTagProcessor(pTagProcessor);
        rubyWidth = writer.CalculateStringWidth(pRubyText, rubyLength);
    }

    const float diff = baseWidth - rubyWidth;
    if (diff > 0.0f) {
        const float space = diff / (rubyLength + 1);
        writer.MoveCursorX(space);
        charSpace += space;
        writer.SetCharSpace(charSpace);
    } else {
        writer.MoveCursorX(diff * 0.5f);
    }

    const TextWriterBase<CharType>& rParent = *pContext->writer;
    if (rParent.GetItalicRatio() != 0.0f) {
        writer.MoveCursorX(rParent.GetItalicRatio() * rParent.GetScaleH() *
                           rParent.GetFont()->GetWidth() * 0.5f);
    }
    if ((pContext->flags & PrintFlag_Ruby) == 0) {
        writer.MoveCursorX(pContext->writer->GetCharSpace());
    }
    writer.MoveCursorY(
        -(pContext->writer->GetScaleV() * pContext->writer->GetFont()->GetBaselinePos()));

    const Font* pFont = pContext->writer->GetFont();
    if (pFont != nullptr) {
        const int kerningValue = pFont->GetKerning(pContext->prevCode, *pBaseText);
        const float kerning = pContext->writer->GetScaleH() * kerningValue;
        if (kerning != 0.0f) {
            writer.MoveCursorX(kerning);
        }
    }

    writer.Print(pRubyText, rubyLength);
    return TagProcessorBase<CharType>::Operation_Default;
}

template class ExtendedTagProcessorBase<char>;
template class ExtendedTagProcessorBase<uint16_t>;

}  // namespace font
}  // namespace nn
