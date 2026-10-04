#pragma once
#include <nn/util/util_IntrusiveListBaseNodeTraits.h>

namespace nn::atk::detail::fnd {
class HeapBase : public util::IntrusiveListNode {
  public:
    using HeapList = util::IntrusiveList<HeapBase, util::IntrusiveListBaseNodeTraits<HeapBase, HeapBase>>;
    enum FillType : unsigned { FillType_NoUse, FillType_Alloc, FillType_Free };
    enum HeapType { HeapType_Exp, HeapType_Frame, HeapType_Unit, HeapType_Unknown };
    static HeapList* FindListContainHeap(HeapBase* pHeap);
    static HeapBase* FindContainHeap(HeapList* pList, const void* pAddress);
    static HeapBase* FindContainHeap(const void* pAddress);
    static HeapBase* FindParentHeap(const HeapBase* pHeap);
    u32 SetFillValue(FillType type, u32 value);
    u32 GetFillValue(FillType type);
    HeapType GetHeapType();
    void Initialize(u32 signature, void* pStart, void* pEnd, u16 flags);
    void Finalize();
    void SetOptionFlag(u16 flags);
    u16 GetOptionFlag();
    void FillNoUseMemory(void* pMemory, size_t size);
    void FillAllocMemory(void* pMemory, size_t size);
    void FillFreeMemory(void* pMemory, size_t size);
    void LockHeap();
    void UnlockHeap();

  protected:
    /** @brief Obtain the managed range's start. @return Inclusive start address. */
    u8* GetStart() const { return static_cast<u8*>(mStart); }
    /** @brief Obtain the managed range's end. @return Exclusive end address. */
    u8* GetEnd() const { return static_cast<u8*>(mEnd); }
    /**
     * @brief Update the managed range after a derived heap releases its tail.
     * @param pEnd New exclusive end address within the original range.
     */
    void SetEnd(u8* pEnd) { mEnd = pEnd; }

  private:
    /**
     * @brief Test whether an address lies in this heap's managed range.
     * @param pAddress Address to test; it is not dereferenced.
     * @return Whether the address lies between the inclusive start and exclusive end.
     */
    bool Contains(const void* pAddress) const { return mStart <= pAddress && pAddress < mEnd; }
    void* mStart;
    void* mEnd;
    u32 mSignature;
    HeapList mChildren;
    union {
        u32 mFlags;
        u8 mOptionFlags;
    };
};
static_assert(sizeof(HeapBase) == 0x40, "HeapBase size");
} // namespace nn::atk::detail::fnd
