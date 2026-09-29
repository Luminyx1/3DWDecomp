#include "common/aglVertexAttribute.h"

#include <cstring>
#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglDrawContext.h"
#include "common/aglVertexBuffer.h"

namespace agl {

/**
 * Constructs a vertex attribute set without vertex buffer slots.
 */
VertexAttribute::VertexAttribute() : mFlags(0) {}

/**
 * Unbinds all vertex buffers and frees the vertex buffer slots.
 */
VertexAttribute::~VertexAttribute()
{
    cleanUp();
    destroy();
}

/**
 * Unbinds all vertex buffers from the attributes and slots.
 */
void VertexAttribute::cleanUp()
{
    for (s32 i = 0; i < cVertexAttributeMax; i++)
    {
        mAttributes[i].mVertexBuffer = nullptr;
    }
    u32 bufferNum = mVertexBuffers.size();
    if (bufferNum != 0)
    {
        std::memset(mVertexBuffers.getBufferPtr(), 0, bufferNum * sizeof(const VertexBuffer*));
    }
    mFlags.reset(cFlag_SetUp);
}

/**
 * Frees the vertex buffer slots allocated by create.
 */
void VertexAttribute::destroy()
{
    if (!mFlags.isOn(cFlag_Created))
    {
        return;
    }

    mVertexBuffers.freeBuffer();
    mFlags.makeAllZero();
}

/**
 * Allocates vertex buffer slots.
 * @param bufferNum number of vertex buffer slots
 * @param pHeap heap to allocate from
 */
void VertexAttribute::create(u32 bufferNum, sead::Heap* pHeap)
{
    if (mVertexBuffers.tryAllocBuffer(bufferNum, pHeap))
    {
        cleanUp();
        mFlags.set(cFlag_Created);
    }
}

/**
 * Binds a stream of a vertex buffer to an attribute location, or unbinds it.
 * @param location attribute location, or -1 to do nothing
 * @param pVertexBuffer vertex buffer, or nullptr to unbind
 * @param streamIndex stream index inside the vertex buffer
 */
void VertexAttribute::setVertexStream(s32 location, const VertexBuffer* pVertexBuffer,
                                      u32 streamIndex)
{
    if (location == -1)
    {
        return;
    }

    Attribute_& rAttribute = mAttributes[location];
    if (rAttribute.mVertexBuffer)
    {
        rAttribute.mBufferIndex = disableVertexBuffer_(&rAttribute);
    }
    if (pVertexBuffer)
    {
        rAttribute.mBufferIndex = enableVertexBuffer_(&rAttribute, pVertexBuffer, streamIndex);
    }
}

/**
 * Unbinds the vertex buffer of an attribute and frees its slot if no other attribute uses it.
 * @param pAttribute attribute
 * @return -1
 */
s32 VertexAttribute::disableVertexBuffer_(Attribute_* pAttribute)
{
    const VertexBuffer* pVertexBuffer = pAttribute->mVertexBuffer;
    pAttribute->mVertexBuffer = nullptr;
    pAttribute->mStreamIndex = 1;

    for (s32 i = 0; i < cVertexAttributeMax; i++)
    {
        if (pVertexBuffer == mAttributes[i].mVertexBuffer)
        {
            return -1;
        }
    }

    mVertexBuffers[pAttribute->mBufferIndex] = nullptr;
    return -1;
}

/**
 * Binds a vertex buffer to an attribute and assigns it a slot.
 * @param pAttribute attribute
 * @param pVertexBuffer vertex buffer
 * @param streamIndex stream index inside the vertex buffer
 * @return slot index of the vertex buffer
 */
s32 VertexAttribute::enableVertexBuffer_(Attribute_* pAttribute,
                                         const VertexBuffer* pVertexBuffer, u32 streamIndex)
{
    pAttribute->mVertexBuffer = pVertexBuffer;
    pAttribute->mStreamIndex = streamIndex;

    s32 index = -1;
    for (s32 i = mVertexBuffers.size() - 1; i >= 0; i--)
    {
        if (mVertexBuffers[i] == pVertexBuffer)
        {
            return i;
        }
        if (!mVertexBuffers[i])
        {
            index = i;
        }
    }

    mVertexBuffers[index] = pVertexBuffer;
    return index;
}

/**
 * Gets the vertex buffer bound to an attribute location.
 * @param location attribute location
 * @param pStreamIndex receives the stream index, may be nullptr
 * @return bound vertex buffer, or nullptr
 */
const VertexBuffer* VertexAttribute::getVertexStream(s32 location, u32* pStreamIndex) const
{
    if (location == -1)
    {
        return nullptr;
    }

    if (pStreamIndex)
    {
        *pStreamIndex = mAttributes[location].mStreamIndex;
    }
    return mAttributes[location].mVertexBuffer;
}

/**
 * Builds the NVN attribute and stream states from the bound vertex buffers.
 */
void VertexAttribute::setUp()
{
    for (s32 i = 0; i < cVertexAttributeMax; i++)
    {
        nvnVertexAttribStateSetDefaults(&mAttribStates[i]);
        nvnVertexStreamStateSetDefaults(&mStreamStates[i]);
    }

    for (s32 i = 0; i < cVertexAttributeMax; i++)
    {
        const VertexBuffer* pVertexBuffer = mAttributes[i].mVertexBuffer;
        if (!pVertexBuffer)
        {
            continue;
        }

        const VertexBuffer::Stream& rStream =
            pVertexBuffer->getStream(mAttributes[i].mStreamIndex);
        nvnVertexAttribStateSetFormat(&mAttribStates[i], NVNformat(rStream.mFormat),
                                      rStream.mOffset);
        nvnVertexAttribStateSetStreamIndex(&mAttribStates[i], mAttributes[i].mBufferIndex);
        if (pVertexBuffer->getStream(mAttributes[i].mStreamIndex).mDivisor)
        {
            nvnVertexStreamStateSetDivisor(&mStreamStates[mAttributes[i].mBufferIndex], 1);
        }
    }

    u32 bufferNum = mVertexBuffers.size();
    const VertexBuffer* const* ppVertexBuffer = mVertexBuffers.getBufferPtr();
    for (u32 i = 0; i < bufferNum; i++)
    {
        if (ppVertexBuffer[i])
        {
            nvnVertexStreamStateSetStride(&mStreamStates[i], ppVertexBuffer[i]->getStride());
        }
    }

    mFlags.set(cFlag_SetUp);
}

/**
 * Binds the attribute and stream states and the vertex buffers.
 * @param pDrawContext draw context
 */
void VertexAttribute::activate(DrawContext* pDrawContext) const
{
    NVNcommandBuffer* pCommandBuffer = pDrawContext->getNvnCommandBuffer();
    nvnCommandBufferBindVertexAttribState(pCommandBuffer, cVertexAttributeMax, mAttribStates);
    nvnCommandBufferBindVertexStreamState(pCommandBuffer, cVertexAttributeMax, mStreamStates);

    u32 bufferNum = mVertexBuffers.size();
    const VertexBuffer* const* ppVertexBuffer = mVertexBuffers.getBufferPtr();
    for (u32 i = 0; i < bufferNum; i++)
    {
        const VertexBuffer* pVertexBuffer = ppVertexBuffer[i];
        if (pVertexBuffer)
        {
            nvnCommandBufferBindVertexBuffer(pCommandBuffer, i,
                                             nvnBufferGetAddress(pVertexBuffer->getNvnBuffer()),
                                             pVertexBuffer->getBufferSize());
        }
    }
}

/**
 * Does nothing (attributes are rebound on activate).
 * @param pDrawContext unused
 */
void VertexAttribute::disableAttributeAll(DrawContext* pDrawContext) {}

}  // namespace agl
