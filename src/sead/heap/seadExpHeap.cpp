#include <heap/seadExpHeap.h>

#include <atomic>
#include <cstring>

#include <heap/seadHeapMgr.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadFormatPrint.h>
#include <prim/seadSafeString.h>
#include <prim/seadScopedLock.h>
#include <stream/seadStream.h>

namespace sead
{
namespace
{
inline bool isPow2(s32 value)
{
    return ((value - 1) & value) == 0;
}

inline uintptr_t alignUp(uintptr_t value, u32 alignment)
{
    u32 mask = alignment - 1;
    return (value + mask) & ~uintptr_t(mask);
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
 * Constructs the heap and sets up its empty free and use lists.
 * @param name Heap name.
 * @param pParent Parent heap.
 * @param pAddress Start of the managed memory.
 * @param size Size of the managed memory.
 * @param direction Allocation direction.
 * @param enableLock Whether the heap lock is enabled.
 */
ExpHeap::ExpHeap(const SafeString& name, Heap* pParent, void* pAddress, size_t size,
                 HeapDirection direction, bool enableLock)
    : Heap(name, pParent, pAddress, size, direction, enableLock), mAllocMode(AllocMode::FirstFit),
      mFindFreeBlockMode(FindFreeBlockMode::Auto)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    mFreeList.initOffset(MemBlock::getOffset());
    mUseList.initOffset(MemBlock::getOffset());
}

/**
 * Destroys the heap and everything allocated from it.
 */
ExpHeap::~ExpHeap()
{
    destruct_();
}

/**
 * Creates an expanded heap inside a parent heap.
 * @param size Heap size, or 0 for the largest allocatable size.
 * @param name Heap name.
 * @param pParent Parent heap, or nullptr for the current heap.
 * @param alignment Alignment of the heap memory.
 * @param direction Allocation direction.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr on failure.
 */
ExpHeap* ExpHeap::create(size_t size, const SafeString& name, Heap* pParent, s32 alignment,
                         HeapDirection direction, bool enableLock)
{
    return tryCreate(size, name, pParent, alignment, direction, enableLock);
}

/**
 * Tries to allocate memory from a parent heap and build an expanded heap in it.
 * @param size Heap size, or 0 for the largest allocatable size.
 * @param name Heap name.
 * @param pParent Parent heap, or nullptr for the current heap.
 * @param alignment Alignment of the heap memory.
 * @param direction Allocation direction.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr on failure.
 */
ExpHeap* ExpHeap::tryCreate(size_t size, const SafeString& name, Heap* pParent, s32 alignment,
                            HeapDirection direction, bool enableLock)
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

    if (size <= sizeof(ExpHeap) + sizeof(MemBlock))
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

    ExpHeap* heap;

    if (direction == cHeapDirection_Forward)
    {
        heap = new (memory) ExpHeap(name, pParent, memory, size, direction, enableLock);
    }
    else
    {
        void* heapAddress = PtrUtil::addOffset(memory, size - sizeof(ExpHeap));
        heap = new (heapAddress) ExpHeap(name, pParent, memory, size, direction, enableLock);
    }

    doCreate(heap, pParent);
    return heap;
}

/**
 * Creates the initial free block and registers the heap with its parent.
 * @param pHeap Heap being created.
 * @param pParent Parent heap, may be nullptr.
 */
void ExpHeap::doCreate(ExpHeap* pHeap, Heap* pParent)
{
    createMaxSizeFreeMemBlock_(pHeap);

    if (pParent)
    {
        pParent->pushBackChild_(pHeap);
    }
}

/**
 * Builds an expanded heap on an existing memory range without a parent.
 * @param pAddress Start of the memory range.
 * @param size Size of the memory range.
 * @param name Heap name.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr if the range is too small.
 */
ExpHeap* ExpHeap::tryCreate(void* pAddress, size_t size, const SafeString& name, bool enableLock)
{
    size &= ~size_t(7);

    if (size <= sizeof(ExpHeap) + sizeof(MemBlock))
    {
        return nullptr;
    }

    auto* heap =
        new (pAddress) ExpHeap(name, nullptr, pAddress, size, cHeapDirection_Forward, enableLock);
    doCreate(heap, nullptr);
    return heap;
}

/**
 * Builds an expanded heap on a memory range owned by a parent heap.
 * @param pAddress Start of the memory range.
 * @param size Size of the memory range.
 * @param name Heap name.
 * @param pParent Heap containing the range.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr on failure.
 */
ExpHeap* ExpHeap::create(void* pAddress, size_t size, const SafeString& name, Heap* pParent,
                         bool enableLock)
{
    return tryCreate(pAddress, size, name, pParent, enableLock);
}

/**
 * Tries to build an expanded heap on a memory range owned by a parent heap.
 * @param pAddress Start of the memory range.
 * @param size Size of the memory range.
 * @param name Heap name.
 * @param pParent Heap containing the range.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr on failure.
 */
ExpHeap* ExpHeap::tryCreate(void* pAddress, size_t size, const SafeString& name, Heap* pParent,
                            bool enableLock)
{
    if (!pParent || !pParent->isInclude(pAddress))
    {
        return nullptr;
    }

    bool isEndIncluded = pParent->isInclude(PtrUtil::addOffset(pAddress, size - 1));

    if ((uintptr_t(pAddress) & 7) != 0 || !isEndIncluded)
    {
        return nullptr;
    }

    if (size <= sizeof(ExpHeap) + sizeof(MemBlock) || (size & 7) != 0)
    {
        return nullptr;
    }

    auto* heap =
        new (pAddress) ExpHeap(name, pParent, pAddress, size, cHeapDirection_Forward, enableLock);
    doCreate(heap, pParent);
    heap->mFlag.setBit(Flag::cEnableDebugFillSystem);
    return heap;
}

/**
 * Returns the bookkeeping overhead of a heap.
 * @param alignment Alignment padding to include.
 * @return Size of the heap object, one block header and the padding.
 */
size_t ExpHeap::getManagementAreaSize(s32 alignment)
{
    return sizeof(ExpHeap) + sizeof(MemBlock) + alignment;
}

/**
 * Turns the whole heap area into a single free block.
 * @param pHeap Heap to initialise.
 */
void ExpHeap::createMaxSizeFreeMemBlock_(ExpHeap* pHeap)
{
    ConditionalScopedLock<CriticalSection> lock(&pHeap->mCS, pHeap->isLockEnabled());

    MemBlock* block;

    if (pHeap->mDirection == cHeapDirection_Forward)
    {
        block = new (static_cast<void*>(pHeap + 1)) MemBlock();
    }
    else
    {
        block = new (pHeap->mStart) MemBlock();
    }

    block->mSize = pHeap->mSize - sizeof(ExpHeap) - sizeof(MemBlock);
    block->mHeapCheckTag = MemBlock::cFreeHeapCheckTag;
    pHeap->mFreeList.pushBack(block);
}

/**
 * Destroys the heap and returns its memory to the parent.
 */
void ExpHeap::destroy()
{
    destroyAndGetAllocatableSize(8);
}

/**
 * Destroys the heap and frees its memory in the parent heap.
 * @param alignment Alignment used to compute the parent's allocatable size.
 * @return Size of the resulting free block in an expanded parent, otherwise 0.
 */
size_t ExpHeap::destroyAndGetAllocatableSize(s32 alignment)
{
    Heap* parent = mParent;
    void* start = mStart;
    BitFlag16 flag = mFlag;

    this->~ExpHeap();

    if (parent && parent->isFreeable() && !flag.isOnBit(Flag::cEnableDebugFillSystem))
    {
        auto* expHeap = DynamicCast<ExpHeap>(parent);

        if (expHeap)
        {
            return expHeap->freeAndGetAllocatableSize(start, alignment);
        }

        parent->free(start);
    }

    return 0;
}

/**
 * Frees a block and reports the size of the free block it ends up in.
 * @param pPtr Memory to free.
 * @param alignment Alignment used to compute the allocatable size.
 * @return Allocatable size of the merged free block, or 0 on failure.
 */
size_t ExpHeap::freeAndGetAllocatableSize(void* pPtr, s32 alignment)
{
    if (!pPtr || !isInclude(pPtr))
    {
        return 0;
    }

    if (mFlag.isOnBit(Flag::cDisposing))
    {
        return 0;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    MemBlock* block = MemBlock::FindManageArea(pPtr);

    if (!block)
    {
        dumpUseList();
        dumpFreeList();
        return 0;
    }

    if (block->mHeapCheckTag != mHeapCheckTag)
    {
        return 0;
    }

    mUseList.erase(block);
    block->mSize = block->mSize + block->mOffset;
    block->mOffset = 0;
    MemBlock* freeBlock = pushToFreeList_(block);

    s32 absAlignment = alignment < 0 ? -alignment : alignment;

    if (absAlignment <= 8)
    {
        return freeBlock->mSize;
    }

    uintptr_t memory = uintptr_t(freeBlock->getMemory());
    uintptr_t alignedMemory = alignUp(memory, absAlignment);
    return memory + freeBlock->mSize - alignedMemory;
}

/**
 * Checks whether a pointer lies inside the heap's allocatable area.
 * @param pPtr Pointer to check.
 * @return True if the pointer is inside the area.
 */
bool ExpHeap::isInclude(const void* pPtr) const
{
    uintptr_t start;
    uintptr_t end;

    if (mDirection == cHeapDirection_Forward)
    {
        start = uintptr_t(mStart) + sizeof(ExpHeap);
        end = uintptr_t(mStart) + mSize;
    }
    else
    {
        start = uintptr_t(mStart);
        end = uintptr_t(mStart) + mSize - sizeof(ExpHeap);
    }

    return start <= uintptr_t(pPtr) && uintptr_t(pPtr) < end;
}

/**
 * Frees every allocation and resets the heap to one free block.
 */
void ExpHeap::freeAll()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    dispose_(nullptr, nullptr);
    mUseList.clear();
    mFreeList.clear();
    createMaxSizeFreeMemBlock_(this);
}

/**
 * Shrinks the heap to its used part and returns the rest to the parent.
 * @return New heap size.
 */
size_t ExpHeap::adjust()
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
        size = adjustBack_();
    }
    else
    {
        size = adjustFront_();
    }

    if (parent->isLockEnabled())
    {
        parent->mCS.unlock();
    }

    return size;
}

/**
 * Releases a trailing free block back to the parent heap.
 * @return New heap size.
 */
size_t ExpHeap::adjustBack_()
{
    MemBlock* block = findLastMemBlockIfFree_();

    if (!block)
    {
        return mSize;
    }

    size_t newSize = uintptr_t(block) - uintptr_t(mStart);
    mFreeList.erase(block);

    if (!mParent->resizeBack(mStart, newSize))
    {
        return mSize;
    }

    mSize = newSize;
    return newSize;
}

/**
 * Releases a leading free block back to the parent heap.
 * @return New heap size.
 */
size_t ExpHeap::adjustFront_()
{
    MemBlock* block = findFirstMemBlockIfFree_();

    if (!block)
    {
        return mSize;
    }

    size_t newSize = mSize - block->getTotalSize();
    mFreeList.erase(block);
    void* newStart = mParent->resizeFront(mStart, newSize);

    if (!newStart)
    {
        return mSize;
    }

    mSize = newSize;
    std::atomic_thread_fence(std::memory_order_seq_cst);
    mStart = newStart;
    return newSize;
}

/**
 * Finds the free block at the end of the heap.
 * @return The last free block if it follows every used block, otherwise nullptr.
 */
MemBlock* ExpHeap::findLastMemBlockIfFree_()
{
    mUseList.sort(compareMemBlockAddr_);
    MemBlock* lastFree = mFreeList.back();
    MemBlock* lastUsed = mUseList.back();

    if (lastFree > lastUsed)
    {
        return lastFree;
    }

    return nullptr;
}

/**
 * Finds the free block at the start of the heap.
 * @return The first free block if it precedes every used block, otherwise nullptr.
 */
MemBlock* ExpHeap::findFirstMemBlockIfFree_()
{
    mUseList.sort(compareMemBlockAddr_);
    MemBlock* firstFree = mFreeList.front();
    MemBlock* firstUsed = mUseList.front();

    if (!firstUsed || firstFree < firstUsed)
    {
        return firstFree;
    }

    return nullptr;
}

/**
 * Allocates memory, reporting failures to the allocation-failed callback.
 * @param size Requested size.
 * @param alignment Requested alignment; negative allocates from the tail.
 * @return The allocated memory, or nullptr.
 */
void* ExpHeap::tryAlloc(size_t size, s32 alignment)
{
    HeapMgr* mgr = HeapMgr::instance();
    size_t allocSize = size > 8 ? size : 8;

    s32 absAlignment = alignment < 0 ? -alignment : alignment;

    if (((absAlignment + 0x7fffffff) & absAlignment) != 0)
    {
        notifyAllocFailed(mgr, this, size, alignment, allocSize, alignment);
        return nullptr;
    }

    allocSize = (allocSize + 7) & ~size_t(7);

    if (allocSize < size)
    {
        notifyAllocFailed(mgr, this, size, alignment, allocSize, alignment);
        return nullptr;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    s32 allocAlignment = mDirection * alignment;
    MemBlock* block;

    if (allocAlignment >= 0)
    {
        if (allocAlignment <= 8)
        {
            block = allocFromHead_(allocSize);
        }
        else
        {
            block = allocFromHead_(allocSize, allocAlignment);
        }
    }
    else
    {
        allocAlignment = -allocAlignment;

        if (allocAlignment <= 8)
        {
            block = allocFromTail_(allocSize);
        }
        else
        {
            block = allocFromTail_(allocSize, allocAlignment);
        }
    }

    if (block)
    {
        block->mHeapCheckTag = mHeapCheckTag;
        return block->getMemory();
    }

    notifyAllocFailed(mgr, this, size, alignment, allocSize, allocAlignment);
    return nullptr;
}

/**
 * Allocates a block from the start of the free list.
 * @param size Aligned allocation size.
 * @return The allocated block, or nullptr.
 */
MemBlock* ExpHeap::allocFromHead_(size_t size)
{
    MemBlock* block = findFreeMemBlockFromHead_(size, static_cast<FindMode>(mAllocMode.mValue));

    if (!block)
    {
        return nullptr;
    }

    size_t blockSize = block->mSize;
    block->mSize = size;
    size_t restSize = blockSize - size;
    MemBlock* next = mFreeList.next(block);
    u16 offset = block->mOffset;

    mFreeList.erase(block);
    pushToUseList_(block);

    if (restSize > sizeof(MemBlock))
    {
        void* pAddress = PtrUtil::addOffset(block, size + offset + sizeof(MemBlock));
        auto* freeBlock = new (pAddress) MemBlock();
        freeBlock->mSize = restSize - sizeof(MemBlock);
        freeBlock->mHeapCheckTag = MemBlock::cFreeHeapCheckTag;

        if (next)
        {
            mFreeList.insertBefore(next, freeBlock);
        }
        else
        {
            mFreeList.pushBack(freeBlock);
        }
    }
    else if (restSize != 0)
    {
        block->mSize = blockSize;
    }

    return block;
}

/**
 * Allocates an aligned block from the start of the free list.
 * @param size Aligned allocation size.
 * @param alignment Memory alignment.
 * @return The allocated block, or nullptr.
 */
MemBlock* ExpHeap::allocFromHead_(size_t size, s32 alignment)
{
    MemBlock* block =
        findFreeMemBlockFromHead_(size, alignment, static_cast<FindMode>(mAllocMode.mValue));
    if (!block)
    {
        return nullptr;
    }

    MemBlock* next = mFreeList.next(block);
    uintptr_t memory = uintptr_t(reinterpret_cast<u8*>(block + 1) + block->mOffset);
    uintptr_t alignedMemory = alignUp(memory, alignment);
    size_t padding = alignedMemory - memory;
    size_t restSize = block->mSize - size;

    if (padding >= 0x10000)
    {
        block->mOffset = 0;
        block->mSize = padding - sizeof(MemBlock);
        block = new (PtrUtil::addOffset(block, padding)) MemBlock();
        block->setOffset(0);
        block->mSize = size;
    }
    else
    {
        block->setOffset(padding);
        block->mSize = size;
        mFreeList.erase(block);
    }

    restSize -= padding;
    pushToUseList_(block);

    if (restSize > sizeof(MemBlock))
    {
        void* pAddress = PtrUtil::addOffset(block, size + block->mOffset + sizeof(MemBlock));
        auto* freeBlock = new (pAddress) MemBlock();
        freeBlock->mSize = restSize - sizeof(MemBlock);
        freeBlock->mHeapCheckTag = MemBlock::cFreeHeapCheckTag;

        if (next)
        {
            mFreeList.insertBefore(next, freeBlock);
        }
        else
        {
            mFreeList.pushBack(freeBlock);
        }
    }
    else if (restSize != 0)
    {
        block->mSize = restSize + size;
    }

    return block;
}

/**
 * Allocates a block from the end of the free list.
 * @param size Aligned allocation size.
 * @return The allocated block, or nullptr.
 */
MemBlock* ExpHeap::allocFromTail_(size_t size)
{
    MemBlock* block = findFreeMemBlockFromTail_(size, static_cast<FindMode>(mAllocMode.mValue));

    if (!block)
    {
        return nullptr;
    }

    size_t restSize = block->mSize - size;

    if (restSize > sizeof(MemBlock))
    {
        block->mSize = restSize - sizeof(MemBlock);
        void* pAddress = PtrUtil::addOffset(block, restSize + block->mOffset);
        block = new (pAddress) MemBlock();
        block->mSize = size;
        pushToUseList_(block);
    }
    else
    {
        mFreeList.erase(block);
        pushToUseList_(block);
    }

    return block;
}

MemBlock* ExpHeap::allocFromTail_(size_t size, s32 alignment)
{
    MemBlock* block =
        findFreeMemBlockFromTail_(size, alignment, static_cast<FindMode>(mAllocMode.mValue));
    if (!block)
    {
        return nullptr;
    }

    u8* memory = reinterpret_cast<u8*>(block + 1) + block->mOffset;
    size_t padding = (uintptr_t(memory) + block->mSize - size) & u32(alignment - 1);
    size_t allocSize = padding + size;
    size_t restSize = block->mSize - allocSize;

    if (restSize > sizeof(MemBlock))
    {
        block->mSize = restSize - sizeof(MemBlock);
        block = new (PtrUtil::addOffset(memory, block->mSize)) MemBlock();
        block->mSize = allocSize;
        pushToUseList_(block);
    }
    else
    {
        block->setOffset(restSize);
        block->mSize = allocSize;
        mFreeList.erase(block);
        pushToUseList_(block);
    }

    return block;
}

/**
 * Searches the free list forwards for a block that fits.
 * @param size Required size.
 * @param mode Search strategy.
 * @return The chosen block, or nullptr.
 */
MemBlock* ExpHeap::findFreeMemBlockFromHead_(size_t size, FindMode mode) const
{
    MemBlock* found = nullptr;

    for (auto& block : mFreeList)
    {
        if (block.mSize < size)
        {
            continue;
        }

        if (mode == FindMode::FirstFit)
        {
            return &block;
        }

        if (!found || (mode == FindMode::BestFit && found->mSize > block.mSize) ||
            (mode == FindMode::LargestFit && found->mSize < block.mSize))
        {
            found = &block;
        }
    }

    return found;
}

/**
 * Searches the free list forwards for a block that fits after alignment.
 * @param size Required size.
 * @param alignment Memory alignment.
 * @param mode Search strategy.
 * @return The chosen block, or nullptr.
 */
MemBlock* ExpHeap::findFreeMemBlockFromHead_(size_t size, s32 alignment, FindMode mode) const
{
    MemBlock* found = nullptr;

    for (auto& block : mFreeList)
    {
        if (block.mSize < size)
        {
            continue;
        }

        uintptr_t memory = uintptr_t(block.getMemory());
        uintptr_t alignedMemory = alignUp(memory, alignment);

        if (block.mSize < size + (alignedMemory - memory))
        {
            continue;
        }

        if (mode == FindMode::FirstFit)
        {
            return &block;
        }

        if (!found || (mode == FindMode::BestFit && found->mSize > block.mSize) ||
            (mode == FindMode::LargestFit && found->mSize < block.mSize))
        {
            found = &block;
        }
    }

    return found;
}

/**
 * Searches the free list backwards for a block that fits.
 * @param size Required size.
 * @param mode Search strategy.
 * @return The chosen block, or nullptr.
 */
MemBlock* ExpHeap::findFreeMemBlockFromTail_(size_t size, FindMode mode) const
{
    MemBlock* found = nullptr;

    for (MemBlock* block = mFreeList.back(); block; block = mFreeList.prev(block))
    {
        if (block->mSize < size)
        {
            continue;
        }

        if (mode == FindMode::FirstFit)
        {
            return block;
        }

        if (!found || (mode == FindMode::BestFit && found->mSize > block->mSize) ||
            (mode == FindMode::LargestFit && found->mSize < block->mSize))
        {
            found = block;
        }
    }

    return found;
}

/**
 * Searches the free list backwards for a block that fits after alignment.
 * @param size Required size.
 * @param alignment Memory alignment.
 * @param mode Search strategy.
 * @return The chosen block, or nullptr.
 */
MemBlock* ExpHeap::findFreeMemBlockFromTail_(size_t size, s32 alignment, FindMode mode) const
{
    MemBlock* found = nullptr;

    for (MemBlock* block = mFreeList.back(); block; block = mFreeList.prev(block))
    {
        if (block->mSize < size)
        {
            continue;
        }

        uintptr_t start = uintptr_t(block->getMemory()) + block->mSize - size;
        size_t padding = start & u32(alignment - 1);

        if (block->mSize < padding + size)
        {
            continue;
        }

        if (mode == FindMode::FirstFit)
        {
            return block;
        }

        if (!found || (mode == FindMode::BestFit && found->mSize > block->mSize) ||
            (mode == FindMode::LargestFit && found->mSize < block->mSize))
        {
            found = block;
        }
    }

    return found;
}

/**
 * Adds a block to the use list on the heap's allocation side.
 * @param pBlock Block to add.
 */
void ExpHeap::pushToUseList_(MemBlock* pBlock)
{
    if (mDirection == cHeapDirection_Forward)
    {
        mUseList.pushBack(pBlock);
    }
    else
    {
        mUseList.pushFront(pBlock);
    }
}

/**
 * Frees memory allocated from this heap.
 * @param pPtr Memory to free.
 */
void ExpHeap::free(void* pPtr)
{
    freeAndGetAllocatableSize(pPtr, 8);
}

MemBlock* ExpHeap::pushToFreeList_(MemBlock* pBlock)
{
    auto insertBetween = [this](MemBlock* prev, MemBlock* next, MemBlock* block) {
        bool merged = false;

        if (prev && prev->getMemoryEnd() == reinterpret_cast<u8*>(block))
        {
            prev->mSize = prev->mSize + sizeof(MemBlock) + block->mOffset + block->mSize;
            block = prev;
            merged = true;
        }

        if (block->getMemoryEnd() == reinterpret_cast<u8*>(next))
        {
            if (!merged)
            {
                block->mHeapCheckTag = MemBlock::cFreeHeapCheckTag;
                mFreeList.insertBefore(next, block);
            }

            block->mSize = block->mSize + next->mOffset + next->mSize + sizeof(MemBlock);
            mFreeList.erase(next);
        }
        else if (!merged)
        {
            block->mHeapCheckTag = MemBlock::cFreeHeapCheckTag;
            mFreeList.insertBefore(next, block);
        }

        return block;
    };

    auto pushBackBlock = [this](MemBlock* block) {
        if (!mFreeList.isEmpty())
        {
            MemBlock* last = mFreeList.back();

            if (last->getMemoryEnd() == reinterpret_cast<u8*>(block))
            {
                last->mSize = last->mSize + sizeof(MemBlock) + block->mOffset + block->mSize;
                return block;
            }
        }

        block->mHeapCheckTag = MemBlock::cFreeHeapCheckTag;
        mFreeList.pushBack(block);
        return block;
    };

    bool useFreeList;

    switch (mFindFreeBlockMode)
    {
    case FindFreeBlockMode::FromFreeList:
        useFreeList = true;
        break;
    case FindFreeBlockMode::ByIteratingMemBlock:
        useFreeList = mFreeList.size() < 2;
        break;
    default:
        useFreeList = mFreeList.size() * 3 <= mUseList.size() + 3;
        break;
    }

    if (useFreeList)
    {
        MemBlock* prev = nullptr;

        for (auto& block : mFreeList)
        {
            if (&block > pBlock)
            {
                return insertBetween(prev, &block, pBlock);
            }

            prev = &block;
        }
    }
    else
    {
        uintptr_t areaEnd =
            uintptr_t(mStart) +
            (mDirection == cHeapDirection_Forward ? mSize : mSize - sizeof(ExpHeap));
        for (auto* block = reinterpret_cast<MemBlock*>(pBlock->getMemoryEnd());
             uintptr_t(block) < areaEnd; block = reinterpret_cast<MemBlock*>(block->getMemoryEnd()))
        {
            if (block->isFree())
            {
                return insertBetween(mFreeList.prev(block), block, pBlock);
            }
        }
    }

    return pushBackBlock(pBlock);
}

/**
 * Walks the use list under the heap lock (printing is compiled out).
 */
void ExpHeap::dumpUseList() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    for (auto& block : mUseList)
    {
        static_cast<void>(block);
    }
}

/**
 * Walks the free list under the heap lock (printing is compiled out).
 */
void ExpHeap::dumpFreeList() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    for (auto& block : mFreeList)
    {
        static_cast<void>(block);
    }
}

void* ExpHeap::resizeFront(void* pPtr, size_t size)
{
    if (!isInclude(pPtr))
    {
        return nullptr;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    MemBlock* block = MemBlock::FindManageArea(pPtr);
    u8* memory = block->getMemory();
    size_t newSize = (size + 7) & ~size_t(7);

    if (block->mSize < newSize)
    {
        return nullptr;
    }

    size_t diff = block->mSize - newSize;

    if (diff == 0)
    {
        return memory;
    }

    size_t newBlockOffset = block->getTotalSize() - newSize - sizeof(MemBlock);

    if (newBlockOffset < sizeof(MemBlock))
    {
        u8* base = reinterpret_cast<u8*>(block + 1);
        size_t newOffset = block->mOffset + diff;
        block->mOffset = newOffset;

        if (u16(newOffset) != 0)
        {
            reinterpret_cast<uintptr_t*>(base + u16(newOffset))[-1] = uintptr_t(block) + 1;
        }

        return base + block->mOffset;
    }

    auto* newBlock = new (PtrUtil::addOffset(block, newBlockOffset)) MemBlock();
    newBlock->mHeapCheckTag = mHeapCheckTag;
    newBlock->mSize = newSize;
    pushToUseList_(newBlock);
    mUseList.erase(block);
    block->mSize = newBlockOffset - sizeof(MemBlock);
    block->mOffset = 0;
    pushToFreeList_(block);
    return newBlock->getMemory();
}

/**
 * Shrinks an allocation from its end and frees the remainder.
 * @param pPtr Allocation to shrink.
 * @param size New size.
 * @return The allocation, or nullptr if it cannot be resized.
 */
void* ExpHeap::resizeBack(void* pPtr, size_t size)
{
    if (!isInclude(pPtr))
    {
        return nullptr;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    MemBlock* block = MemBlock::FindManageArea(pPtr);
    size_t newSize = (size + 7) & ~size_t(7);

    if (block->mSize < newSize)
    {
        return nullptr;
    }

    size_t diff = block->mSize - newSize;

    if (diff == 0)
    {
        return block->getMemory();
    }

    if (diff < sizeof(MemBlock))
    {
        return block->getMemory();
    }

    block->mSize = newSize;
    u8* memory = reinterpret_cast<u8*>(block + 1);
    auto* freeBlock = new (memory + newSize + block->mOffset) MemBlock();
    freeBlock->mSize = diff - sizeof(MemBlock);
    pushToFreeList_(freeBlock);
    return memory + block->mOffset;
}

/**
 * Moves an allocation into a new block.
 * @param pPtr Old allocation.
 * @param pMemory Data to copy.
 * @param copySize Number of bytes to copy.
 * @param size New allocation size.
 * @param alignment New allocation alignment.
 * @return The new allocation, or nullptr.
 */
void* ExpHeap::realloc_(void* pPtr, u8* pMemory, size_t copySize, size_t size, s32 alignment)
{
    void* newPtr = tryAlloc(size, alignment);

    if (newPtr)
    {
        std::memcpy(newPtr, pMemory, copySize);
        free(pPtr);
    }

    return newPtr;
}

/**
 * Resizes an allocation in place or moves it to a new block.
 * @param pPtr Allocation to resize, or nullptr.
 * @param size New size; 0 frees the allocation.
 * @param alignment Required alignment.
 * @return The resized allocation, or nullptr.
 */
void* ExpHeap::tryRealloc(void* pPtr, size_t size, s32 alignment)
{
    if (!pPtr)
    {
        return tryAlloc(size, alignment);
    }

    if (size == 0)
    {
        free(pPtr);
        return nullptr;
    }

    if (!isInclude(pPtr) || alignment < 0)
    {
        return nullptr;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    MemBlock* block = MemBlock::FindManageArea(pPtr);
    size_t newSize = (size + 7) & ~size_t(7);
    size_t blockSize = block->mSize;

    if (blockSize < newSize)
    {
        if (alignment == 0)
        {
            alignment = 8;
        }

        return realloc_(pPtr, block->getMemory(), blockSize, newSize, alignment);
    }

    if (blockSize == newSize)
    {
        u8* memory = block->getMemory();

        if (alignment != 0 && !PtrUtil::isAlignedPow2(memory, alignment))
        {
            return realloc_(pPtr, memory, newSize, newSize, alignment);
        }

        return memory;
    }

    if (alignment != 0 && !PtrUtil::isAlignedPow2(block->getMemory(), alignment))
    {
        return realloc_(pPtr, block->getMemory(), newSize, newSize, alignment);
    }

    size_t diff = blockSize - newSize;

    if (diff < sizeof(MemBlock))
    {
        return block->getMemory();
    }

    block->mSize = newSize;
    u8* memory = reinterpret_cast<u8*>(block + 1);
    auto* freeBlock = new (memory + newSize + block->mOffset) MemBlock();
    freeBlock->mSize = diff - sizeof(MemBlock);
    pushToFreeList_(freeBlock);
    return memory + block->mOffset;
}

/**
 * Returns the start of the heap memory.
 * @return Start address.
 */
uintptr_t ExpHeap::getStartAddress() const
{
    return uintptr_t(mStart);
}

/**
 * Returns the end of the heap memory.
 * @return End address.
 */
uintptr_t ExpHeap::getEndAddress() const
{
    return uintptr_t(mStart) + mSize;
}

/**
 * Returns the heap size.
 * @return Heap size in bytes.
 */
size_t ExpHeap::getSize() const
{
    return mSize;
}

/**
 * Sums the sizes of all free blocks.
 * @return Total free size.
 */
size_t ExpHeap::getFreeSize() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    size_t freeSize = 0;

    for (auto& block : mFreeList)
    {
        freeSize += block.mSize;
    }

    return freeSize;
}

/**
 * Returns the largest size that can be allocated with an alignment.
 * @param alignment Required alignment.
 * @return Largest allocatable size, or 0.
 */
size_t ExpHeap::getMaxAllocatableSize(int alignment) const
{
    s32 absAlignment = alignment < 0 ? -alignment : alignment;

    if (!isPow2(absAlignment))
    {
        return 0;
    }

    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());

    MemBlock* block;
    size_t padding = 0;

    if (absAlignment <= 8)
    {
        block = findFreeMemBlockFromHead_(8, FindMode::LargestFit);
    }
    else
    {
        block = findFreeMemBlockFromHead_(8, absAlignment, FindMode::LargestFit);

        if (block)
        {
            uintptr_t memory = uintptr_t(block->getMemory());
            padding = alignUp(memory, absAlignment) - memory;
        }
    }

    if (!block)
    {
        return 0;
    }

    return block->mSize - padding;
}

/**
 * Returns the worst-case overhead of one allocation.
 * @param alignment Allocation alignment.
 * @return Overhead in bytes.
 */
size_t ExpHeap::getPerAllocationOverhead(s32 alignment)
{
    s32 absAlignment = alignment < 0 ? -alignment : alignment;

    if (absAlignment < 8)
    {
        absAlignment = Mathi::roundUpPow2(absAlignment, 8);
    }

    return sizeof(MemBlock) - 8 + absAlignment;
}

/**
 * Orders two blocks by address.
 * @param pA First block.
 * @param pB Second block.
 * @return -1 if pA is below pB, otherwise 1.
 */
s32 ExpHeap::compareMemBlockAddr_(const MemBlock* pA, const MemBlock* pB)
{
    if (s32(uintptr_t(pA) - uintptr_t(pB)) >= 0)
    {
        return 1;
    }

    return -1;
}

/**
 * Validates the free list.
 * @return True if every free block is well formed and the links are intact.
 */
bool ExpHeap::tryCheckFreeList() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    for (auto& block : mFreeList)
    {
        if (block.mOffset != 0)
        {
            return false;
        }

        if (block.mSize == 0 || !PtrUtil::isAligned(block.getMemory(), 8))
        {
            return false;
        }
    }

    return mFreeList.checkLinks();
}

/**
 * Validates the use list links.
 * @return True if the links are intact.
 */
bool ExpHeap::tryCheckUseList() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    return mUseList.checkLinks();
}

/**
 * Returns the size of an allocation.
 * @param pPtr Allocation to query.
 * @return Block size, or 0 if the pointer is not in this heap.
 */
size_t ExpHeap::getAllocatedSize(void* pPtr)
{
    if (!isInclude(pPtr))
    {
        return 0;
    }

    return MemBlock::FindManageArea(pPtr)->mSize;
}

/**
 * Writes the heap description as YAML.
 * @param rStream Output stream.
 * @param indent Indentation width.
 */
void ExpHeap::dumpYAML(WriteStream& rStream, int indent) const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());

    Heap::dumpYAML(rStream, indent);

    FixedSafeString<128> str("");

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  heap_type: ExpHeap\n");
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    const char* allocModeName = mAllocMode != AllocMode::FirstFit ? "Best Fit" : "First Fit";
    str.appendWithFormat("  alloc_mode: %s\n", allocModeName);
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  find_free_block_mode: %s\n",
                         getFindFreeBlockModeString_(mFindFreeBlockMode));
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  use_list_size: %d\n", mUseList.size());
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  free_list_size: %d\n", mFreeList.size());
    rStream.writeDecorationText(str);
}

/**
 * Prints a human-readable description of an expanded heap.
 * @param rHeap Heap to print.
 * @param pOutput Output target.
 */
template <>
void PrintFormatter::out<ExpHeap>(const ExpHeap& rHeap, const char*, PrintOutput* pOutput)
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&rHeap.mCS),
                                                rHeap.isLockEnabled());

    PrintFormatter::out<Heap>(rHeap, nullptr, pOutput);

    FixedSafeString<128> str;
    const char* allocModeName =
        rHeap.mAllocMode != ExpHeap::AllocMode::FirstFit ? "Best Fit" : "First Fit";
    str.format("         AllocMode: %s\n", allocModeName);
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format(" FindFreeBlockMode: %s\n",
               ExpHeap::getFindFreeBlockModeString_(rHeap.mFindFreeBlockMode));
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format("      UseList size: %d\n", rHeap.mUseList.size());
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format("     FreeList size: %d\n", rHeap.mFreeList.size());
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    OutImpl<char, SafeStringBase>::out("==================================================\n",
                                       nullptr, pOutput);
}

/**
 * Sets how freed blocks locate their free-list neighbours.
 * @param mode New mode.
 */
void ExpHeap::setFindFreeBlockMode(FindFreeBlockMode mode)
{
    mFindFreeBlockMode = mode;
}

/**
 * Dumps the use and free lists.
 */
void ExpHeap::dump() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    dumpUseList();
    dumpFreeList();
}

/**
 * Forwards host I/O information generation to Heap.
 * @param pContext Host I/O context.
 */
void ExpHeap::genInformation_(hostio::Context* pContext)
{
    Heap::genInformation_(pContext);
}

/**
 * Checks whether nothing is allocated.
 * @return True if the use list is empty.
 */
bool ExpHeap::isEmpty() const
{
    return this->mUseList.size() == 0;
}

/**
 * Reports that individual allocations can be freed.
 * @return Always true.
 */
bool ExpHeap::isFreeable() const
{
    return true;
}

/**
 * Reports that allocations can be resized.
 * @return Always true.
 */
bool ExpHeap::isResizable() const
{
    return true;
}

/**
 * Reports that the heap can be adjusted.
 * @return Always true.
 */
bool ExpHeap::isAdjustable() const
{
    return true;
}

/**
 * Returns the display name of a find-free-block mode.
 * @param mode Mode to name.
 * @return The mode name, or an empty string.
 */
const char* ExpHeap::getFindFreeBlockModeString_(FindFreeBlockMode mode)
{
    switch (mode)
    {
    case FindFreeBlockMode::Auto:
        return "Auto";
    case FindFreeBlockMode::FromFreeList:
        return "From FreeList";
    case FindFreeBlockMode::ByIteratingMemBlock:
        return "By Iterating MemBlock";
    default:
        return SafeString::cEmptyString.cstr();
    }
}
}  // namespace sead
