#include "gfx/seadTextWriter.h"

#include "devenv/seadFontMgr.h"
#include "gfx/nvn/seadDebugFontMgrNvn.h"
#include "gfx/seadCamera.h"
#include "gfx/seadGraphicsContext.h"
#include "gfx/seadProjection.h"
#include "gfx/seadViewport.h"
#include "math/seadMatrixCalcCommon.h"
#include "prim/seadSafeString.h"
#include "prim/seadStringUtil.h"

namespace sead
{
namespace
{
class TextWriterGraphicsContext : public GraphicsContext
{
public:
    TextWriterGraphicsContext()
    {
        setCullingMode(0);
        setDepthEnable(false, false);
    }
};

TextWriterGraphicsContext sGraphicsContext;

}  // namespace

FontBase* TextWriter::sDefaultFont = nullptr;

/**
 * Constructs a text writer without a viewport.
 * @param pDrawContext draw context
 */
TextWriter::TextWriter(DrawContext* pDrawContext) : mViewport(nullptr), mDrawContext(pDrawContext)
{
}

/**
 * Gets the default font, falling back to the debug font manager.
 * @return the default font
 */
FontBase* TextWriter::getDefaultFont()
{
    return sDefaultFont ? sDefaultFont : DebugFontMgrNvn::instance();
}

/**
 * Constructs a text writer drawing into a viewport.
 * @param pDrawContext draw context
 * @param pViewport viewport
 */
TextWriter::TextWriter(DrawContext* pDrawContext, const Viewport* pViewport)
    : mViewport(pViewport), mDrawContext(pDrawContext)
{
}

/**
 * Sets the default font.
 * @param pFont font
 */
void TextWriter::setDefaultFont(FontBase* pFont)
{
    sDefaultFont = pFont;
}

/**
 * Applies the graphics state used for drawing text.
 * @param pDrawContext draw context
 */
void TextWriter::setupGraphics(DrawContext* pDrawContext)
{
    sGraphicsContext.apply(pDrawContext);
}

/**
 * Gets the cursor position relative to the top left corner of the viewport.
 * @param pPos output position
 */
void TextWriter::getCursorFromTopLeft(Vector2f* pPos) const
{
    pPos->x = mCursor.x + mViewport->getSizeX() * 0.5f;
    pPos->y = mViewport->getSizeY() * 0.5f - mCursor.y;
}

/**
 * Sets the cursor position relative to the top left corner of the viewport.
 * @param rPos position
 */
void TextWriter::setCursorFromTopLeft(const Vector2f& rPos)
{
    mCursor.x = rPos.x - mViewport->getSizeX() * 0.5f;
    mCursor.y = mViewport->getSizeY() * 0.5f - rPos.y;
}

/**
 * Sets the scale so that characters have the given size.
 * @param rFontSize character size
 */
void TextWriter::setScaleFromFontSize(const Vector2f& rFontSize)
{
    mScale.x = rFontSize.x / mFont->getWidth();
    mScale.y = rFontSize.y / mFont->getHeight();
}

/**
 * Sets a uniform scale so that characters have the given height.
 * @param fontHeight character height
 */
void TextWriter::setScaleFromFontHeight(f32 fontHeight)
{
    f32 scale = fontHeight / mFont->getHeight();
    mScale.set(scale, scale);
}

/**
 * Sets the projection and camera if both are valid.
 * @param pProjection projection
 * @param pCamera camera
 */
void TextWriter::setProjectionAndCamera(const Projection* pProjection, const Camera* pCamera)
{
    if (pProjection && pCamera)
    {
        mProjection = pProjection;
        mCamera = pCamera;
    }
}

/**
 * Sets the line spacing so that lines are the given height apart.
 * @param lineHeight line height
 */
void TextWriter::setLineSpaceFromLineHeight(f32 lineHeight)
{
    f32 fontHeight = mFont->getHeight() * mScale.y;
    mLineSpace = fontHeight <= lineHeight ? lineHeight - fontHeight : 0.0f;
}

/**
 * Sets the buffer used to format strings.
 * @param pBuffer buffer
 * @param size buffer size in characters
 */
void TextWriter::setFormatBuffer(char16_t* pBuffer, s32 size)
{
    if (size >= 0)
    {
        mFormatBuffer = pBuffer;
        mFormatBufferSize = size;
    }
}

/**
 * Sets the draw context.
 * @param pDrawContext draw context
 */
void TextWriter::setDrawContext(DrawContext* pDrawContext)
{
    mDrawContext = pDrawContext;
}

/**
 * Begins drawing multiple strings with a single font begin/end pair.
 */
void TextWriter::beginDraw()
{
    mFont->begin(mDrawContext);
    mIsAutoBeginEnd = false;
}

/**
 * Ends drawing started with beginDraw.
 */
void TextWriter::endDraw()
{
    mIsAutoBeginEnd = true;
    mFont->end(mDrawContext);
}

/**
 * Draws a formatted UTF-16 string at the cursor.
 * @param pFormat format string
 */
void TextWriter::printf(const char16_t* pFormat, ...)
{
    std::va_list args;
    va_start(args, pFormat);
    vprintfImpl_(pFormat, args, true, nullptr);
    va_end(args);
}

/**
 * Formats a UTF-16 string and draws and/or measures it.
 * @param pFormat format string
 * @param args format arguments
 * @param isDraw whether to draw the string
 * @param pRect output bounding rectangle, or nullptr
 */
void TextWriter::vprintfImpl_(const char16_t* pFormat, std::va_list args, bool isDraw,
                              BoundBox2f* pRect)
{
    char16_t localBuffer[0x200];
    char16_t* buffer;
    s32 bufferSize;

    if (mFormatBuffer)
    {
        buffer = mFormatBuffer;
        bufferSize = mFormatBufferSize;
    }
    else
    {
        buffer = localBuffer;
        bufferSize = 0x200;
    }

    BufferedSafeStringBase<char16_t> str(buffer, bufferSize);
    str.formatV(pFormat, args);
    printImpl_(str.cstr(), -1, isDraw, pRect);
}

/**
 * Draws a formatted UTF-16 string at the cursor and calculates its bounding rectangle.
 * @param pRect output bounding rectangle
 * @param pFormat format string
 */
void TextWriter::printfWithCalcRect(BoundBox2f* pRect, const char16_t* pFormat, ...)
{
    std::va_list args;
    va_start(args, pFormat);
    vprintfImpl_(pFormat, args, true, pRect);
    va_end(args);
}

/**
 * Draws a formatted UTF-8 string at the cursor.
 * @param pFormat format string
 */
void TextWriter::printf(const char* pFormat, ...)
{
    std::va_list args;
    va_start(args, pFormat);
    vprintfImpl_(pFormat, args, true, nullptr);
    va_end(args);
}

/**
 * Formats a UTF-8 string, converts it to UTF-16 and draws and/or measures it.
 * @param pFormat format string
 * @param args format arguments
 * @param isDraw whether to draw the string
 * @param pRect output bounding rectangle, or nullptr
 */
void TextWriter::vprintfImpl_(const char* pFormat, std::va_list args, bool isDraw,
                              BoundBox2f* pRect)
{
    char16_t localBuffer[0x200];
    char16_t* buffer;
    s32 bufferSize;

    if (mFormatBuffer)
    {
        buffer = mFormatBuffer;
        bufferSize = mFormatBufferSize;
    }
    else
    {
        buffer = localBuffer;
        bufferSize = 0x200;
    }

    BufferedSafeStringBase<char> str(reinterpret_cast<char*>(buffer) + bufferSize, bufferSize);
    str.formatV(pFormat, args);
    StringUtil::convertUtf8ToUtf16(buffer, bufferSize, str.cstr(), bufferSize - 1);
    printImpl_(buffer, -1, isDraw, pRect);
}

/**
 * Draws a formatted UTF-8 string at the cursor and calculates its bounding rectangle.
 * @param pRect output bounding rectangle
 * @param pFormat format string
 */
void TextWriter::printfWithCalcRect(BoundBox2f* pRect, const char* pFormat, ...)
{
    std::va_list args;
    va_start(args, pFormat);
    vprintfImpl_(pFormat, args, true, pRect);
    va_end(args);
}

/**
 * Calculates the bounding rectangle of a formatted UTF-16 string without drawing it.
 * @param pRect output bounding rectangle
 * @param pFormat format string
 */
void TextWriter::calcFormatStringRect(BoundBox2f* pRect, const char16_t* pFormat, ...)
{
    std::va_list args;
    va_start(args, pFormat);
    vprintfImpl_(pFormat, args, false, pRect);
    va_end(args);
}

/**
 * Calculates the bounding rectangle of a formatted UTF-8 string without drawing it.
 * @param pRect output bounding rectangle
 * @param pFormat format string
 */
void TextWriter::calcFormatStringRect(BoundBox2f* pRect, const char* pFormat, ...)
{
    std::va_list args;
    va_start(args, pFormat);
    vprintfImpl_(pFormat, args, false, pRect);
    va_end(args);
}

/**
 * Lays out a UTF-16 string line by line, drawing and/or measuring it.
 * @param pStr string
 * @param length maximum number of characters, or a negative value for no limit
 * @param isDraw whether to draw the string
 * @param pRect output bounding rectangle, or nullptr
 * @param pProjection projection used for drawing
 * @param pCamera camera used for drawing
 */
void TextWriter::printImpl_(const char16_t* pStr, s32 length, bool isDraw, BoundBox2f* pRect,
                            const Projection* pProjection, const Camera* pCamera)
{
    Matrix34f local(mScale.x, 0.0f, 0.0f, 0.0f, 0.0f, mScale.y, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    s32 maxDrawNum = mFont->getMaxDrawNum();
    f32 x = mCursor.x;
    f32 y = mCursor.y;
    f32 fontHeight = mFont->getHeight() * mScale.y;

    const f32 wrapX = x + mWrapWidth;

    if (pRect)
    {
        *pRect = BoundBox2f(x, y - fontHeight, x, y);
    }

    if (mIsAutoBeginEnd && isDraw)
    {
        mFont->begin(mDrawContext);
    }

    f32 lineX = x;
    f32 endWidth = 0.0f;
    s32 lineStart = 0;
    const char16_t* line = pStr;

    while (*line != u'\0')
    {
        LineEnd lineEnd = LineEnd::cEnd;
        s32 count = 0;
        f32 width = 0.0f;

        for (char16_t c = *line; c != u'\0'; c = line[++count])
        {
            if (length >= 0 && lineStart + count >= length)
            {
                lineEnd = LineEnd::cEnd;
                break;
            }

            if (c == u'\n')
            {
                lineEnd = LineEnd::cNewLine;
                break;
            }

            if (count >= maxDrawNum)
            {
                lineEnd = LineEnd::cMaxDraw;
                break;
            }

            f32 charWidth = mFont->getCharWidth(c) * mScale.x;

            if (mWrapWidth != std::numeric_limits<f32>::infinity() &&
                wrapX < lineX + width + charWidth)
            {
                lineEnd = LineEnd::cWrap;
                break;
            }

            width += charWidth;
        }

        if (isDraw && count != 0)
        {
            local.setTranslation(lineX, y - fontHeight, 0.0f);

            if (mModelMatrix)
            {
                Matrix34f mtx;
                mtx.setMul(*mModelMatrix, local);
                mFont->print(mDrawContext, *pProjection, *pCamera, mtx, mColor, line, count);
            }
            else
            {
                mFont->print(mDrawContext, *pProjection, *pCamera, local, mColor, line, count);
            }
        }

        if (pRect && count != 0)
        {
            pRect->addPoint(Vector2f(lineX + width, y - fontHeight));
        }

        if (lineEnd == LineEnd::cMaxDraw)
        {
            lineX += width;
        }
        else if (lineEnd == LineEnd::cEnd)
        {
            endWidth = width;
            break;
        }
        else
        {
            y -= fontHeight + mLineSpace;
            lineX = x;

            if (lineEnd == LineEnd::cNewLine)
            {
                count++;
            }
        }

        lineStart += count;
        line = pStr + lineStart;
    }

    if (isDraw)
    {
        if (mIsAutoBeginEnd)
        {
            mFont->end(mDrawContext);
        }

        mCursor.set(lineX + endWidth, y);
    }
}

/**
 * Lays out a UTF-16 string using the writer's projection and camera, or a viewport aligned
 * orthographic projection if none were set.
 * @param pStr string
 * @param length maximum number of characters, or a negative value for no limit
 * @param isDraw whether to draw the string
 * @param pRect output bounding rectangle, or nullptr
 */
void TextWriter::printImpl_(const char16_t* pStr, s32 length, bool isDraw, BoundBox2f* pRect)
{
    if ((!mProjection || !mCamera) && isDraw)
    {
        OrthoProjection projection(1.0f, 1000.0f, *mViewport);
        OrthoCamera camera(projection);
        camera.updateViewMatrix();
        printImpl_(pStr, length, isDraw, pRect, &projection, &camera);
    }
    else
    {
        printImpl_(pStr, length, isDraw, pRect, mProjection, mCamera);
    }
}

/**
 * Converts a UTF-8 string to UTF-16 and lays it out.
 * @param pStr string
 * @param length maximum number of characters, or a negative value for no limit
 * @param isDraw whether to draw the string
 * @param pRect output bounding rectangle, or nullptr
 */
void TextWriter::printImpl_(const char* pStr, s32 length, bool isDraw, BoundBox2f* pRect)
{
    char16_t localBuffer[0x200];
    char16_t* buffer;
    s32 bufferSize;

    if (mFormatBuffer)
    {
        buffer = mFormatBuffer;
        bufferSize = mFormatBufferSize;
    }
    else
    {
        buffer = localBuffer;
        bufferSize = 0x200;
    }

    s32 convertSize = length < 0 ? bufferSize : (bufferSize < length + 1 ? bufferSize : length + 1);
    StringUtil::convertUtf8ToUtf16(buffer, convertSize, pStr, convertSize - 1);
    printImpl_(buffer, -1, isDraw, pRect);
}

}  // namespace sead
