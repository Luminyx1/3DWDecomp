#include <heap/seadArena.h>
#include <nn/os.h>

namespace sead
{
/**
 * Allocates the arena's memory from the OS, in whole 2 MiB blocks.
 * @param size bytes to allocate (rounded up to 2 MiB)
 */
void Arena::initialize(size_t size)
{
    nn::os::AllocateMemoryBlock(reinterpret_cast<uintptr_t*>(&mStart), (size + 0x1FFFFF) & 0xFFFFFFFFFFE00000LL);
    mSize = size;
}

/**
 * Returns the arena's memory to the OS, unless it was handed in with a start address.
 */
void Arena::destroy()
{
    if (!mInitWithStartAddress)
    {
        nn::os::FreeMemoryBlock(reinterpret_cast<uintptr_t>(mStart), (mSize + 0x1FFFFF) & 0xFFFFFFFFFFE00000LL);
    }
    mInitWithStartAddress = false;
    mStart = nullptr;
    mSize = 0;
}

}  // namespace sead
