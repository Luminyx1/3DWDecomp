#include "utility/aglParameterStringMgr.h"
#include <prim/seadMemUtil.h>
#include <prim/seadScopedLock.h>
#include <thread/seadCriticalSection.h>

namespace agl::utl
{

SEAD_SINGLETON_DISPOSER_IMPL(ParameterStringMgr)

/**
 * Constructs the string manager with no heap attached.
 */
ParameterStringMgr::ParameterStringMgr()
{
#ifdef SEAD_DEBUG
    setNodeName("utl::ParameterStringMgr");
    setNodeMeta("Icon=NOTE");
#endif
}

/**
 * Frees the string pointer buffer.
 */
ParameterStringMgr::~ParameterStringMgr()
{
    mStrings.freeBuffer();
}

/**
 * Attaches a heap and allocates the sorted string table from it.
 * @param pHeap heap used for the table and for interned strings
 */
void ParameterStringMgr::initialize(sead::Heap* pHeap)
{
    mHeap = pHeap;

    if (pHeap != nullptr)
    {
        mStrings.allocBuffer(0x20000, pHeap);
        mStrings.clear();
    }
}

/**
 * Interns a stack string into the sorted table, returning a persistent copy.
 * @param rString string to intern
 * @return pointer to a persistent string equal to rString
 */
const char* ParameterStringMgr::appendString(const sead::SafeString& rString)
{
    if (mHeap == nullptr)
    {
        return sead::SafeString::cEmptyString.cstr();
    }

    if (!sead::MemUtil::isStack(rString.cstr()))
    {
        return rString.cstr();
    }

    sead::ScopedLock<sead::CriticalSection> lock(&mCS);

    if (mStrings.size() == 0)
    {
        auto* string = new (mHeap) sead::HeapSafeString(mHeap, rString);
        mStrings.pushBack(string);
        return string->cstr();
    }

    s32 a = 0;
    s32 b = mStrings.size() - 1;

    while (a < b)
    {
        const s32 m = (a + b) / 2;
        const s32 c = mStrings.unsafeAt(m)->compare(rString);

        if (c == 0)
        {
            return mStrings.unsafeAt(m)->cstr();
        }

        if (c < 0)
        {
            a = m + 1;
        }
        else
        {
            b = m;
        }
    }

    const s32 c = mStrings.unsafeAt(a)->compare(rString);

    if (c == 0)
    {
        return mStrings.unsafeAt(a)->cstr();
    }

    const s32 length = rString.calcLength();
    SEAD_ASSERT(length > 0);
    auto* string = new (mHeap) sead::HeapSafeString(mHeap, rString);

    if (c < 0)
    {
        mStrings.insert(a + 1, string);
    }
    else
    {
        mStrings.insert(a, string);
    }

    return string->cstr();
}

/**
 * Generates host IO messages (empty in release builds).
 * @param pContext host IO context
 */
void ParameterStringMgr::genMessage(sead::hostio::Context* pContext) {}

/**
 * Handles host IO property events (empty in release builds).
 * @param pEvent property event
 */
void ParameterStringMgr::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

}  // namespace agl::utl
