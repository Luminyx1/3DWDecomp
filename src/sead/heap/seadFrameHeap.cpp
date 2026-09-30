#include <heap/seadFrameHeap.h>

#include <atomic>

#include <heap/seadHeapMgr.h>
#include <prim/seadFormatPrint.h>
#include <prim/seadSafeString.h>
#include <prim/seadScopedLock.h>
#include <stream/seadStream.h>

namespace sead
{
namespace
{
inline uintptr_t alignUp(uintptr_t value, u32 alignment)
{
    u32 mask = alignment - 1;
    return (value + mask) & ~uintptr_t(mask);
}

inline uintptr_t alignDown(uintptr_t value, u32 alignment)
{
    u32 mask = alignment - 1;
    return value & ~uintptr_t(mask);
}

inline void notifyAllocFailed(HeapMgr* pMgr, Heap* pHeap, size_t size, s32 alignment,
                              size_t allocSize, s32 allocAlignment)
{
    if (!pMgr)
    {
        return;
    }

    HeapMgr::IAllocFailedCallback* callback = pMgr->getAllocFailedCallback();

    if (!callback)
    {
        return;
    }

    HeapMgr::AllocFailedCallbackArg arg;
    arg.heap = pHeap;
    arg.request_size = size;
    arg.request_alignment = alignment;
    arg.alloc_size = allocSize;
    arg.alloc_alignment = allocAlignment;
    callback->invoke(&arg);
}
}  // namespace

/**
 * Constructs the heap with an empty state.
 * @param rName Heap name.
 * @param pParent Parent heap.
 * @param pAddress Start of the managed memory.
 * @param size Size of the managed memory.
 * @param direction Allocation direction.
 * @param enableLock Whether the heap lock is enabled.
 */
FrameHeap::FrameHeap(const SafeString& rName, Heap* pParent, void* pAddress, size_t size,
                     HeapDirection direction, bool enableLock)
    : Heap(rName, pParent, pAddress, size, direction, enableLock), mState{nullptr, nullptr}
{
}

/**
 * Destroys the heap and everything allocated from it.
 */
FrameHeap::~FrameHeap()
{
    destruct_();
}

/**
 * Creates a frame heap inside a parent heap.
 * @param size Heap size, or 0 for the largest allocatable size.
 * @param rName Heap name.
 * @param pParent Parent heap, or nullptr for the current heap.
 * @param alignment Alignment of the heap memory.
 * @param direction Allocation direction.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr on failure.
 */
FrameHeap* FrameHeap::create(size_t size, const SafeString& rName, Heap* pParent, s32 alignment,
                             HeapDirection direction, bool enableLock)
{
    return tryCreate(size, rName, pParent, alignment, direction, enableLock);
}

/**
 * Tries to allocate memory from a parent heap and build a frame heap in it.
 * @param size Heap size, or 0 for the largest allocatable size.
 * @param rName Heap name.
 * @param pParent Parent heap, or nullptr for the current heap.
 * @param alignment Alignment of the heap memory.
 * @param direction Allocation direction.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr on failure.
 */
FrameHeap* FrameHeap::tryCreate(size_t size, const SafeString& rName, Heap* pParent,
                                s32 alignment, HeapDirection direction, bool enableLock)
{
    if (!pParent)
    {
        pParent = HeapMgr::instance()->getCurrentHeap();

        if (!pParent)
        {
            return nullptr;
        }
    }

    if (size == 0)
    {
        size = pParent->getMaxAllocatableSize(alignment) & ~size_t(7);
    }
    else
    {
        size = (size + 7) & ~size_t(7);
    }

    if (size < sizeof(FrameHeap))
    {
        return nullptr;
    }

    s32 absAlignment = alignment < 0 ? -alignment : alignment;

    if (((absAlignment + 0x7fffffff) & absAlignment) != 0)
    {
        return nullptr;
    }

    void* memory = pParent->tryAlloc(size, direction * alignment);

    if (!memory)
    {
        return nullptr;
    }

    if (pParent->mDirection == cHeapDirection_Reverse)
    {
        direction = static_cast<HeapDirection>(-direction);
    }

    FrameHeap* heap;

    if (direction == cHeapDirection_Forward)
    {
        heap = new (memory) FrameHeap(rName, pParent, memory, size, direction, enableLock);
    }
    else
    {
        void* heapAddress = PtrUtil::addOffset(memory, size - sizeof(FrameHeap));
        heap = new (heapAddress) FrameHeap(rName, pParent, memory, size, direction, enableLock);
    }

    heap->initialize_();
    pParent->pushBackChild_(heap);
    return heap;
}

/**
 * Resets the head and tail pointers to the ends of the allocatable area.
 */
void FrameHeap::initialize_()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    mState.mHeadPtr = getAreaStart_();
    mState.mTailPtr = getAreaEnd_();
}

/**
 * Returns the start of the allocatable area.
 * @return First byte after the heap object in a forward heap, otherwise the heap start.
 */
void* FrameHeap::getAreaStart_() const
{
    if (mDirection == cHeapDirection_Forward)
    {
        return PtrUtil::addOffset(mStart, sizeof(FrameHeap));
    }

    return mStart;
}

/**
 * Returns the end of the allocatable area.
 * @return Heap end in a forward heap, otherwise the start of the heap object.
 */
void* FrameHeap::getAreaEnd_() const
{
    size_t size = mDirection == cHeapDirection_Forward ? mSize : mSize - sizeof(FrameHeap);
    return reinterpret_cast<void*>(size + uintptr_t(mStart));
}

/**
 * Returns the bookkeeping overhead of a heap.
 * @param alignment Alignment padding to include.
 * @return Size of the heap object plus the padding.
 */
size_t FrameHeap::getManagementAreaSize(s32 alignment)
{
    return sizeof(FrameHeap) + alignment;
}

/**
 * Destroys the heap and returns its memory to the parent.
 */
void FrameHeap::destroy()
{
    Heap* parent = mParent;
    void* start = mStart;

    this->~FrameHeap();

    if (parent && parent->isFreeable())
    {
        parent->free(start);
    }
}

/**
 * Returns the heap size.
 * @return Size of the managed memory.
 */
size_t FrameHeap::getSize() const
{
    return mSize;
}

/**
 * Checks whether a pointer lies inside the allocatable area.
 * @param pPtr Pointer to check.
 * @return True if the pointer is inside the area.
 */
bool FrameHeap::isInclude(const void* pPtr) const
{
    const void* start = getAreaStart_();
    const void* end = getAreaEnd_();
    return start <= pPtr && pPtr < end;
}

/**
 * Frees every allocation and resets the head and tail pointers.
 */
void FrameHeap::freeAll()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    dispose_(nullptr, nullptr);
    mState.mHeadPtr = getAreaStart_();
    mState.mTailPtr = getAreaEnd_();
}

/**
 * Shrinks the heap to its used part and returns the rest to the parent.
 * @return New heap size.
 */
size_t FrameHeap::adjust()
{
    if (!mParent)
    {
        return mSize;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    Heap* parent = mParent;

    if (parent->isLockEnabled())
    {
        parent->mCS.lock();
    }

    size_t size;

    if (mDirection == cHeapDirection_Forward)
    {
        if (mState.mTailPtr == PtrUtil::addOffset(mStart, mSize))
        {
            size = adjustBack_();
        }
        else
        {
            size = mSize;
        }
    }
    else
    {
        if (mState.mHeadPtr == mStart)
        {
            size = adjustFront_();
        }
        else
        {
            size = mSize;
        }
    }

    if (parent->isLockEnabled())
    {
        parent->mCS.unlock();
    }

    return size;
}

/**
 * Releases the unused tail of the heap back to the parent heap.
 * @return New heap size.
 */
size_t FrameHeap::adjustBack_()
{
    size_t newSize = uintptr_t(mState.mHeadPtr) - getStartAddress();

    if (!mParent->resizeBack(mStart, newSize))
    {
        return mSize;
    }

    mState.mTailPtr = mState.mHeadPtr;
    mSize = newSize;
    return newSize;
}

/**
 * Releases the unused head of the heap back to the parent heap.
 * @return New heap size.
 */
size_t FrameHeap::adjustFront_()
{
    size_t newSize = getEndAddress() - uintptr_t(mState.mTailPtr);

    if (!mParent->resizeFront(mStart, newSize))
    {
        return mSize;
    }

    mSize = newSize;
    std::atomic_thread_fence(std::memory_order_seq_cst);
    mState.mHeadPtr = mState.mTailPtr;
    mStart = mState.mHeadPtr;
    return newSize;
}

/**
 * Allocates memory from the head or, for negative alignments, the tail.
 * @param size Requested size.
 * @param alignment Requested alignment; negative allocates from the tail.
 * @return The allocated memory, or nullptr.
 */
void* FrameHeap::tryAlloc(size_t size, s32 alignment)
{
    HeapMgr* mgr = HeapMgr::instance();
    size_t allocSize = size > 8 ? size : 8;
    allocSize = (allocSize + 7) & ~size_t(7);

    s32 absAlignment = alignment < 0 ? -alignment : alignment;

    if (((absAlignment + 0x7fffffff) & absAlignment) != 0)
    {
        notifyAllocFailed(mgr, this, size, alignment, allocSize, alignment);
        return nullptr;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    s32 allocAlignment = mDirection * alignment;

    if (allocAlignment >= 0)
    {
        void* ptr = reinterpret_cast<void*>(alignUp(uintptr_t(mState.mHeadPtr), allocAlignment));
        void* newHead = PtrUtil::addOffset(ptr, allocSize);

        if (newHead < ptr || mState.mTailPtr < newHead)
        {
            notifyAllocFailed(mgr, this, size, alignment, allocSize, allocAlignment);
            return nullptr;
        }

        mState.mHeadPtr = newHead;
        return ptr;
    }

    allocAlignment = -allocAlignment;
    void* tail = mState.mTailPtr;
    void* newTail = PtrUtil::addOffset(tail, -allocSize);

    if (tail < newTail)
    {
        notifyAllocFailed(mgr, this, size, alignment, allocSize, allocAlignment);
        return nullptr;
    }

    uintptr_t ptr = alignDown(uintptr_t(newTail), allocAlignment);

    if (uintptr_t(mState.mHeadPtr) > ptr)
    {
        notifyAllocFailed(mgr, this, size, alignment, allocSize, allocAlignment);
        return nullptr;
    }

    mState.mTailPtr = reinterpret_cast<void*>(ptr);
    return reinterpret_cast<void*>(ptr);
}

/**
 * Does nothing; frame heap allocations cannot be freed individually.
 * @param pPtr Ignored.
 */
void FrameHeap::free(void* pPtr) {}

/**
 * Does nothing; frame heap allocations cannot be resized.
 * @param pPtr Ignored.
 * @param size Ignored.
 * @return Always nullptr.
 */
void* FrameHeap::resizeFront(void* pPtr, size_t size)
{
    return nullptr;
}

/**
 * Does nothing; frame heap allocations cannot be resized.
 * @param pPtr Ignored.
 * @param size Ignored.
 * @return Always nullptr.
 */
void* FrameHeap::resizeBack(void* pPtr, size_t size)
{
    return nullptr;
}

/**
 * Frees every allocation made from the head side.
 */
void FrameHeap::freeHead()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    if (mDirection == cHeapDirection_Forward)
    {
        dispose_(getAreaStart_(), mState.mHeadPtr);
        mState.mHeadPtr = getAreaStart_();
    }
    else
    {
        dispose_(mState.mTailPtr, PtrUtil::addOffset(mStart, mSize - sizeof(FrameHeap)));
        mState.mTailPtr = getAreaEnd_();
    }
}

/**
 * Frees every allocation made from the tail side.
 */
void FrameHeap::freeTail()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    if (mDirection == cHeapDirection_Forward)
    {
        dispose_(mState.mTailPtr, PtrUtil::addOffset(mStart, mSize));
        mState.mTailPtr = getAreaEnd_();
    }
    else
    {
        dispose_(mStart, mState.mHeadPtr);
        mState.mHeadPtr = getAreaStart_();
    }
}

/**
 * Rolls the head and tail pointers back to a saved state.
 * @param rState State saved earlier.
 */
void FrameHeap::restoreState(const State& rState)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    if (rState.mHeadPtr && rState.mHeadPtr != mState.mHeadPtr && isInclude(rState.mHeadPtr) &&
        rState.mHeadPtr <= mState.mHeadPtr)
    {
        dispose_(rState.mHeadPtr, mState.mHeadPtr);
        mState.mHeadPtr = rState.mHeadPtr;
    }

    if (rState.mTailPtr && rState.mTailPtr != mState.mTailPtr && isInclude(rState.mTailPtr) &&
        rState.mTailPtr >= mState.mTailPtr)
    {
        dispose_(mState.mTailPtr, rState.mTailPtr);
        mState.mTailPtr = rState.mTailPtr;
    }
}

/**
 * Returns the heap start address.
 * @return Start of the managed memory.
 */
uintptr_t FrameHeap::getStartAddress() const
{
    return uintptr_t(mStart);
}

/**
 * Returns the heap end address.
 * @return End of the managed memory.
 */
uintptr_t FrameHeap::getEndAddress() const
{
    return uintptr_t(mStart) + mSize;
}

/**
 * Returns the space between the head and tail pointers.
 * @return Free size in bytes.
 */
size_t FrameHeap::getFreeSize() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    return uintptr_t(mState.mTailPtr) - uintptr_t(mState.mHeadPtr);
}

/**
 * Returns the largest block that can be allocated with an alignment.
 * @param alignment Requested alignment.
 * @return Allocatable size, or 0 if the alignment is invalid or nothing fits.
 */
size_t FrameHeap::getMaxAllocatableSize(int alignment) const
{
    s32 absAlignment = alignment < 0 ? -alignment : alignment;
    u32 mask = absAlignment - 1;

    if ((mask & absAlignment) != 0)
    {
        return 0;
    }

    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    void* head = reinterpret_cast<void*>((uintptr_t(mState.mHeadPtr) + mask) & ~uintptr_t(mask));

    if (mState.mTailPtr < head)
    {
        return 0;
    }

    return uintptr_t(mState.mTailPtr) - uintptr_t(head);
}

/**
 * Writes the heap description as YAML.
 * @param rStream Output stream.
 * @param indent Indentation width.
 */
void FrameHeap::dumpYAML(WriteStream& rStream, int indent) const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());

    Heap::dumpYAML(rStream, indent);

    FixedSafeString<128> str("");

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  heap_type: FrameHeap\n");
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  head_ptr: 0x%016llX\n", mState.mHeadPtr);
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  tail_ptr: 0x%016llX\n", mState.mTailPtr);
    rStream.writeDecorationText(str);
}

/**
 * Prints a human-readable description of a frame heap.
 * @param rHeap Heap to print.
 * @param pOutput Output target.
 */
template <>
void PrintFormatter::out<FrameHeap>(const FrameHeap& rHeap, const char*, PrintOutput* pOutput)
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&rHeap.mCS),
                                                rHeap.isLockEnabled());

    PrintFormatter::out<Heap>(rHeap, nullptr, pOutput);

    FixedSafeString<128> str;
    OutImpl<char, SafeStringBase>::out("          HeapType: FrameHeap\n", nullptr, pOutput);

    str.format("           HeadPtr: 0x%016llX\n", rHeap.mState.mHeadPtr);
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format("           TailPtr: 0x%016llX\n", rHeap.mState.mTailPtr);
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    OutImpl<char, SafeStringBase>::out("==================================================\n",
                                       nullptr, pOutput);
}

/**
 * Dumps the heap (no output in release builds).
 */
void FrameHeap::dump() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
}

/**
 * Forwards host I/O information generation to Heap.
 * @param pContext Host I/O context.
 */
void FrameHeap::genInformation_(hostio::Context* pContext)
{
    Heap::genInformation_(pContext);
}

/**
 * Checks whether nothing is allocated.
 * @return True if the head and tail pointers are at the ends of the area.
 */
bool FrameHeap::isEmpty() const
{
    return mState.mHeadPtr == getAreaStart_() && mState.mTailPtr == getAreaEnd_();
}

/**
 * Reports that individual allocations cannot be freed.
 * @return Always false.
 */
bool FrameHeap::isFreeable() const
{
    return false;
}

/**
 * Reports that allocations cannot be resized.
 * @return Always false.
 */
bool FrameHeap::isResizable() const
{
    return false;
}

/**
 * Reports that the heap can be adjusted.
 * @return Always true.
 */
bool FrameHeap::isAdjustable() const
{
    return true;
}
}  // namespace sead
