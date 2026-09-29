#include "utility/aglPrimitiveVertex.h"
#include "common/aglGPUMemAddr.h"

namespace agl::utl
{

SEAD_SINGLETON_DISPOSER_IMPL(PrimitiveVertex)

/**
 * Constructs the primitive vertex buffers without any storage.
 */
PrimitiveVertex::PrimitiveVertex() = default;

/**
 * Destroys the primitive vertex buffers.
 */
PrimitiveVertex::~PrimitiveVertex() = default;

static sead::Vector4f* getBufferPtr_(const GPUMemBlockBase& rBlock)
{
    return reinterpret_cast<sead::Vector4f*>(
        static_cast<u8*>(nvnMemoryPoolMap(rBlock.getMemoryPool()->getDriverPool())) +
        rBlock.getByteOffset());
}

/**
 * Allocates the constant vertex buffers (white, black and zero).
 * @param pHeap heap used for the GPU memory
 */
void PrimitiveVertex::initialize(sead::Heap* pHeap)
{
    const auto setUp = [this, pHeap](Type type, const sead::Vector4f& rValue) {
        mBlocks[type].allocBuffer(1, pHeap, 8, MemoryAttribute::Default);
        const GPUMemAddr<sead::Vector4f> addr(mBlocks[type], 0);
        *getBufferPtr_(mBlocks[type]) = rValue;
        mVertexBuffers[type].setUpBuffer(ConstGPUMemVoidAddr(mBlocks[type], 0), 0,
                                         sizeof(sead::Vector4f));
        mVertexBuffers[type].setUpStream(0, VertexStreamFormat(0x2e), 0, false);
    };

    setUp(cType_White, {1.0f, 1.0f, 1.0f, 1.0f});
    setUp(cType_Black, {0.0f, 0.0f, 0.0f, 1.0f});
    setUp(cType_Zero, {0.0f, 0.0f, 0.0f, 0.0f});
}

}  // namespace agl::utl
