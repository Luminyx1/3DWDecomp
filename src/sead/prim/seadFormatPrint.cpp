#include <prim/seadFormatPrint.h>

#include <cstring>

#include <gfx/seadColor.h>
#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>
#include <prim/seadCuckooClock.h>
#include <prim/seadStringBuilder.h>
#include <prim/seadStringUtil.h>
#include <time/seadTickSpan.h>

namespace sead
{
/**
 * Writes a line break.
 */
void PrintOutput::writeLineBreak()
{
    write(&SafeString::cLineBreakChar, 1);
}

/**
 * Makes a formatter print to this output.
 * @param rFormatter formatter to redirect
 * @return rFormatter
 */
PrintFormatter& PrintOutput::operator<<(PrintFormatter& rFormatter)
{
    rFormatter.setPrintOutput(this);
    return rFormatter;
}

/**
 * Destroys the output.
 */
PrintOutput::~PrintOutput() = default;

/**
 * Flushes the buffered text before destroying the output.
 */
BufferingPrintOutput::~BufferingPrintOutput()
{
    mSrc.flush();
}

/**
 * Appends text to the buffer.
 * @param pString text to write
 * @param size number of characters to write
 */
void BufferingPrintOutput::write(const char* pString, s32 size)
{
    mSrc.write(pString, size);
}

// NON_MATCHING: ~81%; the out of range branch of the temporary string is laid out differently
void StringPrintOutput::write(const char* pString, s32 size)
{
    BufferedSafeString str(mBuffer, mPos);
    mPos += str.cutOffCopy(pString, size);
}

// NON_MATCHING: ~81%; the out of range branch of the temporary string is laid out differently
void StringCutOffPrintOutput::write(const char* pString, s32 size)
{
    BufferedSafeString str(mBuffer, mPos);
    mPos += str.cutOffCopy(pString, size);
}

/**
 * Flushes the stream before destroying the output.
 */
StreamPrintOutput::~StreamPrintOutput()
{
    mSrc->flush();
}

/**
 * Writes text to the stream.
 * @param pString text to write
 * @param size number of characters to write
 */
void StreamPrintOutput::write(const char* pString, s32 size)
{
    mSrc->write(pString, size);
}

/**
 * Creates a formatter.
 * @param pFormat format string (may be null)
 * @param pOutput output to print to
 */
PrintFormatter::PrintFormatter(const char* pFormat, PrintOutput* pOutput)
    : mFormatStr(pFormat), mPrintOutput(pOutput), mPos(0), mFormatStrLength(0),
      mIsFormatSkipped(false)
{
    if (pFormat)
    {
        mFormatStrLength = std::strlen(pFormat);
    }
}

// NON_MATCHING: ~81%; the null check is not moved before the stack frame setup
void PrintFormatter::flush()
{
    if (mFormatStr)
    {
        mIsFormatSkipped = false;

        while (mPos < mFormatStrLength)
        {
            char format[32];
            proceedToFormatMark_(format);
        }
    }
}

// NON_MATCHING: ~74%; the pending text length is known to be zero after the mark in our build
bool PrintFormatter::proceedToFormatMark_(char* pFormat)
{
    pFormat[0] = '\0';

    if (!mFormatStr)
    {
        return false;
    }

    if (mIsFormatSkipped)
    {
        return true;
    }

    if (mPos >= mFormatStrLength)
    {
        return false;
    }

    const char* str = mFormatStr + mPos;
    s32 i = 0;

    while (str[i] != '\0')
    {
        if (str[i] != '%')
        {
            ++i;
            continue;
        }

        const char* mark = &str[i];

        if (mark[1] == '%')
        {
            mPrintOutput->write(str, i + 1);
            mPos += i + 2;
            str = mark + 2;
            i = 0;
            continue;
        }

        if (i > 0)
        {
            mPrintOutput->write(str, i);
            mPos += i;
            str = mark;
            i = 0;
        }

        s32 len = i + 1;

        if (str[len] == '<' || str[len] == '@')
        {
            if (str[len] == '<')
            {
                mIsFormatSkipped = true;
            }

            mPos += i + 2;
            return true;
        }

        pFormat[0] = '%';

        while (isQualification_(str[len]))
        {
            if (len >= 31)
            {
                pFormat[0] = '\0';
                mPos = mFormatStrLength;
                return true;
            }

            pFormat[len] = str[len];
            ++len;
        }

        if (str[len] == '\0')
        {
            pFormat[0] = '\0';
            mPos = mFormatStrLength;
            return true;
        }

        pFormat[len] = str[len];
        pFormat[len + 1] = '\0';
        mPos += len + 1;
        return true;
    }

    if (i > 0)
    {
        mPrintOutput->write(str, i);
        mPos += i;
    }

    return false;
}

/**
 * Prints the rest of the format string followed by a line break.
 */
void PrintFormatter::flushWithLineBreak()
{
    if (!mFormatStr)
    {
        return;
    }

    flush();

    if (mFormatStr)
    {
        mPrintOutput->writeLineBreak();
    }
}

/**
 * Sets the format string, or prints a string for the next format mark.
 * @param pFormat format string or argument
 * @return this formatter
 */
PrintFormatter& PrintFormatter::operator<<(const char* pFormat)
{
    if (!mFormatStr)
    {
        mFormatStr = pFormat;
        mFormatStrLength = std::strlen(pFormat);
        return *this;
    }

    char format[32];

    if (proceedToFormatMark_(format))
    {
        outputString_(format[0] != '\0' ? format : nullptr, mPrintOutput, pFormat, -1);
    }

    return *this;
}

/**
 * Prints a string, handling the width of %s marks.
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 * @param pString string to print
 * @param length length of the string (-1 for null-terminated)
 */
void PrintFormatter::outputString_(const char* pFormat, PrintOutput* pOutput, const char* pString,
                                   s32 length)
{
    if (!pFormat || pFormat[1] == 's')
    {
        if (length == -1)
        {
            length = std::strlen(pString);
        }

        pOutput->write(pString, length);
        return;
    }

    if (pFormat[std::strlen(pFormat) - 1] == 's')
    {
        FixedSafeString<128> sjis;
        const s32 width = StringUtil::convertUtf8ToSjis(sjis.getBuffer(), sjis.getBufferSize(),
                                                        pString, -1);
        if (pFormat[1] == '-')
        {
            pOutput->write(pString, length == -1 ? std::strlen(pString) : length);
            const s32 padding =
                StringUtil::parseNumber<s32>(pFormat + 2, StringUtil::CardinalNumber::Base10) -
                width;
            for (s32 i = 0; i < padding; ++i)
            {
                pOutput->write(" ", 1);
            }
        }
        else
        {
            const s32 padding =
                StringUtil::parseNumber<s32>(pFormat + 1, StringUtil::CardinalNumber::Base10) -
                width;
            for (s32 i = 0; i < padding; ++i)
            {
                pOutput->write(" ", 1);
            }

            pOutput->write(pString, length == -1 ? std::strlen(pString) : length);
        }

        return;
    }

    FixedSafeString<128> str;
    const s32 len = str.format(pFormat, pString);
    pOutput->write(str.cstr(), len);
}

/**
 * Prints a pointer.
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 * @param ptr pointer to print
 */
void PrintFormatter::outputPtr_(const char* pFormat, PrintOutput* pOutput, uintptr_t ptr)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, ptr);
    }
    else
    {
        len = str.format("0x%016llX", ptr);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Creates a formatter that prints through a line buffer.
 */
BufferingPrintFormatter::BufferingPrintFormatter()
    : PrintFormatter(nullptr, &mOutput), mOutput(mBuffer, sizeof(mBuffer))
{
}

/**
 * Creates a formatter that prints through a line buffer.
 * @param pFormat format string
 */
BufferingPrintFormatter::BufferingPrintFormatter(const char* pFormat)
    : PrintFormatter(pFormat, &mOutput), mOutput(mBuffer, sizeof(mBuffer))
{
}

/**
 * Creates a buffering formatter that starts with a time stamp.
 */
TimeBufferingPrintFormatter::TimeBufferingPrintFormatter()
{
    outputTimeStamp_();
}

// NON_MATCHING: needs `mBufferSize - 1 - at` in BufferedSafeStringBase::copyAtWithTerminate
void TimeBufferingPrintFormatter::outputTimeStamp_()
{
    CuckooClock* clock = CuckooClock::instance();

    if (!clock)
    {
        return;
    }

    FixedSafeString<16> str;
    const s32 len = clock->getTimeString(&str);
    str.copyAtWithTerminate(len, " ", 1);
    mPrintOutput->write(str.cstr(), len + 1);
}

/**
 * Creates a buffering formatter that starts with a time stamp.
 * @param pFormat format string
 */
TimeBufferingPrintFormatter::TimeBufferingPrintFormatter(const char* pFormat)
    : BufferingPrintFormatter(pFormat)
{
    outputTimeStamp_();
}

/**
 * Creates a formatter that prints into a string.
 * @param pString destination string
 */
StringPrintFormatter::StringPrintFormatter(BufferedSafeString* pString)
    : PrintFormatter(nullptr, &mOutput), mOutput(pString)
{
}

/**
 * Creates a formatter that prints into a string.
 * @param pString destination string
 * @param pFormat format string
 */
StringPrintFormatter::StringPrintFormatter(BufferedSafeString* pString, const char* pFormat)
    : PrintFormatter(pFormat, &mOutput), mOutput(pString)
{
}

/**
 * Creates a formatter that prints into a string, truncating the output.
 * @param pString destination string
 */
StringCutOffPrintFormatter::StringCutOffPrintFormatter(BufferedSafeString* pString)
    : PrintFormatter(nullptr, &mOutput), mOutput(pString)
{
}

/**
 * Creates a formatter that prints into a string, truncating the output.
 * @param pString destination string
 * @param pFormat format string
 */
StringCutOffPrintFormatter::StringCutOffPrintFormatter(BufferedSafeString* pString,
                                                       const char* pFormat)
    : PrintFormatter(pFormat, &mOutput), mOutput(pString)
{
}

/**
 * Creates a formatter that prints into a stream.
 * @param pSrc destination stream
 */
StreamPrintFormatter::StreamPrintFormatter(StreamSrc* pSrc)
    : PrintFormatter(nullptr, &mOutput), mOutput(pSrc)
{
}

/**
 * Creates a formatter that prints into a stream.
 * @param pSrc destination stream
 * @param pFormat format string
 */
StreamPrintFormatter::StreamPrintFormatter(StreamSrc* pSrc, const char* pFormat)
    : PrintFormatter(pFormat, &mOutput), mOutput(pSrc)
{
}

/**
 * Prints the rest of the format string followed by a null character.
 */
void StreamPrintFormatter::flushAndWriteNullChar()
{
    flush();
    mOutput.mSrc->write(&SafeString::cNullChar, 1);
}

/**
 * Prints an integer.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<u8>(const u8& rValue, const char* pFormat, PrintOutput* pOutput)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, rValue);
    }
    else
    {
        len = str.format("%u", rValue);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints an integer.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<u16>(const u16& rValue, const char* pFormat, PrintOutput* pOutput)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, rValue);
    }
    else
    {
        len = str.format("%u", rValue);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints an integer.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<u32>(const u32& rValue, const char* pFormat, PrintOutput* pOutput)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, rValue);
    }
    else
    {
        len = str.format("%u", rValue);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints an integer.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<u64>(const u64& rValue, const char* pFormat, PrintOutput* pOutput)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, rValue);
    }
    else
    {
        len = str.format("%llu", rValue);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints an integer.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<s8>(const s8& rValue, const char* pFormat, PrintOutput* pOutput)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, rValue);
    }
    else
    {
        len = str.format("%d", rValue);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints an integer.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<s16>(const s16& rValue, const char* pFormat, PrintOutput* pOutput)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, rValue);
    }
    else
    {
        len = str.format("%d", rValue);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints an integer.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<s32>(const s32& rValue, const char* pFormat, PrintOutput* pOutput)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, rValue);
    }
    else
    {
        len = str.format("%d", rValue);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints an integer.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<s64>(const s64& rValue, const char* pFormat, PrintOutput* pOutput)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, rValue);
    }
    else
    {
        len = str.format("%lld", rValue);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints a floating point number.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<f32>(const f32& rValue, const char* pFormat, PrintOutput* pOutput)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, rValue);
    }
    else
    {
        len = str.format("%f", rValue);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints a floating point number.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<f64>(const f64& rValue, const char* pFormat, PrintOutput* pOutput)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, rValue);
    }
    else
    {
        len = str.format("%f", rValue);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints a character code.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<char>(const char& rValue, const char* pFormat, PrintOutput* pOutput)
{
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, rValue);
    }
    else
    {
        len = str.format("%d", rValue);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints a boolean as true or false.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<bool>(const bool& rValue, const char* pFormat, PrintOutput* pOutput)
{
    const bool value = rValue;

    if (pFormat)
    {
        FixedSafeString<32> str;
        const s32 len = str.format(pFormat, value);
        pOutput->write(str.cstr(), len);
    }
    else if (value)
    {
        pOutput->write("true", 4);
    }
    else
    {
        pOutput->write("false", 5);
    }
}

/**
 * Prints a string.
 * @param pValue string to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<char>(const char* pValue, const char* pFormat, PrintOutput* pOutput)
{
    if (!pFormat || SafeString(pFormat).include('s'))
    {
        outputString_(pFormat, pOutput, pValue, -1);
        return;
    }

    FixedSafeString<32> str;
    const s32 len = str.format(pFormat, pValue);
    pOutput->write(str.cstr(), len);
}

/**
 * Prints a UTF-16 string as UTF-8.
 * @param pValue string to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<char16>(const char16* pValue, const char* pFormat, PrintOutput* pOutput)
{
    if (!pFormat || SafeString(pFormat).include('s'))
    {
        FixedSafeString<256> str;
        const s32 len =
            StringUtil::convertUtf16ToUtf8(str.getBuffer(), str.getBufferSize(), pValue, -1);
        outputString_(pFormat, pOutput, str.cstr(), len);
        return;
    }

    FixedSafeString<32> str;
    const s32 len = str.format(pFormat, pValue);
    pOutput->write(str.cstr(), len);
}

/**
 * Prints a null pointer.
 * @param rValue value to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<std::nullptr_t>(const std::nullptr_t& rValue, const char* pFormat,
                                         PrintOutput* pOutput)
{
    const u64 value = reinterpret_cast<const u64&>(rValue);
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, value);
    }
    else
    {
        len = str.format("%llu", value);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints a string.
 * @param rValue string to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<char, SafeStringBase>::out(const SafeString& rValue,
                                                        const char* pFormat,
                                                        PrintOutput* pOutput)
{
    outputString_(pFormat, pOutput, rValue.cstr(), rValue.calcLength());
}

/**
 * Prints a string.
 * @param rValue string to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<char, BufferedSafeStringBase>::out(const BufferedSafeString& rValue,
                                                                const char* pFormat,
                                                                PrintOutput* pOutput)
{
    outputString_(pFormat, pOutput, rValue.cstr(), rValue.calcLength());
}

/**
 * Prints a vector as (x, y).
 * @param rValue vector to print
 * @param pFormat format mark for each component (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<f32, Vector2>::out(const Vector2f& rValue, const char* pFormat,
                                                PrintOutput* pOutput)
{
    if (pFormat)
    {
        pOutput->write("(", 1);
        PrintFormatter::out<f32>(rValue.x, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<f32>(rValue.y, pFormat, pOutput);
        pOutput->write(")", 1);
    }
    else
    {
        FixedSafeString<64> str;
        const s32 len = str.format("(%f, %f)", rValue.x, rValue.y);
        pOutput->write(str.cstr(), len);
    }
}

/**
 * Prints a vector as (x, y, z).
 * @param rValue vector to print
 * @param pFormat format mark for each component (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<f32, Vector3>::out(const Vector3f& rValue, const char* pFormat,
                                                PrintOutput* pOutput)
{
    if (pFormat)
    {
        pOutput->write("(", 1);
        PrintFormatter::out<f32>(rValue.x, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<f32>(rValue.y, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<f32>(rValue.z, pFormat, pOutput);
        pOutput->write(")", 1);
    }
    else
    {
        FixedSafeString<64> str;
        const s32 len = str.format("(%f, %f, %f)", rValue.x, rValue.y, rValue.z);
        pOutput->write(str.cstr(), len);
    }
}

/**
 * Prints a vector as (x, y, z, w).
 * @param rValue vector to print
 * @param pFormat format mark for each component (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<f32, Vector4>::out(const Vector4f& rValue, const char* pFormat,
                                                PrintOutput* pOutput)
{
    if (pFormat)
    {
        pOutput->write("(", 1);
        PrintFormatter::out<f32>(rValue.x, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<f32>(rValue.y, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<f32>(rValue.z, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<f32>(rValue.w, pFormat, pOutput);
        pOutput->write(")", 1);
    }
    else
    {
        FixedSafeString<64> str;
        const s32 len =
            str.format("(%f, %f, %f, %f)", rValue.x, rValue.y, rValue.z, rValue.w);
        pOutput->write(str.cstr(), len);
    }
}

template <typename Matrix>
/**
 * Prints the rows of a matrix, closing the parenthesis after the last row.
 * @param rMtx matrix to print
 * @param rows number of rows
 * @param cols number of columns
 * @param pFormat format mark for each element (null for the default format)
 * @param pOutput output to print to
 */
static void outMatrix_(const Matrix& rMtx, s32 rows, s32 cols, const char* pFormat,
                       PrintOutput* pOutput)
{
    pOutput->write("(", 1);

    for (s32 i = 0; i < rows; ++i)
    {
        for (s32 j = 0; j < cols; ++j)
        {
            PrintFormatter::out<f32>(rMtx(i, j), pFormat, pOutput);

            if (j < cols - 1)
            {
                pOutput->write(" ", 1);
            }
        }

        if (i < rows - 1)
        {
            pOutput->write("\n ", 2);
        }
        else
        {
            pOutput->write(")", 1);
        }
    }
}

/**
 * Prints a matrix row by row.
 * @param rValue matrix to print
 * @param pFormat format mark for each element (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<f32, Matrix22>::out(const Matrix22f& rValue, const char* pFormat,
                                                 PrintOutput* pOutput)
{
    pOutput->write("(", 1);

    for (s32 i = 0; i < 2; ++i)
    {
        for (s32 j = 0; j < 2; ++j)
        {
            PrintFormatter::out<f32>(rValue(i, j), pFormat, pOutput);

            if (j < 1)
            {
                pOutput->write(" ", 1);
            }
        }

        if (i < 1)
        {
            pOutput->write("\n ", 2);
        }
    }

    pOutput->write(")", 1);
}

/**
 * Prints a matrix row by row.
 * @param rValue matrix to print
 * @param pFormat format mark for each element (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<f32, Matrix23>::out(const Matrix23f& rValue, const char* pFormat,
                                                 PrintOutput* pOutput)
{
    outMatrix_(rValue, 2, 3, pFormat, pOutput);
}

/**
 * Prints a matrix row by row.
 * @param rValue matrix to print
 * @param pFormat format mark for each element (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<f32, Matrix33>::out(const Matrix33f& rValue, const char* pFormat,
                                                 PrintOutput* pOutput)
{
    outMatrix_(rValue, 3, 3, pFormat, pOutput);
}

/**
 * Prints a matrix row by row.
 * @param rValue matrix to print
 * @param pFormat format mark for each element (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<f32, Matrix34>::out(const Matrix34f& rValue, const char* pFormat,
                                                 PrintOutput* pOutput)
{
    outMatrix_(rValue, 3, 4, pFormat, pOutput);
}

/**
 * Prints a matrix row by row.
 * @param rValue matrix to print
 * @param pFormat format mark for each element (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<f32, Matrix44>::out(const Matrix44f& rValue, const char* pFormat,
                                                 PrintOutput* pOutput)
{
    outMatrix_(rValue, 4, 4, pFormat, pOutput);
}

/**
 * Prints a quaternion as (w; x, y, z).
 * @param rValue quaternion to print
 * @param pFormat format mark for each component (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<f32, Quat>::out(const Quatf& rValue, const char* pFormat,
                                             PrintOutput* pOutput)
{
    if (pFormat)
    {
        pOutput->write("(", 1);
        PrintFormatter::out<f32>(rValue.w, pFormat, pOutput);
        pOutput->write("; ", 2);
        PrintFormatter::out<f32>(rValue.x, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<f32>(rValue.y, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<f32>(rValue.z, pFormat, pOutput);
        pOutput->write(")", 1);
    }
    else
    {
        FixedSafeString<64> str;
        const s32 len =
            str.format("(%f; %f, %f, %f)", rValue.w, rValue.x, rValue.y, rValue.z);
        pOutput->write(str.cstr(), len);
    }
}

/**
 * Prints a bounding box as (min-max).
 * @param rValue box to print
 * @param pFormat format mark for each component (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<f32, BoundBox2>::out(const BoundBox2f& rValue, const char* pFormat,
                                                  PrintOutput* pOutput)
{
    pOutput->write("(", 1);
    OutImpl<f32, Vector2>::out(rValue.getMin(), pFormat, pOutput);
    pOutput->write("-", 1);
    OutImpl<f32, Vector2>::out(rValue.getMax(), pFormat, pOutput);
    pOutput->write(")", 1);
}

/**
 * Prints a bounding box as (min-max).
 * @param rValue box to print
 * @param pFormat format mark for each component (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<f32, BoundBox3>::out(const BoundBox3f& rValue, const char* pFormat,
                                                  PrintOutput* pOutput)
{
    pOutput->write("(", 1);
    OutImpl<f32, Vector3>::out(rValue.getMin(), pFormat, pOutput);
    pOutput->write("-", 1);
    OutImpl<f32, Vector3>::out(rValue.getMax(), pFormat, pOutput);
    pOutput->write(")", 1);
}

// NON_MATCHING: needs TickSpan::toMicroSeconds to be computed without toNanoSeconds
template <>
void PrintFormatter::out<TickSpan>(const TickSpan& rValue, const char*, PrintOutput* pOutput)
{
    PrintFormatter::out<s64>(rValue.toMicroSeconds(), "%lld", pOutput);
    pOutput->write("us", 2);
}

/**
 * Prints the bits of a flag set.
 * @param rValue flags to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<u8, BitFlag>::out(const BitFlag8& rValue, const char* pFormat,
                                               PrintOutput* pOutput)
{
    const auto bits = rValue.getDirect();
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, bits);
    }
    else
    {
        len = str.format("0x%x", bits);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints the bits of a flag set.
 * @param rValue flags to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<u16, BitFlag>::out(const BitFlag16& rValue, const char* pFormat,
                                                PrintOutput* pOutput)
{
    const auto bits = rValue.getDirect();
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, bits);
    }
    else
    {
        len = str.format("0x%x", bits);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints the bits of a flag set.
 * @param rValue flags to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<u32, BitFlag>::out(const BitFlag32& rValue, const char* pFormat,
                                                PrintOutput* pOutput)
{
    const auto bits = rValue.getDirect();
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, bits);
    }
    else
    {
        len = str.format("0x%x", bits);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints the bits of a flag set.
 * @param rValue flags to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<u64, BitFlag>::out(const BitFlag64& rValue, const char* pFormat,
                                                PrintOutput* pOutput)
{
    const auto bits = rValue.getDirect();
    FixedSafeString<32> str;
    s32 len;

    if (pFormat)
    {
        len = str.format(pFormat, bits);
    }
    else
    {
        len = str.format("0x%llx", bits);
    }

    pOutput->write(str.cstr(), len);
}

/**
 * Prints a color as (r, g, b, a).
 * @param rValue color to print
 * @param pFormat format mark for each component (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<Color4f>(const Color4f& rValue, const char* pFormat,
                                  PrintOutput* pOutput)
{
    if (pFormat)
    {
        pOutput->write("(", 1);
        PrintFormatter::out<f32>(rValue.r, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<f32>(rValue.g, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<f32>(rValue.b, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<f32>(rValue.a, pFormat, pOutput);
        pOutput->write(")", 1);
    }
    else
    {
        FixedSafeString<64> str;
        const s32 len =
            str.format("(%f, %f, %f, %f)", rValue.r, rValue.g, rValue.b, rValue.a);
        pOutput->write(str.cstr(), len);
    }
}

/**
 * Prints a color as (r, g, b, a).
 * @param rValue color to print
 * @param pFormat format mark for each component (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::out<Color4u8>(const Color4u8& rValue, const char* pFormat,
                                   PrintOutput* pOutput)
{
    if (pFormat)
    {
        pOutput->write("(", 1);
        PrintFormatter::out<u8>(rValue.r, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<u8>(rValue.g, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<u8>(rValue.b, pFormat, pOutput);
        pOutput->write(", ", 2);
        PrintFormatter::out<u8>(rValue.a, pFormat, pOutput);
        pOutput->write(")", 1);
    }
    else
    {
        FixedSafeString<64> str;
        const s32 len =
            str.format("(%d, %d, %d, %d)", rValue.r, rValue.g, rValue.b, rValue.a);
        pOutput->write(str.cstr(), len);
    }
}

/**
 * Prints a string.
 * @param rValue string to print
 * @param pFormat format mark (null for the default format)
 * @param pOutput output to print to
 */
template <>
void PrintFormatter::OutImpl<char, StringBuilderBase>::out(const StringBuilder& rValue,
                                                           const char* pFormat,
                                                           PrintOutput* pOutput)
{
    outputString_(pFormat, pOutput, rValue.cstr(), rValue.getLength());
}
}  // namespace sead
