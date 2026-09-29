#include <algorithm>
#include <basis/seadNew.h>
#include <basis/seadRawPrint.h>
#include <container/seadPtrArray.h>

namespace sead
{
/**
 * Uses an existing buffer for the pointers and empties the array.
 * @param ptrNumMax number of pointers the buffer holds
 * @param pBuf buffer of ptrNumMax pointers
 */
void PtrArrayImpl::setBuffer(s32 ptrNumMax, void* pBuf)
{
    if (ptrNumMax < 1)
    {
        SEAD_ASSERT_MSG(false, "ptrNumMax[%d] must be larger than zero", ptrNumMax);
        return;
    }

    if (pBuf == nullptr)
    {
        SEAD_ASSERT_MSG(false, "pBuf is null");
        return;
    }

    mPtrs = static_cast<void**>(pBuf);
    mPtrNum = 0;
    mPtrNumMax = ptrNumMax;
}

/**
 * Allocates the buffer for the pointers.
 * @param ptrNumMax number of pointers to make room for
 * @param pHeap heap to allocate from
 * @param alignment buffer alignment
 */
void PtrArrayImpl::allocBuffer(s32 ptrNumMax, Heap* pHeap, s32 alignment)
{
    SEAD_ASSERT(mPtrs == nullptr);

    if (ptrNumMax < 1)
    {
        SEAD_ASSERT_MSG(false, "ptrNumMax[%d] must be larger than zero", ptrNumMax);
        return;
    }

    setBuffer(ptrNumMax, new (pHeap, alignment, std::nothrow) u8[s32(sizeof(void*)) * ptrNumMax]);
}

/**
 * Allocates the buffer for the pointers if there is room.
 * @param ptrNumMax number of pointers to make room for
 * @param pHeap heap to allocate from
 * @param alignment buffer alignment
 * @return whether the allocation succeeded
 */
bool PtrArrayImpl::tryAllocBuffer(s32 ptrNumMax, Heap* pHeap, s32 alignment)
{
    SEAD_ASSERT(mPtrs == nullptr);

    if (ptrNumMax < 1)
    {
        SEAD_ASSERT_MSG(false, "ptrNumMax[%d] must be larger than zero", ptrNumMax);
        return false;
    }

    auto* buf = new (pHeap, alignment, std::nothrow) u8[s32(sizeof(void*)) * ptrNumMax];
    if (!buf)
    {
        return false;
    }

    setBuffer(ptrNumMax, buf);
    return true;
}

/**
 * Frees the buffer allocated with allocBuffer() and empties the array.
 */
void PtrArrayImpl::freeBuffer()
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
 * Removes pointers, moving the ones after them forward.
 * @param pos position of the first pointer to remove
 * @param count number of pointers to remove
 */
void PtrArrayImpl::erase(s32 pos, s32 count)
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

    if (pos + count > mPtrNum)
    {
        SEAD_ASSERT_MSG(false, "pos[%d] + num[%d] exceed size[%d]", pos, count, mPtrNum);
        return;
    }

    const s32 endPos = pos + count;
    if (mPtrNum > endPos)
    {
        MemUtil::copyOverlap(mPtrs + pos, mPtrs + endPos, sizeof(void*) * (mPtrNum - endPos));
    }

    mPtrNum -= count;
}

/**
 * Reverses the order of the pointers.
 */
void PtrArrayImpl::reverse()
{
    const s32 half = mPtrNum / 2;
    for (s32 i = 0; i < half; ++i)
    {
        swap(mPtrNum - i - 1, i);
    }
}

// Fisher–Yates shuffle.
/**
 * Shuffles the pointers (Fisher-Yates).
 * @param pRandom random number generator to use
 */
void PtrArrayImpl::shuffle(Random* pRandom)
{
    SEAD_ASSERT(pRandom);
    for (s32 i = mPtrNum; i > 1; --i)
    {
        swap(i - 1, pRandom->getS32Range(0, i));
    }
}

/**
 * Inserts a pointer, moving the ones after it back.
 * @param pos position to insert at
 * @param pPtr pointer to insert
 */
void PtrArrayImpl::insert(s32 pos, void* pPtr)
{
    if (!checkInsert(pos, 1))
    {
        return;
    }

    createVacancy(pos, 1);
    mPtrs[pos] = pPtr;
    ++mPtrNum;
}

/**
 * Checks that pointers can be inserted at a position.
 * @param pos position to insert at
 * @param num number of pointers to insert
 * @return whether there is room and pos is valid
 */
bool PtrArrayImpl::checkInsert(s32 pos, s32 num)
{
    if (pos < 0)
    {
        SEAD_ASSERT_MSG(false, "illegal position[%d]", pos);
        return false;
    }

    if (mPtrNum + num > mPtrNumMax)
    {
        SEAD_ASSERT_MSG(false, "list is full.");
        return false;
    }

    if (mPtrNum < pos)
    {
        SEAD_ASSERT_MSG(false, "pos[%d] exceed size[%d]", pos, mPtrNum);
        return false;
    }

    return true;
}

/**
 * Inserts pointers to each element of an array, moving the pointers after them back.
 * @param pos position to insert at
 * @param pArray first element
 * @param arrayLength number of elements
 * @param elemSize size of one element
 */
void PtrArrayImpl::insertArray(s32 pos, void* pArray, s32 arrayLength, s32 elemSize)
{
    if (!checkInsert(pos, arrayLength))
    {
        return;
    }

    createVacancy(pos, arrayLength);
    for (s32 i = 0; i < arrayLength; ++i)
    {
        mPtrs[pos + i] = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pArray) + i * elemSize);
    }
    mPtrNum += arrayLength;
}

}  // namespace sead
