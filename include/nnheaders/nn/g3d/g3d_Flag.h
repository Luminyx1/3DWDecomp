#pragma once
#include <nn/types.h>

namespace nn::g3d {
class MaterialObj;
class ShadingModelObj;
} // namespace nn::g3d
namespace nn::g3d::detail {
class FlagSet {
  public:
    /**
     * @brief Construct an empty flag set with no backing buffers.
     */
    FlagSet()
        : mPending(nullptr), mBufferFlags(nullptr), mBufferCount(0), mWordCount(0), mFlagCount(0), mFlags(0),
          mDirtyBuffers(0) {}
    void Initialize(int flagCount, int bufferCount, void* buffer, size_t bufferSize);
    void Caclulate();

  private:
    friend class nn::g3d::MaterialObj;
    friend class nn::g3d::ShadingModelObj;
    u32* mPending;
    u32* mBufferFlags;
    int mBufferCount;
    int mWordCount;
    int mFlagCount;
    u8 mFlags;
    u8 mDirtyBuffers;
};
static_assert(sizeof(FlagSet) == 0x20, "FlagSet size");
} // namespace nn::g3d::detail
