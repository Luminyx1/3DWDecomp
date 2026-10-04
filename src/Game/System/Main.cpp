#include "System/Main.hpp"
#include "System/Application.hpp"
#include "al/Library/Memory/HeapUtil.hpp"
#include <nn/init.h>
#include <nn/nn.h>
#include <nn/os.h>

/**
 * @brief SDK startup hook: sizes the OS heap and sets up the SDK allocator before main runs.
 */
extern "C" void nninitStartup() {
    nn::os::MemoryInfo info;
    nn::os::QueryMemoryInfo(&info);
    nn::os::SetMemoryHeapSize((info.totalAvailableMemorySize - info.totalUsedMemorySize) &
                              ~0x1FFFFFull);

    const u64 allocatorSize = 0x2800000;
    u64 allocatorHeap;
    nn::os::AllocateMemoryBlock(&allocatorHeap, allocatorSize);
    nn::init::InitializeAllocator(reinterpret_cast<void*>(allocatorHeap), allocatorSize);

    nn::os::QueryMemoryInfo(&info);
    gMaxMemoryBlockSize = info.totalMemoryHeapSize - info.allocatedMemoryHeapSize;
}

/**
 * @brief SDK program entry point; runs the application without host arguments.
 */
extern "C" void nnMain() { AppMain(0, nullptr); }

namespace rc {

/**
 * @brief Look up the heap reserved for network use.
 * @return The heap registered under the name "NetworkHeap".
 */
sead::Heap* findNetworkHeap() { return al::findNamedHeap("NetworkHeap"); }

}  // namespace rc
