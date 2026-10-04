#include <nn/atk/atkfnd_FrameHeapImpl.h>
#include <new>

namespace nn::atk::detail::fnd {
namespace {
/**
 * @brief Align an address upward using the heap's signed alignment arithmetic.
 * @param pAddress Address to align.
 * @param alignment Positive power-of-two alignment representable in a signed int.
 * @return First aligned address at or after pAddress.
 */
inline u8* AlignHead(u8* pAddress, int alignment) {
    return reinterpret_cast<u8*>((reinterpret_cast<uintptr_t>(pAddress) + (alignment - 1)) &
                                 static_cast<intptr_t>(-alignment));
}

/**
 * @brief Align an address downward using the heap's signed alignment arithmetic.
 * @param address Address to align.
 * @param alignment Positive power-of-two alignment representable in a signed int.
 * @return Last aligned address at or before address.
 */
inline uintptr_t AlignTail(uintptr_t address, int alignment) {
    return address & static_cast<intptr_t>(-alignment);
}

/**
 * @brief Calculate a range size using the heap's 32-bit byte count.
 * @param pEnd Exclusive end of the range.
 * @param pStart Inclusive start of the range.
 * @return Difference between the addresses, truncated to 32 bits.
 */
inline u32 RangeSize(const void* pEnd, const void* pStart) {
    return reinterpret_cast<uintptr_t>(pEnd) - reinterpret_cast<uintptr_t>(pStart);
}

/**
 * @brief Round an allocation size to four bytes, including the minimum allocation.
 * @param size Requested size in bytes; zero requests four bytes.
 * @return Rounded allocation size.
 */
inline size_t AllocationSize(size_t size) { return size == 0 ? 4 : (size + 3) & ~size_t(3); }
} // namespace

/**
 * @brief Construct and register a frame heap in caller-provided memory.
 * @param pMemory Writable storage for the heap header and allocations.
 * @param size Available storage size; the aligned range must accommodate the header.
 * @param flags Heap options passed to HeapBase::Initialize.
 * @return Constructed heap, or null when the aligned range is too small.
 */
FrameHeapImpl* FrameHeapImpl::Create(void* pMemory, size_t size, u16 flags) {
    auto* pEnd = reinterpret_cast<u8*>(AlignTail(reinterpret_cast<uintptr_t>(pMemory) + size, 4));
    auto* pStart = AlignHead(static_cast<u8*>(pMemory), 4);
    if (pStart > pEnd || static_cast<size_t>(pEnd - pStart) < sizeof(FrameHeapImpl)) {
        return nullptr;
    }
    auto* pHeap = new (pStart) FrameHeapImpl;
    pHeap->Initialize(0x46524d48, pStart + sizeof(FrameHeapImpl), pEnd, flags);
    pHeap->mHead = pHeap->GetStart();
    pHeap->mTail = pHeap->GetEnd();
    pHeap->mState = nullptr;
    return pHeap;
}

/** @brief Unregister this heap. @return Address of the caller-owned heap header. */
void* FrameHeapImpl::Destroy() {
    Finalize();
    return this;
}

/**
 * @brief Allocate from either end of the frame heap under its lock.
 * @param size Requested byte count; zero is rounded to four bytes.
 * @param alignment Signed power of two; positive allocates from the head, negative from the tail.
 * @return Aligned allocation, or null when insufficient space remains.
 */
void* FrameHeapImpl::Alloc(size_t size, int alignment) {
    size = AllocationSize(size);
    LockHeap();
    void* pResult = alignment >= 0 ? AllocFromHead(size, alignment) : AllocFromTail(size, -alignment);
    UnlockHeap();
    return pResult;
}

/**
 * @brief Allocate bytes from the head without locking or rounding the size.
 * @param size Number of bytes to reserve.
 * @param alignment Positive power-of-two address alignment.
 * @return Aligned allocation, or null when it would overlap tail allocations.
 */
void* FrameHeapImpl::AllocFromHead(size_t size, int alignment) {
    u8* pBlock = AlignHead(mHead, alignment);
    u8* pEnd = pBlock + size;
    if (pEnd > mTail) {
        return nullptr;
    }
    FillAllocMemory(mHead, RangeSize(pEnd, mHead));
    mHead = pEnd;
    return pBlock;
}

/**
 * @brief Allocate bytes from the tail without locking or rounding the size.
 * @param size Byte count interpreted with the original signed 32-bit offset arithmetic.
 * @param alignment Positive power-of-two address alignment.
 * @return Aligned allocation, or null when it would overlap head allocations.
 */
void* FrameHeapImpl::AllocFromTail(size_t size, int alignment) {
    uintptr_t head = reinterpret_cast<uintptr_t>(mHead);
    uintptr_t address = AlignTail(reinterpret_cast<uintptr_t>(mTail) + -static_cast<int>(size), alignment);
    if (address < head) {
        return nullptr;
    }
    auto* pBlock = reinterpret_cast<u8*>(address);
    FillAllocMemory(pBlock, RangeSize(mTail, pBlock));
    mTail = pBlock;
    return pBlock;
}

/**
 * @brief Resize the most recent head allocation.
 * @param pBlock Start of the most recent head allocation.
 * @param size Requested new byte count, rounded to four bytes with a four-byte minimum.
 * @return Rounded size, or zero when the requested growth exceeds the tail cursor.
 */
size_t FrameHeapImpl::ResizeForMBlock(void* pBlock, size_t size) {
    size = AllocationSize(size);
    LockHeap();
    ptrdiff_t oldSize = mHead - static_cast<u8*>(pBlock);
    if (oldSize != static_cast<int>(size)) {
        u8* pEnd = reinterpret_cast<u8*>(size + reinterpret_cast<uintptr_t>(pBlock));
        if (size > static_cast<u32>(oldSize)) {
            if (reinterpret_cast<uintptr_t>(mTail) < reinterpret_cast<uintptr_t>(pEnd)) {
                size = 0;
            } else {
                FillAllocMemory(mHead, size - oldSize);
            }
        } else {
            FillFreeMemory(pEnd, oldSize - size);
        }
        // The original updates the head even when growth fails.
        mHead = pEnd;
    }
    UnlockHeap();
    return size;
}

/**
 * @brief Calculate the remaining allocation capacity for an alignment.
 * @param alignment Signed power-of-two alignment; only its magnitude is used.
 * @return Usable bytes after aligning the head, or zero if it exceeds the tail.
 */
size_t FrameHeapImpl::GetAllocatableSize(int alignment) {
    if (alignment < 0) {
        alignment = -alignment;
    }
    u8* pHead = AlignHead(mHead, alignment);
    u8* pTail = mTail < pHead ? pHead : mTail;
    return pTail - pHead;
}

/**
 * @brief Free allocations from the selected ends under the heap lock.
 * @param mode Bit 0 frees the head, bit 1 frees the tail; other bits are ignored.
 */
void FrameHeapImpl::Free(int mode) {
    LockHeap();
    if ((mode & 1) != 0) {
        FreeHead();
    }
    if ((mode & 2) != 0) {
        FreeTail();
    }
    UnlockHeap();
}

/** @brief Free all head allocations and discard their saved checkpoints. */
void FrameHeapImpl::FreeHead() {
    FillFreeMemory(GetStart(), RangeSize(mHead, GetStart()));
    mHead = GetStart();
    mState = nullptr;
}

/** @brief Free all tail allocations and reset saved checkpoints' tail positions. */
void FrameHeapImpl::FreeTail() {
    FillFreeMemory(mTail, RangeSize(GetEnd(), mTail));
    for (State* pState = mState; pState != nullptr; pState = pState->pPrevious) {
        pState->pTail = GetEnd();
    }
    mTail = GetEnd();
}

/**
 * @brief Save both allocation cursors in a checkpoint allocated from the head.
 * @param tag Application-supplied checkpoint identifier.
 * @return Whether enough memory was available for the checkpoint.
 */
bool FrameHeapImpl::RecordState(u32 tag) {
    LockHeap();
    u8* pHead = mHead;
    auto* pState = static_cast<State*>(AllocFromHead(sizeof(State), 4));
    bool result = pState != nullptr;
    if (pState != nullptr) {
        new (pState) State;
        pState->pHead = pHead;
        pState->tag = tag;
        pState->pTail = mTail;
        pState->pPrevious = mState;
        mState = pState;
    }
    UnlockHeap();
    return result;
}

/**
 * @brief Restore a saved checkpoint and discard it and any newer checkpoints.
 * @param tag Checkpoint identifier; zero selects the most recent checkpoint.
 * @return Whether a matching checkpoint existed.
 */
bool FrameHeapImpl::FreeByState(u32 tag) {
    LockHeap();
    State* pState = mState;
    if (tag != 0) {
        while (pState != nullptr && pState->tag != tag) {
            pState = pState->pPrevious;
        }
    }
    bool result = pState != nullptr;
    if (pState != nullptr) {
        u8* pHead = mHead;
        mHead = pState->pHead;
        u8* pTail = mTail;
        mTail = pState->pTail;
        mState = pState->pPrevious;
        FillFreeMemory(mHead, RangeSize(pHead, mHead));
        FillFreeMemory(pTail, RangeSize(mTail, pTail));
    }
    UnlockHeap();
    return result;
}

/**
 * @brief Shrink the managed range to the head cursor when no tail allocation remains.
 * @return Total heap size including its header, or zero if the tail prevents shrinking.
 */
u32 FrameHeapImpl::Adjust() {
    LockHeap();
    u32 size = 0;
    intptr_t tail = reinterpret_cast<intptr_t>(mTail);
    intptr_t end = reinterpret_cast<intptr_t>(GetEnd());
    if (end <= tail) {
        mTail = mHead;
        SetEnd(mHead);
        size = RangeSize(mHead, this);
    }
    UnlockHeap();
    return size;
}
} // namespace nn::atk::detail::fnd
