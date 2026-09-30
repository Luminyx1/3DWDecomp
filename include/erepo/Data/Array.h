#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <nn/prepo.h>
#include <prim/seadSafeString.h>

namespace sead {
class Heap;
}

namespace erepo {

class Struct;

class Array {
public:
    Array();
    virtual ~Array();

    template <typename T>
    void addData(const T& rValue);

    template <typename T>
    bool constructBuffer(s32 num, sead::Heap* pHeap);
    template <typename T>
    bool constructBuffer(s32 num, s32 length, sead::Heap* pHeap);
    bool constructStructBuffer(s32 num, s32 memberNum, sead::Heap* pHeap);
    bool constructStructBufferWithBufferSize(s32 num, s32 bufferSize, sead::Heap* pHeap);

    Struct* CreateStruct(s32 memberNum);
    Struct* CreateStructWithBufferSize(s32 bufferSize);

    const nn::prepo::Array& getArray() const { return mArray; }

private:
    bool constructBuffer_(size_t size, sead::Heap* pHeap)
    {
        if (!mBuffer.tryAllocBuffer(size, pHeap)) {
            return false;
        }

        mArray.SetBuffer(mBuffer.getBufferPtr(), size);
        return true;
    }

    nn::prepo::Array mArray;
    sead::Buffer<u8> mBuffer;
    sead::Buffer<Struct*> mStructs;
};

}  // namespace erepo
