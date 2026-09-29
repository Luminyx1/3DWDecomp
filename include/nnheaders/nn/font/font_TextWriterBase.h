#pragma once

#include <cfloat>
#include <cstdarg>
#include <nn/font/font_CharWriter.h>
#include <nn/font/font_ExtendedTagProcessorBase.h>
#include <nn/font/font_TagProcessorBase.h>
#include <nn/types.h>

namespace nn {
namespace font {

template <typename CharType>
class TextWriterBase : public CharWriter {
public:
    enum DrawFlag {
        HorizontalAlign_Left = 0x0,
        HorizontalAlign_Center = 0x1,
        HorizontalAlign_Right = 0x2,
        HorizontalAlign_Mask = 0x3,
        HorizontalOrigin_Left = 0x0,
        HorizontalOrigin_Center = 0x10,
        HorizontalOrigin_Right = 0x20,
        HorizontalOrigin_Mask = 0x30,
        VerticalOrigin_Top = 0x0,
        VerticalOrigin_Middle = 0x100,
        VerticalOrigin_Bottom = 0x200,
        VerticalOrigin_Baseline = 0x300,
        VerticalOrigin_Mask = 0x300,
    };

    static const int DefaultTabWidth = 4;
    static const int FormatBufferSize = 0x200;

    TextWriterBase();
    ~TextWriterBase();

    void SetLineSpace(float space) { m_LineSpace = space; }
    float GetLineSpace() const { return m_LineSpace; }
    void SetLineHeight(float height);
    float GetLineHeight() const;
    void SetCharSpace(float space) { m_CharSpace = space; }
    float GetCharSpace() const { return m_CharSpace; }
    void SetWidthLimitOffset(float offset) { m_WidthLimitOffset = offset; }
    float GetWidthLimitOffset() const { return m_WidthLimitOffset; }
    void SetTabWidth(int tabWidth) { m_TabWidth = tabWidth; }
    int GetTabWidth() const { return m_TabWidth; }
    void SetWidthLimit(float limit) { m_WidthLimit = limit; }
    float GetWidthLimit() const { return m_WidthLimit; }
    void ResetWidthLimit() { m_WidthLimit = FLT_MAX; }
    void SetDrawFlag(uint32_t flags) { m_DrawFlag = flags; }
    uint32_t GetDrawFlag() const { return m_DrawFlag; }
    void SetTagProcessor(TagProcessorBase<CharType>* pTagProcessor) {
        m_pTagProcessor = pTagProcessor;
    }
    TagProcessorBase<CharType>& GetTagProcessor() const { return *m_pTagProcessor; }
    void ResetTagProcessor() { m_pTagProcessor = &g_DefaultTagProcessor; }
    static ExtendedTagProcessorBase<CharType>* GetExtendedTagProcessor() {
        return &g_ExtendedTagProcessor;
    }

    float CalculateFormatStringWidth(const CharType* pFormat, ...) const;
    float CalculateFormatStringHeight(const CharType* pFormat, ...) const;
    void CalculateFormatStringRect(Rectangle* pRect, const CharType* pFormat, ...) const;
    void CalculateVStringRect(Rectangle* pRect, const CharType* pFormat, std::va_list args) const;
    float CalculateStringWidth(const CharType* pStr) const;
    float CalculateStringWidth(const CharType* pStr, int length) const;
    float CalculateStringHeight(const CharType* pStr) const;
    float CalculateStringHeight(const CharType* pStr, int length) const;
    void CalculateStringRect(Rectangle* pRect, const CharType* pStr) const;
    void CalculateStringRect(Rectangle* pRect, const CharType* pStr, int length) const;

    const CharType* FindPosOfWidthLimit(const CharType* pStr) const;
    const CharType* FindPosOfWidthLimit(const CharType* pStr, int length) const;

    float Printf(const CharType* pFormat, ...);
    float VPrintf(const CharType* pFormat, std::va_list args);
    float Print(const CharType* pStr);
    float Print(const CharType* pStr, int length);
    float Print(const CharType* pStr, int length, int lineOffsetCount, const float* pLineOffset,
                const float* pLineWidth);

    static int StrLen(const char* pStr);
    static int StrLen(const uint16_t* pStr);
    static int VSNPrintf(char* pBuffer, size_t count, const char* pFormat, std::va_list args);
    static int VSNPrintf(uint16_t* pBuffer, size_t count, const uint16_t* pFormat,
                         std::va_list args);
    static int VSNW16Printf(uint16_t* pBuffer, size_t count, size_t size, const uint16_t* pFormat,
                            std::va_list args);

    void SetCenterCeilingEnabled(bool isEnabled) { m_IsCenterCeilingEnabled = isEnabled; }
    void SetLinefeedKerningEnabled(bool isEnabled) { m_IsLinefeedKerningEnabled = isEnabled; }
    void SetLinefeedByCharacterHeightEnabled(bool isEnabled) {
        m_IsLinefeedByCharacterHeightEnabled = isEnabled;
    }
    bool GetLinefeedByCharacterHeightEnabled() const {
        return m_IsLinefeedByCharacterHeightEnabled;
    }
    void SetIgnoringKerningWithFixedWidth(bool isIgnoring) {
        m_IsIgnoringKerningWithFixedWidth = isIgnoring;
    }
    bool GetIgnoringKerningWithFixedWidth() const { return m_IsIgnoringKerningWithFixedWidth; }

    void UpdateTextWriterWithTags(const CharType* pStr, int length);
    float CalculateLineWidth(const CharType* pStr, int length);
    float CalculateLineHeight(const CharType* pStr, int length);
    float MoveOriginAsFirstLineScale(float y, const CharType* pStr, int length);

    bool IsDrawFlagSet(uint32_t mask, uint32_t flag) const { return (m_DrawFlag & mask) == flag; }

protected:
    float PrintImpl(const CharType* pStr, int length, int lineOffsetCount,
                    const float* pLineOffset, const float* pLineWidth);
    float AdjustCursor(float* pXOrigin, float* pYOrigin, const CharType* pStr, int length);
    void CalculateStringRectImpl(Rectangle* pRect, const CharType* pStr, int length);
    bool CalculateLineRectImpl(Rectangle* pRect, const CharType** ppStr, int length);
    float GetLineOffset(int lineOffsetCount, const float* pLineOffset, int line) const;
    float GetLineWidth(int lineOffsetCount, const float* pLineWidth, int line) const;

    float m_WidthLimit;
    float m_CharSpace;
    float m_LineSpace;
    float m_WidthLimitOffset;
    int m_TabWidth;
    uint32_t m_DrawFlag;
    TagProcessorBase<CharType>* m_pTagProcessor;
    bool m_IsCenterCeilingEnabled;
    bool m_IsLinefeedKerningEnabled;
    bool m_IsLinefeedByCharacterHeightEnabled;
    bool m_IsIgnoringKerningWithFixedWidth;

    static TagProcessorBase<CharType> g_DefaultTagProcessor;
    static ExtendedTagProcessorBase<CharType> g_ExtendedTagProcessor;
};

}  // namespace font
}  // namespace nn
