#include <basis/seadRawPrint.h>
#include <devenv/seadAssertConfig.h>
#include <prim/seadStringUtil.h>

#include <cstring>

namespace nn::diag::detail
{
[[noreturn]] void AbortImpl(const char* condition, const char* function, const char* file,
                            int line);
}  // namespace nn::diag::detail

namespace sead
{
namespace system
{
namespace
{
const u32 cHaltMessageSize = 0x180;
char sHaltMessage[cHaltMessageSize];
}  // namespace

/**
 * Aborts the program.
 */
void Halt()
{
    nn::diag::detail::AbortImpl("", "", "", 0);
}

// NON_MATCHING: placement of the length clamp
/**
 * Prints a formatted halt report, runs the assert callbacks and aborts the program.
 * @param file Source file of the failure.
 * @param line Line number of the failure.
 * @param msg printf-style description.
 */
void HaltWithDetail(const char* file, int line, const char* msg, ...)
{
    std::memset(sHaltMessage, 0, sizeof(sHaltMessage));

    s32 length = StringUtil::snprintf(sHaltMessage, cHaltMessageSize,
                                      "\n//================= PROGRAM HALT ==================//"
                                      "\nSource File: %s\nLine Number: %d\nDescription: ",
                                      file, line);
    if (length >= 0)
    {
        std::va_list args;
        va_start(args, msg);
        s32 msgLength =
            StringUtil::vsnprintf(sHaltMessage + length, cHaltMessageSize - length, msg, args);
        va_end(args);

        if (msgLength >= 0)
        {
            length += msgLength;
            s32 footerLength =
                StringUtil::snprintf(sHaltMessage + length, cHaltMessageSize - length,
                                     "\n//=================================================//");
            if (footerLength > 0)
            {
                length += footerLength;

                if (length < s32(cHaltMessageSize) - 2)
                {
                    sHaltMessage[length] = '\n';
                    sHaltMessage[length + 1] = '\0';
                    length++;
                }
                else
                {
                    length = cHaltMessageSize - 1;
                }
            }
        }
        else
        {
            length = -1;
        }
    }

    sHaltMessage[cHaltMessageSize - 1] = '\0';

    if (length < 0)
    {
        PrintString(sHaltMessage, std::strlen(sHaltMessage));
    }
    else
    {
        PrintString(sHaltMessage, length);
    }

    AssertConfig::execCallbacks(sHaltMessage);
    Halt();
}

/**
 * Breaks into the debugger; does nothing in release builds.
 */
void DebugBreak() {}
}  // namespace system
}  // namespace sead
