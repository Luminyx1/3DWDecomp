#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <prim/seadBitFlag.h>
#include "utility/aglParameterObj.h"

namespace sead {
class Heap;
}

namespace agl::utl {

class ContextParameterBufferImpl {
public:
    ContextParameterBufferImpl();
    ~ContextParameterBufferImpl();

    void initialize(s32 contextNum, bool isSingle, sead::Heap* pHeap);
    void copyParameterToAllContext(s32 srcIndex);
    void copyParameterByIndex(s32 dstIndex, s32 srcIndex);
    void copyParameter(s32 context, const IParameterObj& rObj);
    void copyParameterLerp(s32 context, const IParameterObj& rObjA, const IParameterObj& rObjB,
                           f32 t);

    virtual bool preCopyParameter(s32 index, const IParameterObj* pObjA,
                                  const IParameterObj* pObjB, f32 t)
    {
        return true;
    }
    virtual void postCopyParameter(s32 index, const IParameterObj* pObjA,
                                   const IParameterObj* pObjB, f32 t)
    {
    }

    bool isEnableContext(s32 context) const
    {
        return context == -1 ? mEnableContext.getDirect() : mEnableContext.isOnBit(context);
    }
    s32 getContextNum() const { return mContextNum; }
    s32 getBufferNum() const { return mBufferNum; }
    bool isSingle() const { return mIsSingle; }
    s32 getBufferIndex(s32 context) const { return context & mIndexMask; }
    IParameterObj& getParameterObj(s32 index) { return mParameterObjs[index]; }

protected:
    sead::BitFlag32 mEnableContext;
    s32 mContextNum;
    s32 mBufferNum;
    bool mIsSingle;
    s32 mIndexMask;
    sead::Buffer<IParameterObj> mParameterObjs;
};
static_assert(sizeof(ContextParameterBufferImpl) == 0x30);

template <typename T, typename P>
class ContextParameterBuffer : public ContextParameterBufferImpl, public P {
public:
    ContextParameterBuffer() = default;
    virtual ~ContextParameterBuffer()
    {
        getContextBuffer_().freeBuffer();
        mParameters.freeBuffer();
        mExtraParameters.freeBuffer();
    }

    void initializeContextParameterBuffer(s32 contextNum, bool isSingle, sead::Heap* pHeap);

    const P& getParameter(s32 context) const { return *mParameters[getBufferIndex(context)]; }
    P& getParameter(s32 context) { return *mParameters[getBufferIndex(context)]; }

protected:
    template <typename U = T>
    typename U::Context& getContext_(s32 context) const
    {
        return getContextBuffer_<U>()[context];
    }

    template <typename U = T>
    sead::Buffer<typename U::Context>& getContextBuffer_() const
    {
        return *reinterpret_cast<sead::Buffer<typename U::Context>*>(
            const_cast<ContextStorage*>(&mContexts));
    }

private:
    struct ContextStorage {
        s32 mSize = 0;
        void* mBuffer = nullptr;
    };

    ContextStorage mContexts;
    sead::Buffer<P*> mParameters;
    sead::Buffer<P> mExtraParameters;
};

template <typename T, typename P>
void ContextParameterBuffer<T, P>::initializeContextParameterBuffer(s32 contextNum, bool isSingle,
                                                                    sead::Heap* pHeap)
{
    bool single = isSingle || contextNum < 2;
    initialize(contextNum, single, pHeap);
    getContextBuffer_().tryAllocBuffer(mContextNum, pHeap);
    for (auto& rContext : getContextBuffer_())
    {
        static_cast<T*>(this)->initializeContext(&rContext, pHeap);
    }
    if (!single)
    {
        mExtraParameters.tryAllocBuffer(mBufferNum - 1, pHeap);
    }
    mParameters.tryAllocBuffer(mBufferNum, pHeap);
    for (s32 i = 0; i < mBufferNum; i++)
    {
        mParameters[i] = i == 0 ? static_cast<P*>(this) : &mExtraParameters[i - 1];
        mParameters[i]->initialize(&mParameterObjs[i], pHeap);
    }
}

}  // namespace agl::utl
