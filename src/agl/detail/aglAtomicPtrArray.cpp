#include "utility/aglAtomicPtrArray.h"
#include <basis/seadNew.h>

namespace agl::detail
{

/**
 * Uses an external buffer for the pointers.
 * @param ptrNumMax capacity of the buffer
 * @param pBuffer buffer to use
 */
void AtomicPtrArrayImpl::setBuffer(s32 ptrNumMax, void* pBuffer)
{
    if (ptrNumMax >= 1)
    {
        if (!pBuffer)
        {
            SEAD_ASSERT_MSG(false, "buf is null");
            return;
        }

        mPtrs = static_cast<void**>(pBuffer);
        mPtrNum = 0;
        mPtrNumMax = ptrNumMax;
    }
    else
    {
        SEAD_ASSERT_MSG(false, "ptrNumMax[%d] must be larger than zero", ptrNumMax);
    }
}

/**
 * Allocates the pointer buffer from a heap.
 * @param ptrNumMax capacity of the buffer
 * @param pHeap heap to allocate from
 * @param alignment buffer alignment
 */
void AtomicPtrArrayImpl::allocBuffer(s32 ptrNumMax, sead::Heap* pHeap, s32 alignment)
{
    SEAD_ASSERT(mPtrs == nullptr);

    if (ptrNumMax >= 1)
    {
        setBuffer(ptrNumMax, new (pHeap, alignment) u8[s32(sizeof(void*)) * ptrNumMax]);
    }
    else
    {
        SEAD_ASSERT_MSG(false, "ptrNumMax[%d] must be larger than zero", ptrNumMax);
    }
}

/**
 * Frees the pointer buffer.
 */
void AtomicPtrArrayImpl::freeBuffer()
{
    if (isBufferReady())
    {
        delete[] mPtrs;
        mPtrs = nullptr;
        mPtrNum = 0;
        mPtrNumMax = 0;
    }
}

/**
 * Removes a range of pointers, moving the following ones forward.
 * @param pos index of the first pointer to remove
 * @param count number of pointers to remove
 */
void AtomicPtrArrayImpl::erase(s32 pos, s32 count)
{
    if (pos < 0)
    {
        SEAD_ASSERT_MSG(false, "illegal position[%d]", pos);
        return;
    }

    if (count < 0)
    {
        SEAD_ASSERT_MSG(false, "illegal number[%d]", count);
        return;
    }

    const s32 end_pos = pos + count;
    const s32 ptr_num = mPtrNum;

    if (pos + count > ptr_num)
    {
        SEAD_ASSERT_MSG(false, "pos[%d] + num[%d] exceed size[%d]", pos, count, ptr_num);
        return;
    }

    if (ptr_num > end_pos)
    {
        sead::MemUtil::copyOverlap(mPtrs + pos, mPtrs + end_pos,
                                   sizeof(void*) * (ptr_num - end_pos));
    }

    mPtrNum = ptr_num - count;
}

/**
 * Shuffles the pointers (Fisher-Yates).
 * @param pRandom random number generator to use
 */
void AtomicPtrArrayImpl::shuffle(sead::Random* pRandom)
{
    SEAD_ASSERT(pRandom);

    for (s32 i = mPtrNum; i > 1; --i)
    {
        swap(i - 1, pRandom->getS32Range(0, i));
    }

}

void AtomicPtrArrayImpl::sort(CompareCallbackImpl cmp)
{
    void** ptrs = mPtrs;

    if (mPtrNum < 2)
    {
        return;
    }

    s32 lo = 0;
    s32 hi = mPtrNum - 1;
    do
    {
        s32 last = lo;

        for (s32 i = lo; i < hi; ++i)
        {
            void** p = &ptrs[i];

            if (cmp(p[0], p[1]) > 0)
            {
                void* tmp = p[1];
                p[1] = p[0];
                p[0] = tmp;
                last = i;
            }
        }

        if (last <= lo)
        {
            break;
        }

        hi = last;

        for (s32 i = hi; i > lo; --i)
        {
            void** p = &ptrs[i];

            if (cmp(p[0], p[-1]) < 0)
            {
                void* tmp = p[-1];
                p[-1] = p[0];
                p[0] = tmp;
                last = i;
            }
        }

        lo = last;
    } while (lo != hi);
}

void AtomicPtrArrayImpl::heapSort(CompareCallbackImpl cmp)
{
    const s32 num = mPtrNum;

    if (num < 2)
    {
        return;
    }

    void** ptrs = mPtrs;

    for (s32 root = num / 2; root > 0; --root)
    {
        void* value = ptrs[root - 1];
        s32 parent = root;
        s32 child = parent * 2;

        while (child <= num)
        {
            if (child < num && cmp(ptrs[child - 1], ptrs[child]) < 0)
            {
                child++;
            }

            if (cmp(value, ptrs[child - 1]) >= 0)
            {
                break;
            }

            ptrs[parent - 1] = ptrs[child - 1];
            parent = child;
            child = parent * 2;
        }

        ptrs[parent - 1] = value;
    }

    for (s64 size = num; size > 1; --size)
    {
        const s64 last = size - 1;
        void* value = ptrs[last];
        ptrs[last] = ptrs[0];
        s32 parent = 1;
        s32 child = 2;

        while (child <= last)
        {
            if (child < last && cmp(ptrs[child - 1], ptrs[child]) < 0)
            {
                child++;
            }

            if (cmp(value, ptrs[child - 1]) >= 0)
            {
                break;
            }

            ptrs[parent - 1] = ptrs[child - 1];
            parent = child;
            child = parent * 2;
        }

        ptrs[parent - 1] = value;
    }
}

}  // namespace agl::detail
