#include <heap/seadUnitHeap.h>

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
 * Constructs an empty unit heap.
 * @param rName Heap name.
 * @param pParent Parent heap.
 * @param pAddress Start of the managed memory.
 * @param size Size of the managed memory.
 * @param blockSize Size of every block.
 * @param enableLock Whether the heap lock is enabled.
 */
UnitHeap::UnitHeap(const SafeString& rName, Heap* pParent, void* pAddress, size_t size,
                   u32 blockSize, bool enableLock)
    : Heap(rName, pParent, pAddress, size, cHeapDirection_Forward, enableLock),
      mBlockSize(blockSize), mAreaStart(nullptr), mAreaSize(0), mFreeSize(0)
{
}

/**
 * Destroys the heap and everything allocated from it.
 */
UnitHeap::~UnitHeap()
{
    destruct_();
}

/**
 * Creates a unit heap inside a parent heap.
 * @param size Heap size, or 0 for the largest allocatable size.
 * @param rName Heap name.
 * @param blockSize Size of every block.
 * @param alignment Block alignment.
 * @param pParent Parent heap, or nullptr for the current heap.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr on failure.
 */
UnitHeap* UnitHeap::create(size_t size, const SafeString& rName, u32 blockSize, s32 alignment,
                           Heap* pParent, bool enableLock)
{
    return tryCreate(size, rName, blockSize, alignment, pParent, enableLock);
}

/**
 * Tries to allocate memory from a parent heap and build a unit heap in it.
 * @param size Heap size, or 0 for the largest allocatable size.
 * @param rName Heap name.
 * @param blockSize Size of every block.
 * @param alignment Block alignment.
 * @param pParent Parent heap, or nullptr for the current heap.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr on failure.
 */
UnitHeap* UnitHeap::tryCreate(size_t size, const SafeString& rName, u32 blockSize,
                              s32 alignment, Heap* pParent, bool enableLock)
{
    s32 absAlignment = alignment < 0 ? -alignment : alignment;
    if (blockSize == 0)
    {
        return nullptr;
    }

    s32 blockAlignment = absAlignment > 8 ? absAlignment : 8;
    if (((blockAlignment - 1) & blockAlignment) != 0)
    {
        return nullptr;
    }

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
        size = pParent->getMaxAllocatableSize(blockAlignment);
    }
    else
    {
        size = size + 7;
    }
    size &= ~size_t(7);

    size_t alignedBlockSize = (size_t(blockSize) + (blockAlignment - 1)) & -blockAlignment;
    if (size < alignedBlockSize + sizeof(UnitHeap))
    {
        return nullptr;
    }

    void* memory = pParent->tryAlloc(size, 8);
    if (!memory)
    {
        return nullptr;
    }

    auto* heap = new (memory) UnitHeap(rName, pParent, memory, size, blockSize, enableLock);
    heap->doCreate(blockAlignment, false, pParent);
    return heap;
}

/**
 * Lays out the block area, builds the free list and registers the heap with its parent.
 * @param alignment Block alignment.
 * @param isPadded Whether extra space was reserved for aligning the area start.
 * @param pParent Parent heap.
 */
void UnitHeap::doCreate(s32 alignment, bool isPadded, Heap* pParent)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    uintptr_t base = uintptr_t(this) + sizeof(UnitHeap);
    u8* areaStart = reinterpret_cast<u8*>(alignUp(base, alignment));
    if (isPadded && uintptr_t(areaStart) == base)
    {
        areaStart += alignment;
    }

    mAreaStart = areaStart;
    mAreaSize = mSize - (areaStart - reinterpret_cast<u8*>(this));
    mBlockSize = (mBlockSize + alignment - 1) & -alignment;
    mFreeSize = mBlockSize * getBlockNum();
    resetFreeList_();

    pParent->pushBackChild_(this);
}

/**
 * Tries to allocate a unit heap large enough for a number of blocks.
 * @param blockSize Size of every block.
 * @param blockNum Number of blocks.
 * @param rName Heap name.
 * @param alignment Block alignment.
 * @param pParent Parent heap, or nullptr for the current heap.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr on failure.
 */
UnitHeap* UnitHeap::tryCreateWithBlockNum(u32 blockSize, u32 blockNum, const SafeString& rName,
                                          s32 alignment, Heap* pParent, bool enableLock)
{
    s32 absAlignment = alignment < 0 ? -alignment : alignment;
    if (blockSize == 0 || blockNum == 0)
    {
        return nullptr;
    }

    s32 blockAlignment = absAlignment > 8 ? absAlignment : 8;
    if (((blockAlignment - 1) & blockAlignment) != 0)
    {
        return nullptr;
    }

    if (!pParent)
    {
        pParent = HeapMgr::instance()->getCurrentHeap();
        if (!pParent)
        {
            return nullptr;
        }
    }

    size_t alignedBlockSize = (size_t(blockSize) + (blockAlignment - 1)) & -blockAlignment;
    size_t size = alignedBlockSize * blockNum + getManagementAreaSize(blockAlignment);

    void* memory = pParent->tryAlloc(size, 8);
    if (!memory)
    {
        return nullptr;
    }

    auto* heap = new (memory) UnitHeap(rName, pParent, memory, size, blockSize, enableLock);
    heap->doCreate(blockAlignment, true, pParent);
    return heap;
}

/**
 * Links every block of the area into the free list.
 */
void UnitHeap::resetFreeList_()
{
    mFreeList.setWork(mAreaStart, mBlockSize, getBlockNum());
}

/**
 * Returns the bookkeeping overhead of a heap.
 * @param alignment Alignment padding to include.
 * @return Size of the heap object plus the padding.
 */
size_t UnitHeap::getManagementAreaSize(s32 alignment)
{
    return sizeof(UnitHeap) + alignment;
}

/**
 * Destroys the heap and returns its memory to the parent.
 */
void UnitHeap::destroy()
{
    Heap* parent = mParent;
    void* start = mStart;

    this->~UnitHeap();

    if (parent && parent->isFreeable())
    {
        parent->free(start);
    }
}

/**
 * Checks whether a pointer lies inside the block area.
 * @param pPtr Pointer to check.
 * @return True if the pointer is inside the area.
 */
bool UnitHeap::isInclude(const void* pPtr) const
{
    const void* start = mAreaStart;
    const void* end = PtrUtil::addOffset(mAreaStart, mAreaSize);
    return start <= pPtr && pPtr < end;
}

/**
 * Frees every block and rebuilds the free list.
 */
void UnitHeap::freeAll()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    dispose_(nullptr, nullptr);
    mFreeSize = mBlockSize * getBlockNum();
    resetFreeList_();
}

/**
 * Does nothing; unit heaps cannot be adjusted.
 * @return The heap size.
 */
size_t UnitHeap::adjust()
{
    return mSize;
}

/**
 * Takes one block from the free list.
 * @param size Requested size; must not exceed the block size.
 * @param alignment Requested alignment; must divide the block size.
 * @return The block, or nullptr.
 */
void* UnitHeap::tryAlloc(size_t size, s32 alignment)
{
    HeapMgr* mgr = HeapMgr::instance();
    if (alignment < 0)
    {
        notifyAllocFailed(mgr, this, size, alignment, size, alignment);
        return nullptr;
    }
    if ((mBlockSize & (size_t(alignment) - 1)) != 0 || size > mBlockSize)
    {
        notifyAllocFailed(mgr, this, size, alignment, size, alignment);
        return nullptr;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    void* ptr = mFreeList.getFree();
    if (!ptr)
    {
        HeapMgr::IAllocFailedCallback* callback = mgr ? mgr->getAllocFailedCallback() : nullptr;
        if (callback)
        {
            HeapMgr::AllocFailedCallbackArg arg;
            arg.heap = this;
            arg.request_size = size;
            arg.request_alignment = alignment;
            arg.alloc_size = mBlockSize;
            arg.alloc_alignment = alignment;
            callback->invoke(&arg);
        }
        return ptr;
    }
    mFreeList.alloc();
    mFreeSize -= mBlockSize;
    return ptr;
}

/**
 * Returns a block to the free list.
 * @param pPtr Block to free.
 */
void UnitHeap::free(void* pPtr)
{
    if (!pPtr || !isInclude(pPtr))
    {
        return;
    }

    if (mFlag.isOnBit(Flag::cDisposing))
    {
        return;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    mFreeList.free(pPtr);
    mFreeSize += mBlockSize;
}

/**
 * Does nothing; unit heap blocks cannot be resized.
 * @param pPtr Ignored.
 * @param size Ignored.
 * @return Always nullptr.
 */
void* UnitHeap::resizeFront(void* pPtr, size_t size)
{
    return nullptr;
}

/**
 * Does nothing; unit heap blocks cannot be resized.
 * @param pPtr Ignored.
 * @param size Ignored.
 * @return Always nullptr.
 */
void* UnitHeap::resizeBack(void* pPtr, size_t size)
{
    return nullptr;
}

/**
 * Returns the heap size.
 * @return Size of the managed memory.
 */
size_t UnitHeap::getSize() const
{
    return mSize;
}

/**
 * Returns the total size of the free blocks.
 * @return Free size in bytes.
 */
size_t UnitHeap::getFreeSize() const
{
    return mFreeSize;
}

/**
 * Returns the heap start address.
 * @return Start of the managed memory.
 */
uintptr_t UnitHeap::getStartAddress() const
{
    return uintptr_t(mStart);
}

/**
 * Returns the heap end address.
 * @return End of the managed memory.
 */
uintptr_t UnitHeap::getEndAddress() const
{
    return uintptr_t(mStart) + mSize;
}

/**
 * Returns the largest allocation the heap can serve.
 * @param alignment Ignored.
 * @return The block size if a block is free, otherwise 0.
 */
size_t UnitHeap::getMaxAllocatableSize(int alignment) const
{
    if (!mFreeList.getFree())
    {
        return 0;
    }
    return mBlockSize;
}

/**
 * Writes the heap description as YAML.
 * @param rStream Output stream.
 * @param indent Indentation width.
 */
void UnitHeap::dumpYAML(WriteStream& rStream, int indent) const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());

    Heap::dumpYAML(rStream, indent);

    FixedSafeString<128> str("");

    if (indent > 0)
    {
        str.append(' ', indent);
    }
    str.appendWithFormat("  heap_type: UnitHeap\n");
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }
    str.appendWithFormat("  block_size: %llu\n", mBlockSize);
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }
    str.appendWithFormat("  area_start: %llu\n", mAreaStart);
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }
    str.appendWithFormat("  area_size: %llu\n", mAreaSize);
    rStream.writeDecorationText(str);
}

/**
 * Prints a human-readable description of a unit heap.
 * @param rHeap Heap to print.
 * @param pOutput Output target.
 */
template <>
void PrintFormatter::out<UnitHeap>(const UnitHeap& rHeap, const char*, PrintOutput* pOutput)
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&rHeap.mCS),
                                                rHeap.isLockEnabled());

    PrintFormatter::out<Heap>(rHeap, nullptr, pOutput);

    FixedSafeString<128> str;
    OutImpl<char, SafeStringBase>::out("          HeapType: UnitHeap\n", nullptr, pOutput);

    str.format("         BlockSize: %llu\n", rHeap.mBlockSize);
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format("         AreaStart: 0x%016llX\n", rHeap.mAreaStart);
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format("          AreaSize: %llu\n", rHeap.mAreaSize);
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    OutImpl<char, SafeStringBase>::out("==================================================\n",
                                       nullptr, pOutput);
}

/**
 * Dumps the heap (no output in release builds).
 */
void UnitHeap::dump() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
}

/**
 * Forwards host I/O information generation to Heap.
 * @param pContext Host I/O context.
 */
void UnitHeap::genInformation_(hostio::Context* pContext)
{
    Heap::genInformation_(pContext);
}

/**
 * Checks whether nothing is allocated.
 * @return True if every block is free.
 */
bool UnitHeap::isEmpty() const
{
    return mFreeSize == mBlockSize * getBlockNum();
}

/**
 * Reports that blocks can be freed individually.
 * @return Always true.
 */
bool UnitHeap::isFreeable() const
{
    return true;
}

/**
 * Reports that blocks cannot be resized.
 * @return Always false.
 */
bool UnitHeap::isResizable() const
{
    return false;
}

/**
 * Reports that the heap cannot be adjusted.
 * @return Always false.
 */
bool UnitHeap::isAdjustable() const
{
    return false;
}
}  // namespace sead
