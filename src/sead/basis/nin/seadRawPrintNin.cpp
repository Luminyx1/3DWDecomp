#include <basis/seadRawPrint.h>

namespace sead
{
namespace system
{
/**
 * Writes a string to the platform debug output; does nothing in release builds.
 * @param str String to print.
 * @param length Length of the string.
 */
void PrintStringImpl(const char* str, s32 length) {}
}  // namespace system
}  // namespace sead
