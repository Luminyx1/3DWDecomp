#include "Library/Play/Draw/RenderVariables.hpp"

#include <common/aglGPUMemAddr.h>
#include <gfx/seadColor.h>
#include <lighting/aglLightPrePass.h>
#include <math/seadBoundBox.h>
#include <math/seadMathCalcCommon.h>
#include <utility/aglDynamicTextureAllocator.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Project/Draw/GraphicsStressDirector.hpp"

namespace {
/**
 * @brief Gets the draw context of the game framework.
 * @return The agl draw context.
 */
agl::DrawContext* getDrawContext() {
    return al::GameFrameworkNx::getAglDrawContext();
}

/**
 * @brief Sizes a render buffer to cover a whole texture.
 * @param pRenderBuffer Render buffer to resize.
 * @param width Texture width.
 * @param height Texture height.
 */
template <typename T>
void setRenderBufferSize(agl::RenderBuffer* pRenderBuffer, T width, T height) {
    pRenderBuffer->setVirtualSize(sead::Vector2f(width, height));
    pRenderBuffer->setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, width, height));
}


/**
 * @brief Fast-clears the first color target of a render buffer over its whole area.
 * @param rRenderBuffer Render buffer to clear.
 * @param pDrawContext Draw context to clear with.
 * @param rColor Clear color.
 */
void fastClearColor(const agl::RenderBuffer& rRenderBuffer, agl::DrawContext* pDrawContext,
                    const sead::Color4f& rColor) {
    rRenderBuffer.fastClear(pDrawContext, 0, 1, rColor, 1.0f, 0, sead::Viewport(rRenderBuffer),
                            true);
}
}  // namespace

namespace al {

/**
 * @brief Sets up the render variables of one view and optionally allocates its depth buffer.
 * @param pGraphicsSystemInfo Graphics system info.
 * @param pLiveActorKit Live actor kit of the scene.
 * @param pSimpleModelEnv Simple model environment.
 * @param viewIndex Index of the view rendered with these variables.
 * @param width Requested buffer width.
 * @param height Requested buffer height.
 * @param isFixedSize Whether to keep the requested size instead of the stress director's size.
 * @param isAllocDepthBuffer Whether the depth buffer is owned and allocated here.
 */
RenderVariables::RenderVariables(GraphicsSystemInfo* pGraphicsSystemInfo,
                                 LiveActorKit* pLiveActorKit,
                                 const SimpleModelEnv* pSimpleModelEnv, s32 viewIndex, s32 width,
                                 s32 height, bool isFixedSize, bool isAllocDepthBuffer)
    : mGraphicsSystemInfo(pGraphicsSystemInfo), mLiveActorKit(pLiveActorKit),
      mSimpleModelEnv(pSimpleModelEnv), mViewIndex(viewIndex),
      mIsAllocDepthBuffer(isAllocDepthBuffer) {
    if (mViewIndex == 0 && !isFixedSize) {
        const GraphicsStressDirector* stress = mGraphicsSystemInfo->getGraphicsStressDirector();
        width = sead::Mathi::min(width, stress->getBufferSizeX());
        height = sead::Mathi::min(height, stress->getBufferSizeY());
    }

    mWidth = width;
    mHeight = height;
    mViewport = sead::Viewport(0.0f, 0.0f, width, height);

    if (mIsAllocDepthBuffer) {
        allocDepthBuffer();
    }
}

/**
 * @brief Allocates the depth texture and its Z-cull buffer, and binds them to the depth target.
 */
void RenderVariables::allocDepthBuffer() {
    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();
    const GraphicsStressDirector* stress = mGraphicsSystemInfo->getGraphicsStressDirector();
    agl::TextureFormat format =
        agl::TextureFormat(stress->getCurrentParam().isUsing16BitDepth() ? 0x3b : 0x3e);
    agl::GPUMemVoidAddr zCullBuffer;
    mDepthTexture = allocator->alloc(getDrawContext(), "Depth Target", format, mWidth, mHeight, 1,
                                     &zCullBuffer,
                                     agl::utl::DynamicTextureAllocator::cAllocateType_0, true,
                                     false);
    mDepthTarget.applyTextureData(*mDepthTexture, 0, 0);
    mDepthTarget.setZCullBuffer(agl::GPUMemVoidAddr(zCullBuffer));
    mDepthSampler.applyTextureData(*mDepthTexture);
}

/**
 * @brief Releases the light pre-pass buffer and frees the owned textures.
 */
RenderVariables::~RenderVariables() {
    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();

    if (mLightPrePass != nullptr) {
        mLightPrePass->release(mViewIndex);
    } else if (mIsAllocColorBuffer) {
        allocator->free(mColorTexture);
        mColorTexture = nullptr;
    }

    if (mIsAllocDepthBuffer) {
        allocator->free(mDepthTexture);
        mDepthTexture = nullptr;

        if (mSubTexture != nullptr) {
            allocator->free(mSubTexture);
        }

        mSubTexture = nullptr;
    }
}

/**
 * @brief Sets up the color buffer and binds the render buffer.
 * @param pLightPrePass Light pre-pass whose light buffer is used as the color texture, or
 * nullptr to allocate a dedicated texture.
 * @param isClear Whether to clear the color buffer after binding it.
 */
void RenderVariables::allocColorBuffer(agl::lght::LightPrePass* pLightPrePass, bool isClear) {
    if (mColorTexture != nullptr && pLightPrePass == nullptr) {
        return;
    }

    mLightPrePass = pLightPrePass;

    if (pLightPrePass != nullptr) {
        mColorTexture = const_cast<agl::TextureData*>(
            &pLightPrePass->getContext(mViewIndex).mLightBufferSampler.getTextureData());
    } else {
        mColorTexture = agl::utl::DynamicTextureAllocator::instance()->alloc(
            getDrawContext(), "Deferred Shading Target", agl::TextureFormat(0x1a), mWidth,
            mHeight, 1, nullptr, agl::utl::DynamicTextureAllocator::cAllocateType_0, true,
            false);
    }

    mColorTarget.applyTextureData(*mColorTexture);
    setRenderBufferSize(&mRenderBuffer, mWidth, mHeight);
    mRenderBuffer.setRenderTargetColorNullAll();
    mRenderBuffer.setRenderTargetColor(&mColorTarget);
    mRenderBuffer.setRenderTargetDepth(&mDepthTarget);
    bindRenderBuffer();

    if (isClear) {
        fastClearColor(mRenderBuffer, getDrawContext(), sead::Color4f(0.0f, 0.0f, 0.0f, 0.0f));
    }

    mIsAllocColorBuffer = true;
}

/**
 * @brief Binds the render buffer and applies the viewport.
 */
void RenderVariables::bindRenderBuffer() const {
    mRenderBuffer.bind(getDrawContext());
    mViewport.apply(getDrawContext(), mRenderBuffer);
}

}  // namespace al
