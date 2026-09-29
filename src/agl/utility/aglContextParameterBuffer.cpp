#include "utility/aglContextParameterBuffer.h"

namespace agl::utl {

ContextParameterBufferImpl::ContextParameterBufferImpl()
    : mEnableContext(0), mContextNum(0), mBufferNum(0), mIsSingle(true), mIndexMask(0)
{
}

ContextParameterBufferImpl::~ContextParameterBufferImpl()
{
    mParameterObjs.freeBuffer();
}

void ContextParameterBufferImpl::initialize(s32 contextNum, bool isSingle, sead::Heap* pHeap)
{
    mEnableContext = -1;
    mContextNum = contextNum;
    mIsSingle = isSingle;
    if (isSingle)
    {
        mBufferNum = 1;
        mIndexMask = 0;
    }
    else
    {
        mBufferNum = contextNum;
        mIndexMask = -1;
    }
    mParameterObjs.tryAllocBuffer(mBufferNum, pHeap);
}

void ContextParameterBufferImpl::copyParameterToAllContext(s32 srcIndex)
{
    if (mIsSingle)
    {
        return;
    }
    for (s32 i = 0; i < mBufferNum; i++)
    {
        if (i == srcIndex)
        {
            continue;
        }
        if (preCopyParameter(i, &mParameterObjs[srcIndex], nullptr, 0.0f))
        {
            mParameterObjs[i].copy(mParameterObjs[srcIndex]);
            postCopyParameter(i, &mParameterObjs[srcIndex], nullptr, 0.0f);
        }
    }
}

void ContextParameterBufferImpl::copyParameterByIndex(s32 dstIndex, s32 srcIndex)
{
    if (preCopyParameter(dstIndex, &mParameterObjs[srcIndex], nullptr, 0.0f))
    {
        mParameterObjs[dstIndex].copy(mParameterObjs[srcIndex]);
        postCopyParameter(dstIndex, &mParameterObjs[srcIndex], nullptr, 0.0f);
    }
}

void ContextParameterBufferImpl::copyParameter(s32 context, const IParameterObj& rObj)
{
    if (preCopyParameter(context, &rObj, nullptr, 0.0f))
    {
        mParameterObjs[context & mIndexMask].copy(rObj);
        postCopyParameter(context, &rObj, nullptr, 0.0f);
    }
}

void ContextParameterBufferImpl::copyParameterLerp(s32 context, const IParameterObj& rObjA,
                                                   const IParameterObj& rObjB, f32 t)
{
    if (preCopyParameter(context, &rObjA, &rObjB, t))
    {
        mParameterObjs[context & mIndexMask].copyLerp(rObjA, rObjB, t);
        postCopyParameter(context, &rObjA, &rObjB, t);
    }
}

}  // namespace agl::utl
