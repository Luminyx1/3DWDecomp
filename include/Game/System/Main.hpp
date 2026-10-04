#pragma once
#include <basis/seadTypes.h>

namespace sead {
class Heap;
}  // namespace sead

/// Largest memory block still allocatable from the OS heap after startup (set by nninitStartup).
extern u64 gMaxMemoryBlockSize;

namespace rc {
sead::Heap* findNetworkHeap();
}  // namespace rc
