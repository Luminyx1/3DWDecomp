#include <nn/font/font_TextWriterBase.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <nn/util.h>

namespace nn {
namespace font {

template <typename CharType>
TagProcessorBase<CharType> TextWriterBase<CharType>::g_DefaultTagProcessor;

template <typename CharType>
ExtendedTagProcessorBase<CharType> TextWriterBase<CharType>::g_ExtendedTagProcessor;

/**
 * Constructs a writer with default settings.
 */
template <typename CharType>
TextWriterBase<CharType>::TextWriterBase()
    : m_WidthLimit(FLT_MAX), m_CharSpace(0.0f), m_LineSpace(0.0f), m_WidthLimitOffset(0.0f),
      m_TabWidth(DefaultTabWidth), m_DrawFlag(0), m_pTagProcessor(&g_DefaultTagProcessor),
      m_IsCenterCeilingEnabled(false), m_IsLinefeedKerningEnabled(false),
      m_IsLinefeedByCharacterHeightEnabled(false), m_IsIgnoringKerningWithFixedWidth(true) {}

/**
 * Destroys the writer.
 */
template <typename CharType>
TextWriterBase<CharType>::~TextWriterBase() {}

/**
 * Sets the line space from a line height.
 * @param height font height
 */
template <typename CharType>
void TextWriterBase<CharType>::SetLineHeight(float height) {
    const Font* pFont = GetFont();
    int lineFeed = pFont != nullptr ? pFont->GetLineFeed() : 0;
    m_LineSpace = height - lineFeed * GetScaleV();
}

/**
 * Gets the line height.
 * @return line height
 */
template <typename CharType>
float TextWriterBase<CharType>::GetLineHeight() const {
    const Font* pFont = GetFont();
    int lineFeed = pFont != nullptr ? pFont->GetLineFeed() : 0;
    return lineFeed * GetScaleV() + m_LineSpace;
}

/**
 * Calculates the width of a formatted string.
 * @param pFormat format string
 * @return width
 */
template <typename CharType>
float TextWriterBase<CharType>::CalculateFormatStringWidth(const CharType* pFormat, ...) const {
    std::va_list args;
    va_start(args, pFormat);
    Rectangle rect = {};
    CalculateVStringRect(&rect, pFormat, args);
    va_end(args);
    return rect.GetWidth();
}

/**
 * Calculates the height of a formatted string.
 * @param pFormat format string
 * @return height
 */
template <typename CharType>
float TextWriterBase<CharType>::CalculateFormatStringHeight(const CharType* pFormat, ...) const {
    std::va_list args;
    va_start(args, pFormat);
    Rectangle rect = {};
    CalculateVStringRect(&rect, pFormat, args);
    va_end(args);
    return rect.GetHeight();
}

/**
 * Calculates the rectangle of a formatted string.
 * @param pRect destination rectangle
 * @param pFormat format string
 */
template <typename CharType>
void TextWriterBase<CharType>::CalculateFormatStringRect(Rectangle* pRect,
                                                         const CharType* pFormat, ...) const {
    std::va_list args;
    va_start(args, pFormat);
    CalculateVStringRect(pRect, pFormat, args);
    va_end(args);
}

/**
 * Calculates the rectangle of a formatted string.
 * @param pRect destination rectangle
 * @param pFormat format string
 * @param args format arguments
 */
template <typename CharType>
void TextWriterBase<CharType>::CalculateVStringRect(Rectangle* pRect, const CharType* pFormat,
                                                    std::va_list args) const {
    CharType buffer[FormatBufferSize];
    int length = std::min(VSNPrintf(buffer, FormatBufferSize, pFormat, args), FormatBufferSize - 1);
    CalculateStringRect(pRect, buffer, length);
}

/**
 * Calculates the width of a string.
 * @param pStr string
 * @return width
 */
template <typename CharType>
float TextWriterBase<CharType>::CalculateStringWidth(const CharType* pStr) const {
    return CalculateStringWidth(pStr, StrLen(pStr));
}

/**
 * Calculates the width of a string.
 * @param pStr string
 * @param length string length
 * @return width
 */
template <typename CharType>
float TextWriterBase<CharType>::CalculateStringWidth(const CharType* pStr, int length) const {
    Rectangle rect = {};
    CalculateStringRect(&rect, pStr, length);
    return rect.GetWidth();
}

/**
 * Calculates the height of a string.
 * @param pStr string
 * @return height
 */
template <typename CharType>
float TextWriterBase<CharType>::CalculateStringHeight(const CharType* pStr) const {
    return CalculateStringHeight(pStr, StrLen(pStr));
}

/**
 * Calculates the height of a string.
 * @param pStr string
 * @param length string length
 * @return height
 */
template <typename CharType>
float TextWriterBase<CharType>::CalculateStringHeight(const CharType* pStr, int length) const {
    Rectangle rect = {};
    CalculateStringRect(&rect, pStr, length);
    return rect.GetHeight();
}

/**
 * Calculates the rectangle of a string.
 * @param pRect destination rectangle
 * @param pStr string
 */
template <typename CharType>
void TextWriterBase<CharType>::CalculateStringRect(Rectangle* pRect,
                                                   const CharType* pStr) const {
    CalculateStringRect(pRect, pStr, StrLen(pStr));
}

/**
 * Calculates the rectangle of a string.
 * @param pRect destination rectangle
 * @param pStr string
 * @param length string length
 */
template <typename CharType>
void TextWriterBase<CharType>::CalculateStringRect(Rectangle* pRect, const CharType* pStr,
                                                   int length) const {
    if (pStr == nullptr) {
        pRect->SetEdge(0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }

    TextWriterBase<CharType> myCopy(*this);
    const CharType* pStrEnd = pStr + length;
    m_pTagProcessor->BeginCalculateRectWhole(this, pStr, pStrEnd);
    myCopy.CalculateStringRectImpl(pRect, pStr, length);
    m_pTagProcessor->EndCalculateRectWhole(this, pStr, pStrEnd);
}

/**
 * Calculates the rectangle of a string from the origin.
 * @param pRect destination rectangle
 * @param pStr string
 * @param length string length
 */
template <typename CharType>
void TextWriterBase<CharType>::CalculateStringRectImpl(Rectangle* pRect, const CharType* pStr,
                                                       int length) {
    const CharType* pStrEnd = pStr + length;
    const CharType* pPos = pStr;
    int remain = length;

    pRect->SetEdge(0.0f, 0.0f, 0.0f, 0.0f);
    SetCursor(0.0f, 0.0f);

    do {
        Rectangle lineRect;
        lineRect.SetEdge(0.0f, 0.0f, 0.0f, 0.0f);
        CalculateLineRectImpl(&lineRect, &pPos, remain);
        remain = static_cast<int>(pStrEnd - pPos);
        pRect->left = std::min(pRect->left, lineRect.left);
        pRect->top = std::min(pRect->top, lineRect.top);
        pRect->right = std::max(pRect->right, lineRect.right);
        pRect->bottom = std::max(pRect->bottom, lineRect.bottom);
    } while (remain > 0);
}

/**
 * Finds where the first line is broken by the width limit.
 * @param pStr string
 * @param length string length
 * @return position of the break
 */
template <typename CharType>
const CharType* TextWriterBase<CharType>::FindPosOfWidthLimit(const CharType* pStr,
                                                              int length) const {
    Rectangle rect = {};
    TextWriterBase<CharType> myCopy(*this);
    myCopy.SetCursor(0.0f, 0.0f);
    m_pTagProcessor->BeginCalculateRectWhole(this, pStr, pStr + length);
    myCopy.CalculateLineRectImpl(&rect, &pStr, length);
    m_pTagProcessor->EndCalculateRectWhole(this, pStr, pStr + length);
    return pStr;
}

/**
 * Finds where the first line is broken by the width limit.
 * @param pStr string
 * @return position of the break
 */
template <typename CharType>
const CharType* TextWriterBase<CharType>::FindPosOfWidthLimit(const CharType* pStr) const {
    return FindPosOfWidthLimit(pStr, StrLen(pStr));
}

/**
 * Prints a formatted string.
 * @param pFormat format string
 * @return width of the printed text
 */
template <typename CharType>
float TextWriterBase<CharType>::Printf(const CharType* pFormat, ...) {
    std::va_list args;
    va_start(args, pFormat);
    float width = VPrintf(pFormat, args);
    va_end(args);
    return width;
}

/**
 * Prints a formatted string.
 * @param pFormat format string
 * @param args format arguments
 * @return width of the printed text
 */
template <typename CharType>
float TextWriterBase<CharType>::VPrintf(const CharType* pFormat, std::va_list args) {
    CharType buffer[FormatBufferSize];
    int length = std::min(VSNPrintf(buffer, FormatBufferSize, pFormat, args), FormatBufferSize - 1);
    return Print(buffer, length);
}

/**
 * Prints a string.
 * @param pStr string
 * @return width of the printed text
 */
template <typename CharType>
float TextWriterBase<CharType>::Print(const CharType* pStr) {
    return Print(pStr, StrLen(pStr));
}

/**
 * Prints a string.
 * @param pStr string
 * @param length string length
 * @return width of the printed text
 */
template <typename CharType>
float TextWriterBase<CharType>::Print(const CharType* pStr, int length) {
    return Print(pStr, length, 0, nullptr, nullptr);
}

/**
 * Prints a string with per-line offsets.
 * @param pStr string
 * @param length string length
 * @param lineOffsetCount number of line offsets
 * @param pLineOffset per-line horizontal offsets
 * @param pLineWidth per-line width limit adjustments
 * @return width of the printed text
 */
template <typename CharType>
float TextWriterBase<CharType>::Print(const CharType* pStr, int length, int lineOffsetCount,
                                      const float* pLineOffset, const float* pLineWidth) {
    TextWriterBase<CharType> myCopy(*this);
    float width = myCopy.PrintImpl(pStr, length, lineOffsetCount, pLineOffset, pLineWidth);
    SetCursor(myCopy.GetCursorX(), myCopy.GetCursorY());
    return width;
}

/**
 * Gets the length of a string.
 * @param pStr string
 * @return length
 */
template <typename CharType>
int TextWriterBase<CharType>::StrLen(const char* pStr) {
    return static_cast<int>(std::strlen(pStr));
}

/**
 * Gets the length of a string.
 * @param pStr string
 * @return length
 */
template <typename CharType>
int TextWriterBase<CharType>::StrLen(const uint16_t* pStr) {
    int length = 0;
    while (*pStr++ != 0) {
        length++;
    }
    return length;
}

/**
 * Formats a string.
 * @param pBuffer destination buffer
 * @param count size of the destination buffer
 * @param pFormat format string
 * @param args format arguments
 * @return formatted length
 */
template <typename CharType>
int TextWriterBase<CharType>::VSNPrintf(char* pBuffer, size_t count, const char* pFormat,
                                        std::va_list args) {
    return nn::util::VSNPrintf(pBuffer, count, pFormat, args);
}

/**
 * Formats a UTF-16 string.
 * @param pBuffer destination buffer
 * @param count size of the destination buffer
 * @param pFormat format string
 * @param args format arguments
 * @return formatted length
 */
template <typename CharType>
int TextWriterBase<CharType>::VSNPrintf(uint16_t* pBuffer, size_t count, const uint16_t* pFormat,
                                        std::va_list args) {
    return VSNW16Printf(pBuffer, count, count - 1, pFormat, args);
}

/**
 * Formats a UTF-16 string through UTF-8.
 * @param pBuffer destination buffer
 * @param count size of the destination buffer
 * @param size size passed to the formatter
 * @param pFormat format string
 * @param args format arguments
 * @return formatted length
 */
template <typename CharType>
int TextWriterBase<CharType>::VSNW16Printf(uint16_t* pBuffer, size_t count, size_t size,
                                           const uint16_t* pFormat, std::va_list args) {
    char buffer[0x400];
    char format[0x400];
    nn::util::ConvertStringUtf16NativeToUtf8(format, sizeof(format), pFormat, size);
    int result = std::vsnprintf(buffer, size, format, args);
    buffer[count - 1] = '\0';
    nn::util::ConvertStringUtf8ToUtf16Native(pBuffer, count, buffer, sizeof(buffer));
    return result;
}

/**
 * Applies the effects of the tags in a string to the writer.
 * @param pStr string
 * @param length string length
 */
template <typename CharType>
void TextWriterBase<CharType>::UpdateTextWriterWithTags(const CharType* pStr, int length) {
    const CharType* pStrEnd = pStr + length;
    const CharType* pPos = pStr;
    int remain = length;
    SetCursor(0.0f, 0.0f);
    do {
        Rectangle rect = {};
        CalculateLineRectImpl(&rect, &pPos, remain);
        remain = static_cast<int>(pStrEnd - pPos);
    } while (remain > 0);
}

/**
 * Calculates the width of the first line of a string.
 * @param pStr string
 * @param length string length
 * @return width
 */
template <typename CharType>
float TextWriterBase<CharType>::CalculateLineWidth(const CharType* pStr, int length) {
    Rectangle rect = {};
    const CharType* pPos = pStr;
    TextWriterBase<CharType> myCopy(*this);
    myCopy.SetCursor(0.0f, 0.0f);
    myCopy.CalculateLineRectImpl(&rect, &pPos, length);
    return rect.GetWidth();
}

/**
 * Calculates the height of the first line of a string.
 * @param pStr string
 * @param length string length
 * @return height
 */
template <typename CharType>
float TextWriterBase<CharType>::CalculateLineHeight(const CharType* pStr, int length) {
    Rectangle rect = {};
    const CharType* pPos = pStr;
    TextWriterBase<CharType> myCopy(*this);
    myCopy.SetCursor(0.0f, 0.0f);
    myCopy.CalculateLineRectImpl(&rect, &pPos, length);
    return rect.GetHeight();
}

/**
 * Gets the offset of a line.
 * @param lineOffsetCount number of line offsets
 * @param pLineOffset per-line horizontal offsets
 * @param line line index
 * @return line offset
 */
template <typename CharType>
float TextWriterBase<CharType>::GetLineOffset(int lineOffsetCount, const float* pLineOffset,
                                              int line) const {
    if (line < lineOffsetCount) {
        return pLineOffset[line];
    }
    return 0.0f;
}

/**
 * Gets the width adjustment of a line.
 * @param lineOffsetCount number of line offsets
 * @param pLineWidth per-line width limit adjustments
 * @param line line index
 * @return line width adjustment
 */
template <typename CharType>
float TextWriterBase<CharType>::GetLineWidth(int lineOffsetCount, const float* pLineWidth,
                                             int line) const {
    if (line < lineOffsetCount) {
        return pLineWidth[line];
    }
    return 0.0f;
}

/**
 * Moves a vertical origin by the extra height of the first line.
 * @param y vertical origin
 * @param pStr string
 * @param length string length
 * @return moved origin
 */
template <typename CharType>
float TextWriterBase<CharType>::MoveOriginAsFirstLineScale(float y, const CharType* pStr,
                                                           int length) {
    float lineHeight = CalculateLineHeight(pStr, length);
    return y + (lineHeight - m_pTagProcessor->GetLineHeight(this));
}

namespace {

const uint32_t PrintContextFlag_NoCharSpace = 1 << 0;

/**
 * Checks whether a character is not an invisible formatting character.
 * @param code character code
 * @return whether it is printable
 */
bool IsPrintableChar(uint32_t code) {
    if ((code & 0xffffff00) == 0x2000) {
        switch (code) {
        case 0x200b:
        case 0x200e:
        case 0x200f:
        case 0x202a:
        case 0x202b:
        case 0x202c:
        case 0x202d:
        case 0x202e:
        case 0x2060:
            return false;
        default:
            break;
        }
        if (code >= 0x2066 && code <= 0x2069) {
            return false;
        }
    }
    return true;
}

}  // namespace

template <typename CharType>
bool TextWriterBase<CharType>::CalculateLineRectImpl(Rectangle* pRect, const CharType** ppStr,
                                                     int length) {
    const bool isIgnoringKerning = IsWidthFixed() ? m_IsIgnoringKerningWithFixedWidth : false;
    const CharType* pStr = *ppStr;
    const CharType* pStrEnd = pStr + length;
    const float widthLimit = m_WidthLimit;
    PrintContext<CharType> context(this, pStr, pStrEnd, 0.0f, 0.0f, GetScaleH(), GetScaleV());
    float x = 0.0f;
    bool isCharSpace = false;
    bool isOverLimit = false;
    const CharType* pPrevStreamPos = nullptr;
    CharStrmReader reader = GetFont()->GetCharStrmReader(CharType(0));

    const float fontHeight = context.writer->GetFontHeight();
    pRect->left = 0.0f;
    pRect->right = 0.0f;
    pRect->top = std::min(0.0f, fontHeight);
    pRect->bottom = std::max(0.0f, fontHeight);

    reader.Set(pStr);
    m_pTagProcessor->BeginCalculateRect(&context);

    uint32_t code = reader.Next();
    for (;;) {
        if (static_cast<const CharType*>(reader.GetCurrentPos()) > pStrEnd) {
            break;
        }
        if (code < ' ') {
            if (code == '\n' && !m_IsLinefeedKerningEnabled) {
                if (!isIgnoringKerning) {
                    x += GetFont()->GetKerning(context.prevCode, 0) * GetScaleH();
                }
                context.prevCode = 0;
            }

            Rectangle rect = {x, 0.0f, 0.0f, 0.0f};
            context.str = static_cast<const CharType*>(reader.GetCurrentPos());
            context.flags = isCharSpace ? 0 : PrintContextFlag_NoCharSpace;
            SetCursorX(x);

            if (pPrevStreamPos != nullptr && widthLimit < FLT_MAX && code != '\n') {
                PrintContext<CharType> context2 = context;
                TextWriterBase<CharType> myCopy(*this);
                Rectangle rect2 = {};
                context2.writer = &myCopy;
                m_pTagProcessor->CalculateRect(&rect2, &context2, code);
                if (rect2.GetWidth() > 0.0f &&
                    myCopy.GetCursorX() - context.xOrigin > m_WidthLimit) {
                    isOverLimit = true;
                    code = '\n';
                    reader.Set(pPrevStreamPos);
                    continue;
                }
            }

            typename TagProcessorBase<CharType>::Operation operation =
                m_pTagProcessor->CalculateRect(&rect, &context, code);

            if (code == '\n' && m_IsLinefeedByCharacterHeightEnabled) {
                if (GetCursorY() < pRect->bottom) {
                    SetCursorY(pRect->bottom);
                }
            }

            reader.Set(context.str);
            pRect->left = std::min(pRect->left, rect.left);
            pRect->top = std::min(pRect->top, rect.top);
            pRect->right = std::max(pRect->right, rect.right);
            pRect->bottom = std::max(pRect->bottom, rect.bottom);
            x = GetCursorX();

            if (operation == TagProcessorBase<CharType>::Operation_NextLine) {
                break;
            }
            if (operation == TagProcessorBase<CharType>::Operation_EndDraw) {
                *ppStr += length;
                return false;
            }
            if (operation == TagProcessorBase<CharType>::Operation_NoCharSpace) {
                isCharSpace = false;
            } else if (operation == TagProcessorBase<CharType>::Operation_CharSpace) {
                isCharSpace = true;
            }
        } else if (IsPrintableChar(code)) {
            float left = x;
            if (isCharSpace) {
                left += m_CharSpace;
            }

            float right;
            if (IsWidthFixed()) {
                right = left + GetFixedWidth();
            } else {
                right = left + GetFont()->GetCharWidth(code) * GetScaleH();
            }

            float kerning = 0.0f;
            if (!isIgnoringKerning) {
                right += GetFont()->GetKerning(context.prevCode, code) * GetScaleH();
                kerning = GetFont()->GetKerning(code, 0) * GetScaleH();
            }
            right += kerning;

            if (widthLimit < FLT_MAX && pPrevStreamPos != nullptr &&
                right + m_WidthLimitOffset > m_WidthLimit) {
                isOverLimit = true;
                code = '\n';
                reader.Set(pPrevStreamPos);
                continue;
            }

            x = right;
            pRect->left = std::min(pRect->left, x);
            pRect->right = std::max(pRect->right, x);

            if (m_IsLinefeedByCharacterHeightEnabled) {
                const float y = GetCursorY();
                pRect->top = std::min(pRect->top, y);
                pRect->bottom = std::max(pRect->bottom, y + m_pTagProcessor->GetLineHeight(this));
            }

            x = right - kerning;
            context.prevCode = code;
            isCharSpace = true;
        }

        if (widthLimit < FLT_MAX) {
            pPrevStreamPos = static_cast<const CharType*>(reader.GetCurrentPos());
        }
        code = reader.Next();
    }

    if (!isIgnoringKerning) {
        x += GetFont()->GetKerning(context.prevCode, 0) * GetScaleH();
    }
    pRect->left = std::min(pRect->left, x);
    pRect->right = std::max(pRect->right, x);
    *ppStr = static_cast<const CharType*>(reader.GetCurrentPos());
    m_pTagProcessor->EndCalculateRect(&context);
    return isOverLimit;
}

template <typename CharType>
float TextWriterBase<CharType>::AdjustCursor(float* pXOrigin, float* pYOrigin,
                                             const CharType* pStr, int length) {
    float textWidth = 0.0f;
    float textHeight = 0.0f;

    const uint32_t mask = HorizontalAlign_Mask | HorizontalOrigin_Mask | VerticalOrigin_Mask;
    if (!IsDrawFlagSet(mask, HorizontalAlign_Left | HorizontalOrigin_Left | VerticalOrigin_Top) &&
        !IsDrawFlagSet(mask, HorizontalAlign_Left | HorizontalOrigin_Left |
                                 VerticalOrigin_Baseline)) {
        Rectangle rect;
        if (pStr == nullptr) {
            rect.SetEdge(0.0f, 0.0f, 0.0f, 0.0f);
        } else {
            TextWriterBase<CharType> myCopy(*this);
            myCopy.CalculateStringRectImpl(&rect, pStr, length);
        }
        textWidth = rect.GetWidth();
        textHeight = rect.GetHeight();
    }

    switch (m_DrawFlag & HorizontalOrigin_Mask) {
    case HorizontalOrigin_Center: {
        float offset = textWidth * 0.5f;
        if (m_IsCenterCeilingEnabled) {
            offset = std::ceil(offset);
        }
        *pXOrigin -= offset;
        break;
    }
    case HorizontalOrigin_Right:
        *pXOrigin -= textWidth;
        break;
    default:
        break;
    }

    switch (m_DrawFlag & VerticalOrigin_Mask) {
    case VerticalOrigin_Middle: {
        float offset = textHeight * 0.5f;
        if (m_IsCenterCeilingEnabled) {
            offset = std::ceil(offset);
        }
        *pYOrigin -= offset;
        break;
    }
    case VerticalOrigin_Bottom:
        *pYOrigin -= textHeight;
        break;
    default:
        break;
    }

    if (m_IsLinefeedByCharacterHeightEnabled) {
        *pYOrigin = MoveOriginAsFirstLineScale(*pYOrigin, pStr, length);
    }

    switch (m_DrawFlag & HorizontalAlign_Mask) {
    case HorizontalAlign_Center: {
        const float lineWidth = CalculateLineWidth(pStr, length);
        const float offset =
            m_IsCenterCeilingEnabled ? std::ceil(textWidth * 0.5f) : textWidth * 0.5f;
        const float lineOffset =
            m_IsCenterCeilingEnabled ? std::ceil(lineWidth * 0.5f) : lineWidth * 0.5f;
        SetCursorX(*pXOrigin + (offset - lineOffset));
        break;
    }
    case HorizontalAlign_Right: {
        const float lineWidth = CalculateLineWidth(pStr, length);
        SetCursorX(textWidth - lineWidth + *pXOrigin);
        break;
    }
    default:
        SetCursorX(*pXOrigin);
        break;
    }

    float y = *pYOrigin;
    if (!IsDrawFlagSet(VerticalOrigin_Mask, VerticalOrigin_Baseline)) {
        y += GetFontAscent();
    }
    SetCursorY(y);
    return textWidth;
}

template <typename CharType>
float TextWriterBase<CharType>::PrintImpl(const CharType* pStr, int length, int lineOffsetCount,
                                          const float* pLineOffset, const float* pLineWidth) {
    const bool isIgnoringKerning = IsWidthFixed() ? m_IsIgnoringKerningWithFixedWidth : false;
    float xOrigin = GetCursorX();
    const float cursorY = GetCursorY();
    float yOrigin = cursorY;
    const float widthLimit = m_WidthLimit;
    const CharType* pStrEnd = pStr + length;

    m_pTagProcessor->BeginPrintWhole(this, pStr, pStrEnd);
    float textWidth = AdjustCursor(&xOrigin, &yOrigin, pStr, length);
    const float adjustedCursorY = GetCursorY();

    PrintContext<CharType> context(this, pStr, pStrEnd, xOrigin, yOrigin, GetScaleH(),
                                   GetScaleV());
    if (lineOffsetCount != 0) {
        SetCursorX(GetLineOffset(lineOffsetCount, pLineOffset, 0));
    }

    CharStrmReader reader = GetFont()->GetCharStrmReader(CharType(0));
    reader.Set(pStr);
    m_pTagProcessor->BeginPrint(&context);

    int lineNo = 0;
    bool isCharSpace = false;
    const CharType* pPrevStreamPos = pStr;
    const CharType* pLineHead = pStr;
    float x = 0.0f;

    uint32_t code = reader.Next();
    while (static_cast<const CharType*>(reader.GetCurrentPos()) - pStr <= length) {
        const bool isLineHead = lineNo >= lineOffsetCount && pPrevStreamPos == pLineHead;
        if (code < ' ') {
            context.str = static_cast<const CharType*>(reader.GetCurrentPos());
            context.flags = isCharSpace ? 0 : PrintContextFlag_NoCharSpace;

            if (!(!(widthLimit < FLT_MAX) || code == '\n' || isLineHead)) {
                PrintContext<CharType> context2 = context;
                TextWriterBase<CharType> myCopy(*this);
                Rectangle rect = {};
                context2.writer = &myCopy;
                m_pTagProcessor->CalculateRect(&rect, &context2, code);
                if (rect.GetWidth() > 0.0f &&
                    myCopy.GetCursorX() - context.xOrigin >
                        m_WidthLimit + GetLineWidth(lineOffsetCount, pLineWidth, lineNo)) {
                    code = '\n';
                    context.prevCode = 0;
                    reader.Set(pPrevStreamPos);
                    continue;
                }
            }

            typename TagProcessorBase<CharType>::Operation operation =
                m_pTagProcessor->Process(code, &context);
            x = GetCursorX() - xOrigin;

            if (code == '\n' && !m_IsLinefeedKerningEnabled) {
                context.prevCode = 0;
                if (m_IsLinefeedByCharacterHeightEnabled) {
                    const CharType* pPos = static_cast<const CharType*>(reader.GetCurrentPos());
                    SetCursorY(MoveOriginAsFirstLineScale(
                        GetCursorY(), pPos, length - static_cast<int>(pPos - pStr)));
                }
            }

            if (operation == TagProcessorBase<CharType>::Operation_NextLine) {
                lineNo++;
                const float lineOffset = GetLineOffset(lineOffsetCount, pLineOffset, lineNo);
                switch (m_DrawFlag & HorizontalAlign_Mask) {
                case HorizontalAlign_Center: {
                    const float lineWidth = CalculateLineWidth(
                        context.str, length - static_cast<int>(context.str - pStr));
                    float offset = textWidth * 0.5f;
                    if (m_IsCenterCeilingEnabled) {
                        offset = std::ceil(offset);
                    }
                    float lineOffsetCenter = lineWidth * 0.5f;
                    if (m_IsCenterCeilingEnabled) {
                        lineOffsetCenter = std::ceil(lineOffsetCenter);
                    }
                    SetCursorX(lineOffset + (context.xOrigin + (offset - lineOffsetCenter)));
                    break;
                }
                case HorizontalAlign_Right: {
                    const float lineWidth = CalculateLineWidth(
                        context.str, length - static_cast<int>(context.str - pStr));
                    SetCursorX(lineOffset + (context.xOrigin + (textWidth - lineWidth)));
                    break;
                }
                default: {
                    const float width = GetCursorX() - context.xOrigin;
                    textWidth = std::max(textWidth, width);
                    SetCursorX(lineOffset + context.xOrigin);
                    break;
                }
                }

                if (widthLimit < FLT_MAX) {
                    pLineHead = static_cast<const CharType*>(reader.GetCurrentPos());
                }
                isCharSpace = false;
            } else if (operation == TagProcessorBase<CharType>::Operation_EndDraw) {
                break;
            } else if (operation == TagProcessorBase<CharType>::Operation_NoCharSpace) {
                isCharSpace = false;
            } else if (operation == TagProcessorBase<CharType>::Operation_CharSpace) {
                isCharSpace = true;
            }

            reader.Set(context.str);
        } else if (IsPrintableChar(code)) {
            const float y = GetCursorY();

            float kerning = 0.0f;
            if (!isIgnoringKerning) {
                kerning = GetFont()->GetKerning(context.prevCode, code) * GetScaleH();
            }

            Glyph glyph;
            GetFont()->GetGlyph(&glyph, code);
            if (glyph.height > glyph.texHeight) {
                code = reader.Next();
                continue;
            }

            float right = x;
            if (isCharSpace) {
                right += m_CharSpace;
            }
            if (IsWidthFixed()) {
                right += GetFixedWidth();
            } else {
                right += GetScaleH() * glyph.widths.charWidth;
            }
            right = kerning + right;

            float nextKerning = 0.0f;
            if (!isIgnoringKerning) {
                nextKerning = GetFont()->GetKerning(code, 0) * GetScaleH();
            }
            right += nextKerning;

            if (!(!(widthLimit < FLT_MAX) || isLineHead) &&
                right > m_WidthLimit + GetLineWidth(lineOffsetCount, pLineWidth, lineNo)) {
                context.prevCode = 0;
                code = '\n';
                reader.Set(pPrevStreamPos);
                continue;
            }

            if (isCharSpace) {
                MoveCursorX(m_CharSpace);
            }
            x = right - nextKerning;
            MoveCursorX(kerning);

            const int baseline = GetFont()->GetBaselinePos() + glyph.baselineDifference;
            MoveCursorY(-baseline * GetScaleV());
            PrintGlyph(glyph);
            SetCursorY(y);

            context.prevCode = code;
            isCharSpace = true;
        }

        if (widthLimit < FLT_MAX) {
            pPrevStreamPos = static_cast<const CharType*>(reader.GetCurrentPos());
        }
        code = reader.Next();
    }

    if (!isIgnoringKerning) {
        MoveCursorX(GetFont()->GetKerning(context.prevCode, 0) * GetScaleH());
    }
    textWidth = std::max(textWidth, GetCursorX() - context.xOrigin);

    float y = cursorY;
    if (!IsDrawFlagSet(VerticalOrigin_Mask, VerticalOrigin_Middle) &&
        !IsDrawFlagSet(VerticalOrigin_Mask, VerticalOrigin_Bottom)) {
        y = cursorY - adjustedCursorY + GetCursorY();
    }
    SetCursorY(y);

    m_pTagProcessor->EndPrint(&context);
    m_pTagProcessor->EndPrintWhole(this, pStr, pStrEnd);
    return textWidth;
}

template class TextWriterBase<char>;
template class TextWriterBase<uint16_t>;

}  // namespace font
}  // namespace nn
