#include <heap/seadHeap.h>
#include <heap/seadHeapMgr.h>
#include <prim/seadFormatPrint.h>
#include <prim/seadScopedLock.h>
#include <stream/seadStream.h>

namespace sead
{
/**
 * Constructs a heap, registers it with its parent's disposers and assigns a fresh check tag.
 * @param rName Name of the heap.
 * @param pParent Parent heap, or nullptr for a root heap.
 * @param pAddress Start address of the memory managed by the heap.
 * @param size Size of the managed memory in bytes.
 * @param direction Allocation direction.
 * @param enableLock Whether operations on this heap take its critical section.
 */
Heap::Heap(const SafeString& rName, Heap* pParent, void* pAddress, size_t size,
           HeapDirection direction, bool enableLock)
    : IDisposer(pParent, HeapNullOption::UseSpecifiedOrContainHeap), INamable(rName),
      mStart(pAddress), mSize(size), mParent(pParent), mDirection(direction), mCS(pParent),
      mFlag(1 << Flag::cEnableWarning)
{
    u32 tag;
    do
    {
        tag = HeapMgr::sHeapCheckTag.increment();
    } while (u16(tag) == 0xFFFF);
    mHeapCheckTag = tag;
    mFlag.changeBit(Flag::cEnableLock, enableLock);

    auto lock = makeScopedHeapLock();
    mChildren.initOffset(offsetof(Heap, mListNode));
    mDisposerList.initOffset(IDisposer::getListNodeOffset());
}

/**
 * Destroys the heap.
 */
Heap::~Heap() = default;

/**
 * Disposes every disposer in this heap and detaches it from its parent or the root heap list.
 */
void Heap::destruct_()
{
    auto lock = makeScopedHeapLock();
    dispose_(nullptr, nullptr);
    HeapMgr::removeFromFindContainHeapCache_(this);

    if (mParent != nullptr)
    {
        mParent->eraseChild_(this);
    }
    else
    {
        HeapMgr::removeRootHeap(this);
    }
}

/**
 * Destroys every disposer owned by this heap, optionally only those inside a memory range.
 * @param pBegin Start of the range, or nullptr together with pEnd to dispose everything.
 * @param pEnd End of the range (exclusive).
 */
void Heap::dispose_(const void* pBegin, const void* pEnd)
{
    mFlag.setBit(Flag::cDisposing);

    const bool disposeAll = pBegin == nullptr && pEnd == nullptr;
    auto it = mDisposerList.begin();

    while (it != mDisposerList.end())
    {
        if (it->mDisposerHeap != nullptr && (disposeAll || (pBegin <= &*it && &*it < pEnd)))
        {
            it->~IDisposer();
            it = mDisposerList.begin();
        }
        else
        {
            ++it;
        }
    }

    mFlag.resetBit(Flag::cDisposing);
}

/**
 * Removes a child heap from this heap's child list under the heap tree lock.
 * @param pChild Child heap to remove.
 */
void Heap::eraseChild_(Heap* pChild)
{
    ScopedCriticalSectionLock treeLock(&HeapMgr::sHeapTreeLockCS);
    auto lock = makeScopedHeapLock();
    mChildren.erase(pChild);
}

/**
 * Adds a disposer to this heap's disposer list.
 * @param pDisposer Disposer to add.
 */
void Heap::appendDisposer_(IDisposer* pDisposer)
{
    auto lock = makeScopedHeapLock();
    mDisposerList.pushBack(pDisposer);
}

/**
 * Removes a disposer from this heap's disposer list.
 * @param pDisposer Disposer to remove.
 */
void Heap::removeDisposer_(IDisposer* pDisposer)
{
    auto lock = makeScopedHeapLock();
    mDisposerList.erase(pDisposer);
}

/**
 * Finds the deepest heap in this subtree that contains an address.
 * @param pAddress Address to look up.
 * @return The containing heap, or nullptr if this heap does not contain the address.
 */
Heap* Heap::findContainHeap_(const void* pAddress)
{
    if (!isInclude(pAddress))
    {
        return nullptr;
    }

    for (auto it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        if (it->isInclude(pAddress))
        {
            return it->findContainHeap_(pAddress);
        }
    }

    return this;
}

/**
 * Appends a child heap to this heap's child list under the heap tree lock.
 * @param pChild Child heap to append.
 */
void Heap::pushBackChild_(Heap* pChild)
{
    ScopedCriticalSectionLock treeLock(&HeapMgr::sHeapTreeLockCS);
    auto lock = makeScopedHeapLock();
    mChildren.pushBack(pChild);
}

/**
 * Checks the accessing thread (no-op in release builds).
 */
void Heap::checkAccessThread_() const {}

/**
 * Writes this heap and all of its descendants to a stream as YAML.
 * @param rStream Stream to write to.
 * @param indent Number of spaces to indent this heap's entry by.
 */
void Heap::dumpTreeYAML(WriteStream& rStream, int indent) const
{
    dumpYAML(rStream, indent);

    FixedSafeString<128> str("");

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  children:\n");
    rStream.writeDecorationText(str);

    for (const auto& child : mChildren)
    {
        child.dumpTreeYAML(rStream, indent + 4);
    }
}

/**
 * Writes this heap's properties to a stream as a YAML list entry.
 * @param rStream Stream to write to.
 * @param indent Number of spaces to indent each line by.
 */
void Heap::dumpYAML(WriteStream& rStream, int indent) const
{
    auto lock = makeScopedHeapLock();

    FixedSafeString<128> str("");

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("- name: \"%s\"\n", getName().cstr());
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  start_address: 0x%016llX\n", getStartAddress());
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  end_address: 0x%016llX\n", getEndAddress());
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    const char* parentName = (mParent != nullptr) ? mParent->getName().cstr() : "--";
    str.appendWithFormat("  parent: %s\n", parentName);
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    const char* directionName = mDirection != cHeapDirection_Forward ? "Reverse" : "Forward";
    str.appendWithFormat("  direction: %s\n", directionName);
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  size: %llu\n", getSize());
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  free_size: %llu\n", getFreeSize());
    rStream.writeDecorationText(str);
    str.clear();

    if (indent > 0)
    {
        str.append(' ', indent);
    }

    str.appendWithFormat("  max_allocatable_size: %llu\n", getMaxAllocatableSize(8));
    rStream.writeDecorationText(str);
}

/**
 * Prints a human readable summary of a heap.
 * @param rHeap Heap to print.
 * @param pOutput Output to print to.
 */
template <>
void PrintFormatter::out<Heap>(const Heap& rHeap, const char*, PrintOutput* pOutput)
{
    auto lock = rHeap.makeScopedHeapLock();

    FixedSafeString<128> str;
    OutImpl<char, SafeStringBase>::out("\n", nullptr, pOutput);
    OutImpl<char, SafeStringBase>::out("==================================================\n",
                                       nullptr, pOutput);

    str.format("              Name: %s\n", rHeap.getName().cstr());
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format("             Range: [0x%016llX - 0x%016llX)\n", rHeap.getStartAddress(),
               rHeap.getEndAddress());
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format("            Parent: %s (0x%016llX)\n",
               (rHeap.mParent != nullptr) ? rHeap.mParent->getName().cstr() : "--", rHeap.mParent);
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format("         Direction: %s\n",
               rHeap.mDirection == Heap::cHeapDirection_Forward ? "Forward" : "Reverse");
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format("              Size: %llu\n", rHeap.getSize());
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format("          FreeSize: %llu\n", rHeap.getFreeSize());
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    str.format("MaxAllocatableSize: %llu\n", rHeap.getMaxAllocatableSize(8));
    OutImpl<char, SafeStringBase>::out(str.cstr(), nullptr, pOutput);

    OutImpl<char, SafeStringBase>::out("--------------------------------------------------\n",
                                       nullptr, pOutput);
}

/**
 * Generates host I/O information (no-op in release builds).
 */
void Heap::genInformation_(hostio::Context*) {}

/**
 * Formats this heap's usage percentage as a host I/O meta string.
 * @param pMeta String to write the meta string to.
 */
void Heap::makeMetaString_(BufferedSafeString* pMeta)
{
    pMeta->format("$SEAD_META_HEAP_%03d",
                  s32((1.0f - f32(getFreeSize()) / f32(getSize())) * 10) * 10);
}

}  // namespace sead
