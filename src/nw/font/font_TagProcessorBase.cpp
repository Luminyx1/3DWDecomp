#include <nn/font/font_TagProcessorBase.h>

#include <nn/font/font_TextWriterBase.h>
#include <nn/util.h>

namespace nn {
namespace font {

/**
 * Constructs a tag processor.
 */
template <typename CharType>
TagProcessorBase<CharType>::TagProcessorBase() = default;

/**
 * Destroys the tag processor.
 */
template <typename CharType>
TagProcessorBase<CharType>::~TagProcessorBase() = default;

/**
 * Processes a control character while printing.
 * @param code character code
 * @param pContext print context
 * @return the operation for the caller
 */
template <typename CharType>
typename TagProcessorBase<CharType>::Operation TagProcessorBase<CharType>::Process(
    uint32_t code, PrintContext<CharType>* pContext) {
    switch (code) {
    case '\n':
        ProcessLinefeed(pContext);
        return Operation_NextLine;
    case '\t':
        ProcessTab(pContext);
        return Operation_NoCharSpace;
    default:
        return Operation_Default;
    }
}

/**
 * Moves the cursor to the start of the next line.
 * @param pContext print context
 */
template <typename CharType>
void TagProcessorBase<CharType>::ProcessLinefeed(PrintContext<CharType>* pContext) const {
    TextWriterBase<CharType>& rWriter = *pContext->writer;
    const float x = pContext->xOrigin;
    const float y = rWriter.GetCursorY() + GetLineHeight(&rWriter);
    rWriter.SetCursorX(x);
    rWriter.SetCursorY(y);
}

/**
 * Moves the cursor to the next tab stop.
 * @param pContext print context
 */
template <typename CharType>
void TagProcessorBase<CharType>::ProcessTab(PrintContext<CharType>* pContext) const {
    TextWriterBase<CharType>& rWriter = *pContext->writer;
    const int tabWidth = rWriter.GetTabWidth();
    if (tabWidth > 0) {
        const float charWidth =
            rWriter.IsWidthFixed() ? rWriter.GetFixedWidth() : rWriter.GetFontWidth();
        const float dx = rWriter.GetCursorX() - pContext->xOrigin;
        const float tabPixel = tabWidth * charWidth;
        const int tabCount = static_cast<int>(dx / tabPixel) + 1;
        const float x = pContext->xOrigin + tabPixel * tabCount;
        rWriter.SetCursorX(x);
    }
}

/**
 * Calculates the area covered by a control character.
 * @param pRect destination rectangle
 * @param pContext print context
 * @param code character code
 * @return the operation for the caller
 */
template <typename CharType>
typename TagProcessorBase<CharType>::Operation TagProcessorBase<CharType>::CalculateRect(
    Rectangle* pRect, PrintContext<CharType>* pContext, uint32_t code) {
    switch (code) {
    case '\n': {
        const TextWriterBase<CharType>& rWriter = *pContext->writer;
        pRect->right = rWriter.GetCursorX();
        pRect->top = rWriter.GetCursorY();
        ProcessLinefeed(pContext);
        pRect->left = rWriter.GetCursorX();
        pRect->bottom = rWriter.GetCursorY();
        if (!pContext->writer->GetLinefeedByCharacterHeightEnabled()) {
            pRect->bottom += pContext->writer->GetFontHeight();
        }
        pRect->Normalize();
        return Operation_NextLine;
    }
    case '\t': {
        const TextWriterBase<CharType>& rWriter = *pContext->writer;
        pRect->left = rWriter.GetCursorX();
        ProcessTab(pContext);
        pRect->right = rWriter.GetCursorX();
        pRect->top = rWriter.GetCursorY();
        pRect->bottom = pRect->top + rWriter.GetFontHeight();
        pRect->Normalize();
        return Operation_NoCharSpace;
    }
    default:
        return Operation_Default;
    }
}

/**
 * Gets the line height of a text writer.
 * @param pWriter text writer
 * @return the line height
 */
template <typename CharType>
float TagProcessorBase<CharType>::GetLineHeight(const TextWriterBase<CharType>* pWriter) const {
    return pWriter->GetLineHeight();
}

/**
 * Called before printing a string.
 * @param pContext print context
 */
template <typename CharType>
void TagProcessorBase<CharType>::BeginPrint(PrintContext<CharType>* pContext) {}

/**
 * Called after printing a string.
 * @param pContext print context
 */
template <typename CharType>
void TagProcessorBase<CharType>::EndPrint(PrintContext<CharType>* pContext) {}

/**
 * Called before calculating the area of a string.
 * @param pContext print context
 */
template <typename CharType>
void TagProcessorBase<CharType>::BeginCalculateRect(PrintContext<CharType>* pContext) {}

/**
 * Called after calculating the area of a string.
 * @param pContext print context
 */
template <typename CharType>
void TagProcessorBase<CharType>::EndCalculateRect(PrintContext<CharType>* pContext) {}

/**
 * Called before printing a whole text.
 * @param pWriter text writer
 * @param pStr start of the text
 * @param pStrEnd end of the text
 */
template <typename CharType>
void TagProcessorBase<CharType>::BeginPrintWhole(const TextWriterBase<CharType>* pWriter,
                                                 const CharType* pStr, const CharType* pStrEnd) {}

/**
 * Called after printing a whole text.
 * @param pWriter text writer
 * @param pStr start of the text
 * @param pStrEnd end of the text
 */
template <typename CharType>
void TagProcessorBase<CharType>::EndPrintWhole(const TextWriterBase<CharType>* pWriter,
                                               const CharType* pStr, const CharType* pStrEnd) {}

/**
 * Called before calculating the area of a whole text.
 * @param pWriter text writer
 * @param pStr start of the text
 * @param pStrEnd end of the text
 */
template <typename CharType>
void TagProcessorBase<CharType>::BeginCalculateRectWhole(const TextWriterBase<CharType>* pWriter,
                                                         const CharType* pStr,
                                                         const CharType* pStrEnd) {}

/**
 * Called after calculating the area of a whole text.
 * @param pWriter text writer
 * @param pStr start of the text
 * @param pStrEnd end of the text
 */
template <typename CharType>
void TagProcessorBase<CharType>::EndCalculateRectWhole(const TextWriterBase<CharType>* pWriter,
                                                       const CharType* pStr,
                                                       const CharType* pStrEnd) {}

/**
 * Skips to the next printable character.
 * @param pIsPrintable set to whether the character is printable
 * @param pStr current position in the string
 * @return the position after the character
 */
template <typename CharType>
const CharType* TagProcessorBase<CharType>::AcquireNextPrintableChar(bool* pIsPrintable,
                                                                     const CharType* pStr) {
    return detail::AcquireNextPrinableCharImpl(pIsPrintable, pStr);
}

namespace detail {

/**
 * Skips one UTF-8 character.
 * @param pIsPrintable set to true
 * @param pStr current position in the string
 * @return the position after the character
 */
const char* AcquireNextPrinableCharImpl(bool* pIsPrintable, const char* pStr) {
    char buffer[4] = {};
    nn::util::PickOutCharacterFromUtf8String(buffer, &pStr);
    *pIsPrintable = true;
    return pStr;
}

/**
 * Skips one UTF-16 character.
 * @param pIsPrintable set to true
 * @param pStr current position in the string
 * @return the position after the character
 */
const uint16_t* AcquireNextPrinableCharImpl(bool* pIsPrintable, const uint16_t* pStr) {
    *pIsPrintable = true;
    return pStr + 1;
}

}  // namespace detail

template class TagProcessorBase<char>;
template class TagProcessorBase<uint16_t>;

}  // namespace font
}  // namespace nn
