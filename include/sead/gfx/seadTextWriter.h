#pragma once

#include <cstdarg>
#include <limits>
#include <gfx/seadColor.h>
#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace sead
{
class Viewport;
class Camera;
class Projection;
class DrawContext;
class FontBase;
class DebugFontMgrNvn;

class TextWriter
{
public:
    explicit TextWriter(DrawContext* pDrawContext);
    TextWriter(DrawContext* pDrawContext, const Viewport* pViewport);
    virtual ~TextWriter() {}

    static FontBase* getDefaultFont();
    static void setDefaultFont(FontBase* pFont);
    static void setupGraphics(DrawContext* pDrawContext);
    void getCursorFromTopLeft(Vector2f* pPos) const;
    void setCursorFromTopLeft(const Vector2f& rPos);
    void setScaleFromFontSize(const Vector2f& rFontSize);
    void setScaleFromFontHeight(f32 fontHeight);
    void setProjectionAndCamera(const Projection* pProjection, const Camera* pCamera);
    void setLineSpaceFromLineHeight(f32 lineHeight);
    void setFormatBuffer(char16_t* pBuffer, s32 size);
    void setDrawContext(DrawContext* pDrawContext);
    void beginDraw();
    void endDraw();
    void printf(const char16_t* pFormat, ...);
    void printfWithCalcRect(BoundBox2f* pRect, const char16_t* pFormat, ...);
    void printf(const char* pFormat, ...);
    void printfWithCalcRect(BoundBox2f* pRect, const char* pFormat, ...);
    void calcFormatStringRect(BoundBox2f* pRect, const char16_t* pFormat, ...);
    void calcFormatStringRect(BoundBox2f* pRect, const char* pFormat, ...);

    void setCursor(const Vector2f& rCursor) { mCursor = rCursor; }
    const Vector2f& getCursor() const { return mCursor; }
    void setScale(const Vector2f& rScale) { mScale = rScale; }
    const Vector2f& getScale() const { return mScale; }
    void setColor(const Color4f& rColor) { mColor = rColor; }
    const Color4f& getColor() const { return mColor; }
    void setLineSpace(f32 lineSpace) { mLineSpace = lineSpace; }
    void setModelMatrix(const Matrix34f* pModelMatrix) { mModelMatrix = pModelMatrix; }
    void setWrapWidth(f32 wrapWidth) { mWrapWidth = wrapWidth; }
    void setFont(FontBase* pFont) { mFont = pFont; }
    FontBase* getFont() const { return mFont; }
    void setViewport(const Viewport* pViewport) { mViewport = pViewport; }

private:
    enum class LineEnd
    {
        cEnd,
        cNewLine,
        cWrap,
        cMaxDraw,
    };

    void vprintfImpl_(const char16_t* pFormat, std::va_list args, bool isDraw, BoundBox2f* pRect);
    void vprintfImpl_(const char* pFormat, std::va_list args, bool isDraw, BoundBox2f* pRect);
    void printImpl_(const char16_t* pStr, s32 length, bool isDraw, BoundBox2f* pRect,
                    const Projection* pProjection, const Camera* pCamera);
    void printImpl_(const char16_t* pStr, s32 length, bool isDraw, BoundBox2f* pRect);
    void printImpl_(const char* pStr, s32 length, bool isDraw, BoundBox2f* pRect);

    static FontBase* sDefaultFont;

    const Viewport* mViewport;
    const Projection* mProjection = nullptr;
    const Camera* mCamera = nullptr;
    FontBase* mFont = getDefaultFont();
    Vector2f mCursor{0.0f, 0.0f};
    Vector2f mScale{1.0f, 1.0f};
    Color4f mColor{1.0f, 1.0f, 1.0f, 1.0f};
    f32 _48 = 0.0f;
    f32 mLineSpace = 0.0f;
    const Matrix34f* mModelMatrix = nullptr;
    f32 mWrapWidth = std::numeric_limits<f32>::infinity();
    char16_t* mFormatBuffer = nullptr;
    s32 mFormatBufferSize = 0;
    bool mIsAutoBeginEnd = true;
    DrawContext* mDrawContext;
};
static_assert(sizeof(TextWriter) == 0x78);

}  // namespace sead
