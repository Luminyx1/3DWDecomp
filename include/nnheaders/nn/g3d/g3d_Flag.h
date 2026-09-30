#pragma once
#include <nn/types.h>

namespace nn::g3d { class MaterialObj; }
namespace nn::g3d::detail {
class FlagSet {
public:
    void Initialize(int flagCount, int bufferCount, void* buffer, size_t bufferSize);
    void Caclulate();
private:
    friend class nn::g3d::MaterialObj;
    u32* mPending;
    u32* mBufferFlags;
    int mBufferCount;
    int mWordCount;
    int mFlagCount;
    u8 mFlags;
    u8 mDirtyBuffers;
};
static_assert(sizeof(FlagSet) == 0x20, "FlagSet size");
}
