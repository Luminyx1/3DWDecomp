#include <nn/atk/atkfnd_HeapBase.h>
#include <cstring>

namespace nn::atk::detail::fnd {
static u32 sFillValues[3] = {0xc3c3c3c3, 0xf3f3f3f3, 0xd3d3d3d3};
static HeapBase::HeapList sHeapList;

/**
 * @brief Find the list to which a heap belongs.
 * @param pHeap Heap header whose address identifies its enclosing heap.
 * @return Enclosing heap's child list, or the root list when no heap contains the header.
 */
HeapBase::HeapList* HeapBase::FindListContainHeap(HeapBase* pHeap) {
    HeapBase* pParent = FindContainHeap(&sHeapList, pHeap);
    return pParent != nullptr ? &pParent->mChildren : &sHeapList;
}

/**
 * @brief Find the deepest heap containing an address within a hierarchy.
 * @param pList Non-null list of heaps to search.
 * @param pAddress Address to locate; it is not dereferenced.
 * @return Deepest enclosing heap, or null when the address is outside the hierarchy.
 */
HeapBase* HeapBase::FindContainHeap(HeapList* pList, const void* pAddress) {
    for (auto it = pList->begin(); it != pList->end();) {
        HeapBase* pHeap = &*it++;
        if (pHeap->Contains(pAddress)) {
            HeapBase* pChild = FindContainHeap(&pHeap->mChildren, pAddress);
            return pChild != nullptr ? pChild : pHeap;
        }
    }
    return nullptr;
}

/**
 * @brief Search all registered heaps for an address.
 * @param pAddress Address to locate; it is not dereferenced.
 * @return Deepest enclosing heap, or null when no registered heap contains the address.
 */
HeapBase* HeapBase::FindContainHeap(const void* pAddress) { return FindContainHeap(&sHeapList, pAddress); }

/**
 * @brief Find a heap's parent in the registered hierarchy.
 * @param pHeap Heap whose parent is requested; its header must lie in its parent's range.
 * @return Enclosing heap, or null when the heap has no registered parent.
 */
HeapBase* HeapBase::FindParentHeap(const HeapBase* pHeap) {
    for (auto it = sHeapList.begin(); it != sHeapList.end();) {
        HeapBase* pRoot = &*it++;
        if (pRoot->Contains(pHeap)) {
            return FindContainHeap(&pRoot->mChildren, pRoot);
        }
    }
    return nullptr;
}

/**
 * @brief Replace a diagnostic memory-fill pattern.
 * @param type Pattern slot; must be NoUse, Alloc, or Free.
 * @param value New pattern whose low byte is passed to memset.
 * @return Previous pattern for the selected slot.
 */
u32 HeapBase::SetFillValue(FillType type, u32 value) {
    u32 previous = sFillValues[type];
    sFillValues[type] = value;
    return previous;
}

/**
 * @brief Read a diagnostic memory-fill pattern.
 * @param type Pattern slot; must be NoUse, Alloc, or Free.
 * @return Current pattern for the selected slot.
 */
u32 HeapBase::GetFillValue(FillType type) { return sFillValues[type]; }

/**
 * @brief Identify this heap from its format signature.
 * @return Expandable, frame, unit, or unknown heap type.
 */
HeapBase::HeapType HeapBase::GetHeapType() {
    switch (mSignature) {
    case 0x45585048:
        return HeapType_Exp;
    case 0x46524d48:
        return HeapType_Frame;
    case 0x554e5448:
        return HeapType_Unit;
    default:
        return HeapType_Unknown;
    }
}

/**
 * @brief Register a heap and optionally fill its unused memory.
 * @param signature Four-byte heap format signature.
 * @param pStart Inclusive start of the managed memory range.
 * @param pEnd Exclusive end; the range must fit in an unsigned 32-bit byte count.
 * @param flags Options in the low byte; bit 1 enables diagnostic fill patterns.
 */
void HeapBase::Initialize(u32 signature, void* pStart, void* pEnd, u16 flags) {
    mSignature = signature;
    mStart = pStart;
    mEnd = pEnd;
    mFlags = static_cast<u8>(flags);
    if ((flags & 2) != 0) {
        u32 size = reinterpret_cast<uintptr_t>(pEnd) - reinterpret_cast<uintptr_t>(pStart);
        std::memset(pStart, GetFillValue(FillType_NoUse), size);
    }
    FindListContainHeap(this)->push_back(*this);
}

/**
 * @brief Unregister the heap and invalidate its signature.
 */
void HeapBase::Finalize() {
    HeapList* pList = FindListContainHeap(this);
    pList->erase(pList->iterator_to(*this));
    mSignature = 0;
}

/**
 * @brief Replace the low byte of heap options.
 * @param flags New options; bits above bit 7 are ignored.
 */
void HeapBase::SetOptionFlag(u16 flags) { mOptionFlags = flags; }

/**
 * @brief Read the low byte of heap options.
 * @return Current heap option bits.
 */
u16 HeapBase::GetOptionFlag() { return mOptionFlags; }

/**
 * @brief Fill unused memory when diagnostic filling is enabled.
 * @param pMemory Writable start of the unused range.
 * @param size Number of bytes available at pMemory.
 */
void HeapBase::FillNoUseMemory(void* pMemory, size_t size) {
    if ((mOptionFlags & 2) != 0) {
        std::memset(pMemory, sFillValues[FillType_NoUse], size);
    }
}

/**
 * @brief Initialize allocated memory according to the heap's fill options.
 * @param pMemory Writable allocation to initialize.
 * @param size Allocation size in bytes.
 */
void HeapBase::FillAllocMemory(void* pMemory, size_t size) {
    if ((mFlags & 1) != 0) {
        std::memset(pMemory, 0, size);
    } else if ((mFlags & 2) != 0) {
        std::memset(pMemory, sFillValues[FillType_Alloc], size);
    }
}

/**
 * @brief Fill freed memory when diagnostic filling is enabled.
 * @param pMemory Writable start of the freed range.
 * @param size Number of bytes available at pMemory.
 */
void HeapBase::FillFreeMemory(void* pMemory, size_t size) {
    if ((mOptionFlags & 2) != 0) {
        std::memset(pMemory, sFillValues[FillType_Free], size);
    }
}

/** @brief Enter the heap lock; this build performs no synchronization here. */
void HeapBase::LockHeap() {}
/** @brief Leave the heap lock; this build performs no synchronization here. */
void HeapBase::UnlockHeap() {}
} // namespace nn::atk::detail::fnd
