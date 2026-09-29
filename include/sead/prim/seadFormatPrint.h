#pragma once

#include "basis/seadTypes.h"
#include "prim/seadSafeString.h"
#include "stream/seadBufferStream.h"
#include "stream/seadPrintStream.h"

namespace sead
{
class PrintFormatter;
class StreamSrc;

// region Print outputs
class PrintOutput
{
public:
    virtual ~PrintOutput();
    virtual void write(const char* pString, s32 size) = 0;
    void writeLineBreak();
    PrintFormatter& operator<<(PrintFormatter& rFormatter);
};

class StringPrintOutput : public PrintOutput
{
public:
    explicit StringPrintOutput(BufferedSafeString* pBuffer) : mBuffer(pBuffer), mPos(0)
    {
        pBuffer->clear();
    }
    ~StringPrintOutput() override = default;
    void write(const char* pString, s32 size) override;

protected:
    BufferedSafeString* mBuffer;
    s32 mPos;
};

class StringCutOffPrintOutput : public PrintOutput
{
public:
    explicit StringCutOffPrintOutput(BufferedSafeString* pBuffer) : mBuffer(pBuffer), mPos(0)
    {
        pBuffer->clear();
    }
    ~StringCutOffPrintOutput() override = default;
    void write(const char* pString, s32 size) override;

protected:
    BufferedSafeString* mBuffer;
    s32 mPos;
};

class StreamPrintOutput : public PrintOutput
{
public:
    explicit StreamPrintOutput(StreamSrc* pSrc) : mSrc(pSrc) {}
    ~StreamPrintOutput() override;
    void write(const char* pString, s32 size) override;

protected:
    friend class StreamPrintFormatter;

    StreamSrc* mSrc;
};

class BufferingPrintOutput : public PrintOutput
{
public:
    BufferingPrintOutput(char* pBuffer, u32 bufferSize)
        : mSrc(&PrintStreamSrc::sPrintStreamSrc, pBuffer, bufferSize - 1)
    {
    }
    ~BufferingPrintOutput() override;
    void write(const char* pString, s32 size) override;

protected:
    BufferMultiByteNullTerminatedTextWriteStreamSrc mSrc;
};
// endregion

// region Print formatters

class PrintFormatter
{
public:
    template <typename T, template <typename> class Class>
    class OutImpl
    {
    public:
        static void out(const Class<T>& rValue, const char* pFormat, PrintOutput* pOutput);
    };

    PrintFormatter(const char* pFormat, PrintOutput* pOutput);

    void setPrintOutput(PrintOutput* pOutput) { mPrintOutput = pOutput; }

    void flush();
    void flushWithLineBreak();

    PrintFormatter& operator,(s8);
    PrintFormatter& operator,(u8);
    PrintFormatter& operator,(s16);
    PrintFormatter& operator,(u16);
    PrintFormatter& operator,(s32);
    PrintFormatter& operator,(u32);
    PrintFormatter& operator<<(char*);
    PrintFormatter& operator<<(const char* pFormat);

    PrintFormatter& operator<<(PrintFormatter& (&fn)(PrintFormatter&)) { return fn(*this); }

    template <typename T>
    PrintFormatter& operator,(const T&);

    template <typename T>
    static void out(const T& rValue, const char* pFormat, PrintOutput* pOutput);
    template <typename T>
    static void out(const T* pValue, const char* pFormat, PrintOutput* pOutput);

protected:
    bool proceedToFormatMark_(char* pFormat);
    static bool isQualification_(char c)
    {
        switch (c)
        {
        case ' ':
        case '#':
        case '+':
        case '-':
        case '.':
        case 'L':
        case 'h':
        case 'l':
            return true;
        default:
            return (c >= '0' && c <= '9') || c == 'z';
        }
    }
    static void outputString_(const char* pFormat, PrintOutput* pOutput, const char* pString,
                              s32 length);
    static void outputPtr_(const char* pFormat, PrintOutput* pOutput, uintptr_t ptr);

    const char* mFormatStr;
    PrintOutput* mPrintOutput;
    s32 mPos;
    s32 mFormatStrLength;
    bool mIsFormatSkipped;
};

inline PrintFormatter& flush(PrintFormatter& rFormatter)
{
    rFormatter.flush();
    return rFormatter;
}

class StringPrintFormatter : public PrintFormatter
{
public:
    explicit StringPrintFormatter(BufferedSafeString* pString);
    StringPrintFormatter(BufferedSafeString* pString, const char* pFormat);

protected:
    StringPrintOutput mOutput;
};

class StringCutOffPrintFormatter : public PrintFormatter
{
public:
    explicit StringCutOffPrintFormatter(BufferedSafeString* pString);
    StringCutOffPrintFormatter(BufferedSafeString* pString, const char* pFormat);

protected:
    StringCutOffPrintOutput mOutput;
};

class StreamPrintFormatter : public PrintFormatter
{
public:
    explicit StreamPrintFormatter(StreamSrc* pSrc);
    StreamPrintFormatter(StreamSrc* pSrc, const char* pFormat);
    void flushAndWriteNullChar();

protected:
    StreamPrintOutput mOutput;
};

class BufferingPrintFormatter : public PrintFormatter
{
public:
    BufferingPrintFormatter();
    explicit BufferingPrintFormatter(const char* pFormat);

protected:
    BufferingPrintOutput mOutput;
    char mBuffer[128];
};

class TimeBufferingPrintFormatter : public BufferingPrintFormatter
{
public:
    TimeBufferingPrintFormatter();
    explicit TimeBufferingPrintFormatter(const char* pFormat);

private:
    void outputTimeStamp_();
};
// endregion
}  // namespace sead
