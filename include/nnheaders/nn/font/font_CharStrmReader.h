#pragma once

#include <nn/types.h>

namespace nn {
namespace font {

class CharStrmReader {
public:
    typedef uint32_t (CharStrmReader::*ReadNextCharFunc)();

    explicit CharStrmReader(ReadNextCharFunc func) : m_pCharStrm(nullptr), m_ReadFunc(func) {}

    uint32_t ReadNextCharUtf8();
    uint32_t ReadNextCharUtf16();
    uint32_t ReadNextCharCp1252();
    uint32_t ReadNextCharSjis();

    const void* GetCurrentPos() const { return m_pCharStrm; }

    void Set(const char* pStream) { m_pCharStrm = pStream; }
    void Set(const uint16_t* pStream) { m_pCharStrm = pStream; }

    uint32_t Next() { return (this->*m_ReadFunc)(); }

private:
    template <typename CharType>
    const CharType* GetChar(int offset = 0) const {
        return static_cast<const CharType*>(m_pCharStrm) + offset;
    }

    template <typename CharType>
    void StepStrm(int step = 1) {
        m_pCharStrm = static_cast<const CharType*>(m_pCharStrm) + step;
    }

    const void* m_pCharStrm;
    ReadNextCharFunc m_ReadFunc;
};

}  // namespace font
}  // namespace nn
