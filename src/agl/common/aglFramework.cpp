#include "common/aglInitArg.h"

#include <heap/seadHeapMgr.h>

#include "detail/aglDynamicUniformBlock.h"
#include "detail/aglFileIOMgr.h"
#include "detail/aglGPUMemBlockMgr.h"
#include "detail/aglPrivateResource.h"
#include "detail/aglRootNode.h"
#include "detail/aglShaderHolder.h"
#include "driver/aglNVNMgr.h"
#include "utility/aglDebugTextureDrawer.h"
#include "utility/aglDynamicTextureAllocator.h"
#include "utility/aglPrimitiveShape.h"
#include "utility/aglPrimitiveTexture.h"
#include "utility/aglPrimitiveVertex.h"
#include "utility/aglVertexAttributeHolder.h"

namespace agl {

namespace {

/**
 * Resolves the heap used for initialization.
 * @param pHeap heap given in the init argument
 * @return pHeap, or the current heap when it is nullptr
 */
sead::Heap* getHeap(sead::Heap* pHeap)
{
    if (pHeap == nullptr)
    {
        pHeap = sead::HeapMgr::instance()->getCurrentHeap();
    }

    pHeap->getFreeSize();
    return pHeap;
}

}  // namespace

/**
 * Constructs the init argument with the default sizes.
 */
InitArg::InitArg() = default;

/**
 * Creates and initializes every agl singleton.
 * @param rArg init argument
 */
void Initialize(const InitArg& rArg)
{
    sead::Heap* pArgHeap = rArg.mHeap;
    {
        sead::Heap* pHeap = getHeap(pArgHeap);
        detail::GPUMemBlockMgr::createInstance(pHeap);
        detail::GPUMemBlockMgr::instance()->initialize(pHeap, rArg.mDebugHeap);
        detail::GPUMemBlockMgr::instance()->setMinBlockSize(rArg.mMinGPUMemBlockSize);
    }

    {
        sead::Heap* pHeap = getHeap(pArgHeap);
        driver::NVNMgr::createInstance(pHeap);
        driver::NVNMgr::instance()->initialize(pHeap, rArg.mDebugHeap);
    }

    {
        sead::Heap* pHeap = getHeap(pArgHeap);
        detail::PrivateResource::createInstance(pHeap);
        detail::PrivateResource::instance()->initialize(pHeap, rArg.mDebugHeap, rArg.mWorkHeapSize,
                                                        rArg._18);
    }

    {
        sead::Heap* pHeap = getHeap(pArgHeap);
        detail::RootNode::createInstance(pHeap);
        detail::RootNode::instance()->initialize(pHeap, rArg.mRootNodeMetaSuffix);
    }

    {
        sead::Heap* pHeap = getHeap(pArgHeap);
        detail::ShaderHolder::createInstance(pHeap);
        detail::ShaderHolder::instance()->setNoOption(rArg.mShaderNoOption);
    }

    {
        sead::Heap* pHeap = getHeap(pArgHeap);
        utl::DynamicTextureAllocator::createInstance(pHeap);
        utl::DynamicTextureAllocator::instance()->initialize(
            rArg.mDynamicTextureNum, rArg.mDynamicTextureSize, rArg.mDynamicTextureDebugSize, pHeap,
            rArg.mDebugHeap);
    }

    {
        sead::Heap* pHeap = getHeap(pArgHeap);
        detail::DynamicUniformBlock::createInstance(pHeap);
        detail::DynamicUniformBlock::instance()->initialize(rArg.mDynamicUniformBlockSize, pHeap);
    }

    if (rArg.mDebugHeap != nullptr)
    {
        utl::DebugTextureDrawer::createInstance(rArg.mDebugHeap);
        utl::DebugTextureDrawer::instance()->initialize(rArg.mDebugHeap);
    }

    {
        sead::Heap* pHeap = getHeap(pArgHeap);
        utl::PrimitiveTexture::createInstance(pHeap);
        utl::PrimitiveTexture::instance()->initialize(pHeap);
    }

    {
        sead::Heap* pHeap = getHeap(pArgHeap);
        utl::PrimitiveShape::createInstance(pHeap);
        utl::PrimitiveShape::instance()->initialize(pHeap);
    }

    {
        sead::Heap* pHeap = getHeap(pArgHeap);
        utl::VertexAttributeHolder::createInstance(pHeap);
        utl::VertexAttributeHolder::instance()->initialize(pHeap);
    }

    {
        sead::Heap* pHeap = getHeap(pArgHeap);
        utl::PrimitiveVertex::createInstance(pHeap);
        utl::PrimitiveVertex::instance()->initialize(pHeap);
    }

    {
        detail::FileIOMgr::CreateArg createArg;
        createArg.mUseCheckout = rArg.mUseCheckout;
        detail::FileIOMgr::createInstance(pArgHeap);
        detail::FileIOMgr::instance()->initialize(createArg, rArg.mDebugHeap);
    }
}

/**
 * Appends the agl root node to an OR node (empty in release builds).
 * @param pNode parent node
 */
void AppendRootNodeToOR(sead::hostio::Node* pNode) {}

/**
 * Loads the agl resource archive and the built-in shaders.
 * @param pArchive agl resource archive
 */
void LoadResource(sead::ArchiveRes* pArchive)
{
    sead::Heap* pHeap = detail::PrivateResource::instance()->getWorkHeap();
    detail::PrivateResource::instance()->createArchive(pArchive);
    detail::ShaderHolder::instance()->initialize(pArchive, pHeap);
}

/**
 * Destroys every agl singleton.
 */
void Finalize()
{
    utl::VertexAttributeHolder::deleteInstance();
    detail::ShaderHolder::deleteInstance();
    utl::PrimitiveTexture::deleteInstance();
    utl::PrimitiveVertex::deleteInstance();
    utl::PrimitiveShape::deleteInstance();
    utl::DynamicTextureAllocator::deleteInstance();
    utl::DebugTextureDrawer::deleteInstance();
    detail::FileIOMgr::deleteInstance();
    detail::PrivateResource::deleteInstance();
    detail::RootNode::deleteInstance();
    detail::GPUMemBlockMgr::deleteInstance();
    driver::GraphicsDriverMgr::deleteInstance();
}

}  // namespace agl
