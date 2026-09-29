#pragma once

#include <nn/font/font_TextWriterBase.h>
#include <nn/types.h>

namespace nn {
namespace font {

struct WordWrapConfig {
    int maxLineCount;
    bool isLeadingSpaceRemoved;
    bool isAlternativePairTableUsed;
    bool isCyrillicBreakEnabled;
};

template <typename CharType>
class WordWrapCallbackBase {
public:
    virtual const CharType* GetLineBreakLimit(const CharType* pStart, const CharType* pEnd) = 0;

    /**
     * Gets the size of a tag at a position.
     * @param pSkipCount number of following characters that must not be broken
     * @param pPos position of the tag
     * @param pEnd end of the text
     * @return the size of the tag in bytes, or 0 if there is no tag
     */
    virtual size_t GetTagSize(size_t* pSkipCount, const CharType* pPos, const CharType* pEnd) {
        *pSkipCount = 0;
        return 0;
    }
};

template <typename CharType>
class DefaultWordWrapCallbackBase : public WordWrapCallbackBase<CharType> {
public:
    explicit DefaultWordWrapCallbackBase(const TextWriterBase<CharType>* pWriter);
    virtual ~DefaultWordWrapCallbackBase();

    const CharType* GetLineBreakLimit(const CharType* pStart, const CharType* pEnd) override;

private:
    TextWriterBase<CharType> m_Writer;
    const CharType* m_pUpdatedPos;
};

class WordWrapping {
public:
    static const uint16_t* FindLineBreak(const uint16_t* pStart, const uint16_t* pEnd,
                                         WordWrapCallbackBase<uint16_t>& rCallback,
                                         const WordWrapConfig& rConfig);
    static const char* FindLineBreakUtf8(const char* pStart, const char* pEnd,
                                         WordWrapCallbackBase<char>& rCallback,
                                         const WordWrapConfig& rConfig);
    static bool CalculateWordWrapping(uint32_t* pOutLength, uint16_t* pDst, uint32_t dstSize,
                                      const uint16_t* pSrc, uint32_t srcLength,
                                      WordWrapCallbackBase<uint16_t>& rCallback,
                                      const WordWrapConfig& rConfig);
    static bool CalculateWordWrappingUtf8(uint32_t* pOutLength, char* pDst, uint32_t dstSize,
                                          const char* pSrc, uint32_t srcLength,
                                          WordWrapCallbackBase<char>& rCallback,
                                          const WordWrapConfig& rConfig);
};

}  // namespace font
}  // namespace nn
