#include "environment/aglEnvObjBuffer.h"

namespace agl::env {

namespace {

s32 compareEnvObj(EnvObj* const* ppLhs, EnvObj* const* ppRhs)
{
    if (!*ppLhs)
    {
        return 1;
    }
    if (!*ppRhs)
    {
        return -1;
    }
    return (*ppLhs)->getEnvObjName().compare((*ppRhs)->getEnvObjName()) < 0 ? -1 : 1;
}

}  // namespace

/**
 * Constructs an allocation argument with no object of any type.
 */
EnvObjBuffer::AllocateArg::AllocateArg()
{
    for (auto& rCount : mCounts)
    {
        rCount = 0;
    }
}

/**
 * Sets the maximum number of objects of a type.
 * @param type type index
 * @param count maximum number of objects
 */
void EnvObjBuffer::AllocateArg::setContainMax(int type, int count)
{
    mTotal -= mCounts[type];
    mCounts[type] = count;
    mTotal += count;
}

/**
 * Constructs an empty buffer.
 */
EnvObjBuffer::EnvObjBuffer() = default;

/**
 * Frees the buffer.
 */
EnvObjBuffer::~EnvObjBuffer()
{
    mObj.freeBuffer();
    mTypeRange.freeBuffer();
}

/**
 * Allocates the object slots of every type.
 * @param rArg number of objects of each type
 * @param pHeap heap to allocate from
 */
void EnvObjBuffer::allocBuffer(const AllocateArg& rArg, sead::Heap* pHeap)
{
    mObj.tryAllocBuffer(rArg.getTotal(), pHeap);
    mTypeRange.tryAllocBuffer(EnvObj::sTypeNum, pHeap);

    s32 start = 0;
    for (auto it = mTypeRange.begin(), itEnd = mTypeRange.end(); it != itEnd; ++it)
    {
        it->mStart = start;
        it->mNum = rArg.getCount(it.getIndex());
        start += it->mNum;
    }
    for (auto& rObj : mObj)
    {
        rObj = nullptr;
    }
}

/**
 * Frees the object slots.
 */
void EnvObjBuffer::freeBuffer()
{
    mObj.freeBuffer();
    mTypeRange.freeBuffer();
}

/**
 * Searches an object of a type by name.
 * @param type type index
 * @param rName object name
 * @return index of the object within its type, or -1 if it is not found
 */
s32 EnvObjBuffer::searchTypeIndex(s32 type, const sead::SafeString& rName) const
{
    s32 index = searchBufferIndex(type, rName);
    if (index < 0)
    {
        return -1;
    }
    return index - mTypeRange[type].mStart;
}

/**
 * Searches an object of a type by name.
 * @param type type index
 * @param rName object name
 * @return index of the object in the buffer, or -1 if it is not found
 */
s32 EnvObjBuffer::searchBufferIndex(s32 type, const sead::SafeString& rName) const
{
    for (auto it = begin(type), itEnd = end(type); it != itEnd; ++it)
    {
        if (*it && (*it)->getEnvObjName() == rName)
        {
            return it.getIndex();
        }
    }
    return -1;
}

/**
 * Searches an object.
 * @param pObj object to search
 * @return index of the object within its type, or -1 if it is not found
 */
s32 EnvObjBuffer::searchTypeIndex(const EnvObj* pObj) const
{
    if (!pObj)
    {
        return -1;
    }

    s32 type = pObj->getTypeID();
    for (auto it = begin(type), itEnd = end(type); it != itEnd; ++it)
    {
        if (*it == pObj)
        {
            return it.getIndex() - mTypeRange[type].mStart;
        }
    }
    return -1;
}

/**
 * Gets the type of the object stored at an index of the buffer.
 * @param bufferIndex index in the buffer
 * @return type index, or -1 if the index is out of range
 */
s32 EnvObjBuffer::searchType(s32 bufferIndex) const
{
    for (s32 i = 0; i < EnvObj::sTypeNum; i++)
    {
        const TypeRange& rRange = mTypeRange[i];
        if (rRange.mStart <= bufferIndex && bufferIndex < rRange.mStart + rRange.mNum)
        {
            return i;
        }
    }
    return -1;
}

/**
 * Enables or disables every object of a type.
 * @param type type index
 * @param enable whether the objects are enabled
 */
void EnvObjBuffer::setEnable(s32 type, bool enable)
{
    for (auto it = begin(type), itEnd = end(type); it != itEnd; ++it)
    {
        if (*it)
        {
            (*it)->setEnable(enable);
        }
    }
}

/**
 * Sorts the objects of a type by name.
 * @param type type index
 */
void EnvObjBuffer::sort(s32 type)
{
    const TypeRange& rRange = mTypeRange[type];
    if (rRange.mNum != 0)
    {
        mObj.heapSort(compareEnvObj, rRange.mStart, rRange.mStart + rRange.mNum - 1);
    }
}

/**
 * Enables or disables every object.
 * @param enable whether the objects are enabled
 */
void EnvObjBuffer::setEnableAll(bool enable)
{
    for (auto* pObj : mObj)
    {
        if (pObj)
        {
            pObj->setEnable(enable);
        }
    }
}

}  // namespace agl::env
