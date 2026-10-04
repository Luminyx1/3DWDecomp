#pragma once
#include <nn/atk/atkfnd_HeapBase.h>

namespace nn::atk::detail::fnd {
class FrameHeapImpl : public HeapBase {
  public:
    static FrameHeapImpl* Create(void* pMemory, size_t size, u16 flags);
    void* Destroy();
    void* Alloc(size_t size, int alignment);
    void* AllocFromHead(size_t size, int alignment);
    void* AllocFromTail(size_t size, int alignment);
    size_t ResizeForMBlock(void* pBlock, size_t size);
    size_t GetAllocatableSize(int alignment);
    void Free(int mode);
    void FreeHead();
    void FreeTail();
    bool RecordState(u32 tag);
    bool FreeByState(u32 tag);
    u32 Adjust();

  private:
    struct State {
        /** @brief Initialize an empty allocation checkpoint. */
        State() : tag(0), pHead(nullptr), pTail(nullptr), pPrevious(nullptr) {}
        u32 tag;
        u8* pHead;
        u8* pTail;
        State* pPrevious;
    };
    u8* mHead;
    u8* mTail;
    State* mState;
};
static_assert(sizeof(FrameHeapImpl) == 0x58, "Frame heap header size");
} // namespace nn::atk::detail::fnd
