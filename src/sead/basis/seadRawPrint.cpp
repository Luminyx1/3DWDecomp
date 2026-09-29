#include <basis/seadRawPrint.h>
#include <devenv/seadPrintConfig.h>
#include <prim/seadStringUtil.h>

namespace sead
{
namespace system
{
/**
 * Prints a formatted string.
 * @param format printf-style format.
 */
void Print(const char* format, ...)
{
    std::va_list args;
    va_start(args, format);
    PrintV(format, args);
    va_end(args);
}

/**
 * Prints a formatted string using a va_list.
 * @param format printf-style format.
 * @param args Format arguments.
 */
void PrintV(const char* format, std::va_list args)
{
    PrintConfig::PrintEventArg arg;
    char buffer[0x200];
    arg.length = StringUtil::vsnprintf(buffer, sizeof(buffer), format, args);
    arg.str = buffer;
    PrintConfig::execCallbacks(arg);
}

/**
 * Prints a string of known length.
 * @param str String to print.
 * @param length Length of the string.
 */
void PrintString(const char* str, s32 length)
{
    PrintConfig::PrintEventArg arg;
    arg.str = str;
    arg.length = length;
    PrintConfig::execCallbacks(arg);
}
}  // namespace system
}  // namespace sead
