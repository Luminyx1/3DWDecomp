#include "common/aglResCommon.h"

#include <prim/seadEndian.h>

namespace agl
{

/**
 * Converts an array of 32-bit words between host and file endianness.
 * @param is_le whether the file data is little endian
 * @param p_data data to convert in place
 * @param size size of the data in bytes
 */
void ModifyEndianU32(bool is_le, void* p_data, size_t size)
{
    u32* data = static_cast<u32*>(p_data);
    const s32 count = size / sizeof(u32);
    for (s32 i = 0; i < count; i++)
    {
        data[i] = sead::Endian::fromHostU32(static_cast<sead::Endian::Types>(is_le), data[i]);
    }
}

}  // namespace agl
