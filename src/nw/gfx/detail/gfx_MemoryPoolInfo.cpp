#include <nn/gfx/gfx_MemoryPoolInfo.h>
namespace nn::gfx {
void MemoryPoolInfo::SetDefault() {
    memoryPoolProperty = 0x22;
    memorySize = 0;
    pMemory = nullptr;
}
}
