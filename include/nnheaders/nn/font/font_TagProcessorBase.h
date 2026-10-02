#pragma once

#include <nn/font/font_Util.h>
#include <nn/types.h>

namespace nn {
namespace font {

template <typename CharType>
class TextWriterBase;

struct Rectangle {
    float left;
    float top;
    float right;
    float bottom;

    ~Rectangle() {}

    float GetWidth() const { return right - left; }
    float GetHeight() const { return bottom - top; }

    void SetEdge(float l, float r, float t, float b) {
        left = l;
        right = r;
        top = t;
        bottom = b;
    }

    void Normalize() {
        const float l = left;
        const float t = top;
        const float r = right;
        const float b = bottom;
        left = r - l >= 0 ? l : r;
        right = r - l >= 0 ? r : l;
        top = b - t >= 0 ? t : b;
        bottom = b - t >= 0 ? b : t;
    }
};

enum PrintFlag {
    PrintFlag_Ruby = 1 << 0,
};

template <typename CharType>
struct PrintContext {
    PrintContext(TextWriterBase<CharType>* pWriter, const CharType* pStr, const CharType* pStrEnd,
                 float xOrigin_, float yOrigin_, float hScale_, float vScale_)
        : writer(pWriter), str(pStr), strEnd(pStrEnd), xOrigin(xOrigin_), yOrigin(yOrigin_),
          hScale(hScale_), vScale(vScale_), flags(0), prevCode(0) {}

    TextWriterBase<CharType>* writer;
    const CharType* str;
    const CharType* strEnd;
    float xOrigin;
    float yOrigin;
    float hScale;
    float vScale;
    uint32_t flags;
    uint32_t prevCode;
};

template <typename CharType>
class TagProcessorBase {
public:
    NN_RUNTIME_TYPEINFO_BASE();

    enum Operation {
        Operation_Default,
        Operation_NoCharSpace,
        Operation_CharSpace,
        Operation_NextLine,
        Operation_EndDraw,
    };

    TagProcessorBase();
    virtual ~TagProcessorBase();

    virtual Operation Process(uint32_t code, PrintContext<CharType>* pContext);
    virtual Operation CalculateRect(Rectangle* pRect, PrintContext<CharType>* pContext,
                                    uint32_t code);
    virtual float GetLineHeight(const TextWriterBase<CharType>* pWriter) const;
    virtual void BeginPrint(PrintContext<CharType>* pContext);
    virtual void EndPrint(PrintContext<CharType>* pContext);
    virtual void BeginCalculateRect(PrintContext<CharType>* pContext);
    virtual void EndCalculateRect(PrintContext<CharType>* pContext);
    virtual void BeginPrintWhole(const TextWriterBase<CharType>* pWriter, const CharType* pStr,
                                 const CharType* pStrEnd);
    virtual void EndPrintWhole(const TextWriterBase<CharType>* pWriter, const CharType* pStr,
                               const CharType* pStrEnd);
    virtual void BeginCalculateRectWhole(const TextWriterBase<CharType>* pWriter,
                                         const CharType* pStr, const CharType* pStrEnd);
    virtual void EndCalculateRectWhole(const TextWriterBase<CharType>* pWriter,
                                       const CharType* pStr, const CharType* pStrEnd);
    virtual const CharType* AcquireNextPrintableChar(bool* pIsPrintable, const CharType* pStr);

protected:
    void ProcessLinefeed(PrintContext<CharType>* pContext) const;
    void ProcessTab(PrintContext<CharType>* pContext) const;
};

namespace detail {

const char* AcquireNextPrinableCharImpl(bool* pIsPrintable, const char* pStr);
const uint16_t* AcquireNextPrinableCharImpl(bool* pIsPrintable, const uint16_t* pStr);

}  // namespace detail

}  // namespace font
}  // namespace nn
