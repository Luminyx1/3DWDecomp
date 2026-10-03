#pragma once

#include <basis/seadNew.h>
#include <basis/seadTypes.h>

namespace sead
{
class Heap;

/**
 * Two dimensional buffer of objects (width x height), stored row by row.
 */
template <typename T>
class Buffer2
{
public:
    Buffer2() = default;

    /**
     * Allocates width * height objects.
     * @param width number of columns
     * @param height number of rows
     * @param pHeap heap to allocate from (nullptr for the current heap)
     * @param alignment allocation alignment
     * @return true if the buffer was allocated
     */
    bool tryAllocBuffer(s32 width, s32 height, Heap* pHeap, s32 alignment = sizeof(void*))
    {
        if (width > 0 && height > 0)
        {
            T* pBuffer = new (pHeap, alignment, std::nothrow) T[width * height];

            if (pBuffer != nullptr)
            {
                mWidth = width;
                mHeight = height;
                mBuffer = pBuffer;
                return true;
            }
        }

        return false;
    }

    /**
     * Frees the buffer.
     */
    void freeBuffer()
    {
        if (mBuffer != nullptr)
        {
            delete[] mBuffer;
            mBuffer = nullptr;
            mWidth = 0;
            mHeight = 0;
        }
    }

    /**
     * Gets the object at a position.
     * @param x column
     * @param y row
     * @return the object, or nullptr when out of range
     */
    T* get(s32 x, s32 y) const
    {
        if (u32(x) < u32(mWidth) && u32(y) < u32(mHeight))
        {
            return &mBuffer[mWidth * y + x];
        }

        return nullptr;
    }

    s32 getWidth() const { return mWidth; }
    s32 getHeight() const { return mHeight; }
    T* getBufferPtr() const { return mBuffer; }

private:
    s32 mWidth = 0;
    s32 mHeight = 0;
    T* mBuffer = nullptr;
};

}  // namespace sead
