#include <heap/seadSeparateHeap.h>

#include <heap/seadHeapMgr.h>
#include <prim/seadFormatPrint.h>
#include <prim/seadSafeString.h>
#include <prim/seadScopedLock.h>
#include <stream/seadStream.h>

namespace sead
{
namespace
{
constexpr size_t cBlockNodeSize = 0x20;

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
 * Constructs the heap and sets up its block list in the management area.
 * @param rName Heap name.
 * @param pParent Parent heap.
 * @param pManagementArea Memory for the block list nodes.
 * @param managementAreaSize Size of the management area.
 * @param pHeapStart Start of the managed memory.
 * @param heapSize Size of the managed memory.
 * @param enableLock Whether the heap lock is enabled.
 */
SeparateHeap::SeparateHeap(const SafeString& rName, Heap* pParent, void* pManagementArea,
                           size_t managementAreaSize, void* pHeapStart, size_t heapSize,
                           bool enableLock)
    : Heap(rName, pParent, pHeapStart, heapSize, cHeapDirection_Forward, enableLock)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    s32 nodeNum = managementAreaSize / cBlockNodeSize;
    if (pManagementArea && nodeNum > 0)
    {
        mBlockList.setBuffer(nodeNum, pManagementArea);
    }
}

/**
 * Destroys the heap and everything allocated from it.
 */
SeparateHeap::~SeparateHeap()
{
    destruct_();
}

/**
 * Builds a separate heap whose bookkeeping lives in a dedicated management area.
 * @param rName Heap name.
 * @param pManagementArea Memory for the heap object and the block list.
 * @param managementAreaSize Size of the management area.
 * @param pHeapStart Start of the managed memory.
 * @param heapSize Size of the managed memory.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap.
 */
SeparateHeap* SeparateHeap::create(const SafeString& rName, void* pManagementArea,
                                   size_t managementAreaSize, void* pHeapStart, size_t heapSize,
                                   bool enableLock)
{
    return new (pManagementArea) SeparateHeap(
        rName, nullptr, PtrUtil::addOffset(pManagementArea, sizeof(SeparateHeap)),
        managementAreaSize - sizeof(SeparateHeap), pHeapStart, heapSize, enableLock);
}

/**
 * Tries to allocate the management area and the heap memory from a parent heap.
 * @param rName Heap name.
 * @param managementAreaSize Size of the management area, including the heap object.
 * @param heapSize Heap size, or 0 for the largest allocatable size.
 * @param pParent Parent heap, or nullptr for the current heap.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr on failure.
 */
SeparateHeap* SeparateHeap::tryCreate(const SafeString& rName, size_t managementAreaSize,
                                      size_t heapSize, Heap* pParent, bool enableLock)
{
    if (managementAreaSize < sizeof(SeparateHeap) + cBlockNodeSize ||
        (managementAreaSize & 3) != 0)
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

    size_t maxSize = pParent->getMaxAllocatableSize(8);
    if (heapSize != 0)
    {
        heapSize = (heapSize + 7) & ~size_t(7);
        if (maxSize < heapSize + managementAreaSize)
        {
            return nullptr;
        }
    }
    else
    {
        if (maxSize < managementAreaSize + 8)
        {
            return nullptr;
        }

        heapSize = (maxSize - managementAreaSize) & ~size_t(7);
    }

    void* memory = pParent->tryAlloc(heapSize + managementAreaSize, 8);
    auto* heap = new (memory) SeparateHeap(
        rName, pParent, PtrUtil::addOffset(memory, sizeof(SeparateHeap)),
        managementAreaSize - sizeof(SeparateHeap), PtrUtil::addOffset(memory, managementAreaSize),
        heapSize, enableLock);
    pParent->pushBackChild_(heap);
    return heap;
}

/**
 * Returns the management area size needed for a number of blocks.
 * @param nodeNum Maximum number of live allocations.
 * @return Size of the heap object plus the block list nodes.
 */
size_t SeparateHeap::getManagementAreaSize(size_t nodeNum)
{
    return nodeNum * cBlockNodeSize + sizeof(SeparateHeap);
}

/**
 * Creates a separate heap inside a parent heap.
 * @param rName Heap name.
 * @param managementAreaSize Size of the management area, including the heap object.
 * @param heapSize Heap size, or 0 for the largest allocatable size.
 * @param pParent Parent heap, or nullptr for the current heap.
 * @param enableLock Whether the heap lock is enabled.
 * @return The new heap, or nullptr on failure.
 */
SeparateHeap* SeparateHeap::create(const SafeString& rName, size_t managementAreaSize,
                                   size_t heapSize, Heap* pParent, bool enableLock)
{
    return tryCreate(rName, managementAreaSize, heapSize, pParent, enableLock);
}

/**
 * Destroys the heap and returns its management area to the parent.
 */
void SeparateHeap::destroy()
{
    Heap* parent = mParent;

    this->~SeparateHeap();

    if (parent && parent->isFreeable())
    {
        parent->free(this);
    }
}

/**
 * Returns the size needed to place an allocation at an aligned address.
 * @param pAddress Candidate address.
 * @param size Allocation size.
 * @param alignment Allocation alignment.
 * @return The size plus the alignment padding.
 */
size_t SeparateHeap::getAllocateAreaSize(void* pAddress, size_t size, s32 alignment)
{
    return size + (alignUp(uintptr_t(pAddress), alignment) - uintptr_t(pAddress));
}

/**
 * Allocates memory in the first gap between blocks that is large enough.
 * @param size Requested size.
 * @param alignment Requested alignment.
 * @return The allocated memory, or nullptr.
 */
void* SeparateHeap::tryAlloc(size_t size, s32 alignment)
{
    HeapMgr* mgr = HeapMgr::instance();
    s32 absAlignment = alignment < 0 ? -alignment : alignment;
    size_t allocSize = size > 8 ? size : 8;

    u32 mask = absAlignment - 1;
    if ((mask & absAlignment) != 0)
    {
        notifyAllocFailed(mgr, this, size, alignment, allocSize, absAlignment);
        return nullptr;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());

    if (mBlockList.isFull())
    {
        notifyAllocFailed(mgr, this, size, alignment, allocSize, absAlignment);
        return nullptr;
    }

    Block* next = nullptr;
    uintptr_t address = 0;
    uintptr_t prevEnd = uintptr_t(mStart);
    for (auto& block : mBlockList)
    {
        uintptr_t blockStart = uintptr_t(block.mAddress);
        uintptr_t alignedEnd = (prevEnd + mask) & ~uintptr_t(mask);
        if (intptr_t(blockStart - alignedEnd) >= intptr_t(allocSize))
        {
            address = alignedEnd;
            next = &block;
            break;
        }

        prevEnd = uintptr_t(block.mAddress) + block.mSize;
    }

    if (address == 0)
    {
        address = (prevEnd + mask) & ~uintptr_t(mask);
        if (address == 0 || intptr_t(uintptr_t(mStart) - address + mSize) < intptr_t(allocSize))
        {
            notifyAllocFailed(mgr, this, size, alignment, allocSize, absAlignment);
            return nullptr;
        }
    }

    Block* block;
    if (next)
    {
        block = mBlockList.emplaceBefore(next);
    }
    else
    {
        block = mBlockList.emplaceBack();
    }

    block->mAddress = reinterpret_cast<void*>(address);
    block->mSize = allocSize;
    return reinterpret_cast<void*>(address);
}

/**
 * Frees an allocation and releases its block.
 * @param pPtr Memory to free.
 */
void SeparateHeap::free(void* pPtr)
{
    if (!pPtr)
    {
        return;
    }

    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    Block* block = findBlock_(pPtr);
    if (block)
    {
        mBlockList.erase(block);
    }
}

/**
 * Finds the block that starts at an address.
 * @param pPtr Allocation address.
 * @return The block, or nullptr.
 */
SeparateHeap::Block* SeparateHeap::findBlock_(void* pPtr)
{
    for (auto& block : mBlockList)
    {
        if (block.mAddress == pPtr)
        {
            return &block;
        }
    }

    return nullptr;
}

/**
 * Resizes an allocation by moving its start while keeping its end.
 * @param pPtr Allocation to resize.
 * @param size New size.
 * @return The new allocation address, or nullptr on failure.
 */
void* SeparateHeap::resizeFront(void* pPtr, size_t size)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    Block* block = findBlock_(pPtr);
    if (!block)
    {
        return nullptr;
    }

    size_t diff = block->mSize - size;
    if (block->mSize < size)
    {
        Block* prev = mBlockList.prev(block);
        uintptr_t prevEnd = prev ? uintptr_t(prev->mAddress) + prev->mSize : uintptr_t(mStart);
        void* newAddress = PtrUtil::addOffset(block->mAddress, diff);
        if (prevEnd > uintptr_t(newAddress))
        {
            return nullptr;
        }

        block->mAddress = newAddress;
        block->mSize = size;
        return newAddress;
    }

    if (block->mSize > size)
    {
        void* newAddress = PtrUtil::addOffset(pPtr, diff);
        block->mAddress = newAddress;
        block->mSize = size;
        return newAddress;
    }

    return pPtr;
}

/**
 * Resizes an allocation by moving its end while keeping its start.
 * @param pPtr Allocation to resize.
 * @param size New size.
 * @return The allocation address, or nullptr on failure.
 */
void* SeparateHeap::resizeBack(void* pPtr, size_t size)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    Block* block = findBlock_(pPtr);
    if (!block)
    {
        return nullptr;
    }

    if (block->mSize < size)
    {
        Block* next = mBlockList.next(block);
        uintptr_t nextStart = next ? uintptr_t(next->mAddress) : uintptr_t(mStart) + mSize;
        if (nextStart < uintptr_t(pPtr) + size)
        {
            return nullptr;
        }

        block->mSize = size;
        return pPtr;
    }

    if (block->mSize > size)
    {
        block->mSize = size;
    }

    return pPtr;
}

/**
 * Frees every allocation.
 */
void SeparateHeap::freeAll()
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    dispose_(nullptr, nullptr);
    mBlockList.clear();
}

/**
 * Returns the heap size minus the size of every allocation.
 * @return Free size in bytes.
 */
size_t SeparateHeap::getFreeSize() const
{
    size_t usedSize = 0;
    for (auto& block : mBlockList)
    {
        usedSize += block.mSize;
    }

    return mSize - usedSize;
}

/**
 * Returns the largest aligned gap between blocks.
 * @param alignment Requested alignment.
 * @return Size of the largest gap.
 */
size_t SeparateHeap::getMaxAllocatableSize(int alignment) const
{
    size_t maxSize = 0;
    const void* prevEnd = mStart;
    for (auto& block : mBlockList)
    {
        uintptr_t address = alignUp(uintptr_t(prevEnd), alignment);
        if (uintptr_t(block.mAddress) > address)
        {
            maxSize = std::max(maxSize, uintptr_t(block.mAddress) - address);
        }

        prevEnd = PtrUtil::addOffset(block.mAddress, block.mSize);
    }

    const void* end = PtrUtil::addOffset(mStart, mSize);
    const void* address = reinterpret_cast<const void*>(alignUp(uintptr_t(prevEnd), alignment));
    if (address < end)
    {
        maxSize = std::max(maxSize, size_t(PtrUtil::diff(end, address)));
    }

    return maxSize;
}

/**
 * Checks whether a pointer lies inside the managed memory.
 * @param pPtr Pointer to check.
 * @return True if the pointer is inside the heap.
 */
bool SeparateHeap::isInclude(const void* pPtr) const
{
    return uintptr_t(mStart) <= uintptr_t(pPtr) && uintptr_t(pPtr) < uintptr_t(mStart) + mSize;
}

/**
 * Walks the block list (no output in release builds).
 */
void SeparateHeap::dumpBlockList() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    for (auto& block : mBlockList)
    {
        static_cast<void>(block);
    }
}

/**
 * Writes the heap description as YAML.
 * @param rStream Output stream.
 * @param indent Indentation width.
 */
void SeparateHeap::dumpYAML(WriteStream& rStream, int indent) const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());

    Heap::dumpYAML(rStream, indent);

    FixedSafeString<128> str("");

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  heap_type: SeparateHeap\n");
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  block_list_size: %d\n", mBlockList.size());
    rStream.writeDecorationText(str);
}

/**
 * Prints a human-readable description of a separate heap.
 * @param rHeap Heap to print.
 * @param pOutput Output target.
 */
template <>
void PrintFormatter::out<SeparateHeap>(const SeparateHeap& rHeap, const char*,
                                       PrintOutput* pOutput)
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&rHeap.mCS),
                                                rHeap.isLockEnabled());

    PrintFormatter::out<Heap>(rHeap, nullptr, pOutput);

    FixedSafeString<128> str;
    str.format(" BlockList size: %d\n", rHeap.mBlockList.size());
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    OutImpl<char, SafeStringBase>::out("==================================================\n",
                                       nullptr, pOutput);
}

/**
 * Dumps the block list (no output in release builds).
 */
void SeparateHeap::dump() const
{
    ConditionalScopedLock<CriticalSection> lock(const_cast<CriticalSection*>(&mCS),
                                                isLockEnabled());
    dumpBlockList();
}

/**
 * Forwards host I/O information generation to Heap.
 * @param pContext Host I/O context.
 */
void SeparateHeap::genInformation_(hostio::Context* pContext)
{
    Heap::genInformation_(pContext);
}
}  // namespace sead
