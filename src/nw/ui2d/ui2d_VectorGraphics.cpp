#include <nn/ui2d/ui2d_VectorGraphics.h>
#include <nn/ui2d/ui2d_Layout.h>
namespace nn::ui2d::detail {
ReservedVectorGraphicsSceneMemory::ReservedVectorGraphicsSceneMemory() : mMemory(nullptr), mReservedSize(0), mNext(nullptr) {}
ReservedVectorGraphicsSceneMemory::~ReservedVectorGraphicsSceneMemory() = default;
// size reserves the arena in bytes; zero leaves the current arena unchanged.
void ReservedVectorGraphicsSceneMemory::Initialize(size_t size) {
    if (size) { mReservedSize = size; mMemory = static_cast<char*>(Layout::AllocateMemory(size)); mNext = mMemory; }
}

void ReservedVectorGraphicsSceneMemory::Finalize() {
    if (mMemory != nullptr) { Layout::FreeMemory(mMemory); mMemory = nullptr; mReservedSize = 0; mNext = nullptr; }
}

size_t ReservedVectorGraphicsSceneMemory::GetReservedSize() const { return mReservedSize; }
size_t ReservedVectorGraphicsSceneMemory::GetAllocatedSize() const { return mNext - mMemory; }
// size consumes that many bytes from the pre-sized scene arena.
void* ReservedVectorGraphicsSceneMemory::Allocate(size_t size) { char* result = mNext; mNext += size; return result; }
// size is the requested byte count; alignment is a power-of-two byte alignment.
void* ReservedVectorGraphicsSceneMemory::Allocate(size_t size, size_t alignment) {
    char* result = reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(mNext) + alignment - 1) & -alignment);
    mNext = result + size;
    return result;
}
}
