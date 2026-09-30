#include <eui/euiMassDrawPane.h>
#include <eui/euiUtility.h>
#include <nn/ui2d/ui2d_Material.h>
#include <eui/euiDrawInfoEx.h>
#include <common/aglDrawContext.h>
#include <common/aglTextureSampler.h>
#include <utility/aglDevTools.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadProjection.h>
#include <math/seadMatrix.hpp>

namespace eui {

namespace {
// rDestination receives the common prefix of rSource without changing its allocation.
template <typename T>
void copyBuffer(const sead::Buffer<T>& rSource, sead::Buffer<T>& rDestination) {
    if (&rDestination == &rSource) return;
    const int count = rSource.size() < rDestination.size() ? rSource.size() : rDestination.size();
    const T* source = rSource.getBufferPtr();
    T* destination = rDestination.getBufferPtr();
    for (int i = 0; i < count; ++i) destination[i] = source[i];
}
}

// textureCount is the number of texture slots in the picture material.
MassDrawPane::MassDrawPane(u8 textureCount) : PictureEx(textureCount) {}

// rTexture initializes the shared texture used by the repeated pictures.
MassDrawPane::MassDrawPane(const nn::ui2d::TextureInfo& rTexture) : PictureEx(rTexture) {}

// pResource contains the picture; pOverride supplies resource overrides;
// rArgs provides the layout build context.
MassDrawPane::MassDrawPane(const nn::ui2d::ResPicture* pResource,
    const nn::ui2d::ResPicture* pOverride, const nn::ui2d::BuildArgSet& rArgs)
    : PictureEx(pResource, pOverride, rArgs) {}

MassDrawPane::~MassDrawPane() = default;

// rOther supplies the picture and instance buffers to duplicate into the layout heap.
MassDrawPane::MassDrawPane(const MassDrawPane& rOther) : PictureEx(rOther) {
    if (rOther.mAlphas.getBufferPtr()) {
        initialize(GetNwAllocatorHeap(), rOther.mAlphas.size());
        copyBuffer(rOther.mAlphas, mAlphas);
        copyBuffer(rOther.mIndices, mIndices);
        copyBuffer(rOther.mPositions, mPositions);
    }
}

// pHeap owns the instance buffers; count is the number of repeated pictures.
void MassDrawPane::initialize(sead::Heap* pHeap, int count) {
    mAlphas.tryAllocBuffer(count, pHeap);
    mAlphas.fill(0);
    mIndices.tryAllocBuffer(count, pHeap);
    mIndices.fill(0);
    mPositions.tryAllocBuffer(count, pHeap);
    mPositions.fill(sead::Vector2f::zero);
}

// rDrawInfo and rContext provide calculation state; force forwards the update request.
void MassDrawPane::Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext,
                            bool force) {
    Pane::Calculate(rDrawInfo, rContext, force);
}

// Convert the first material's texture wrapper into the agl texture description.
void MassDrawPane::initializeTextureData_() {
    const auto* textureInfo = Pane::GetMaterial()->GetFirstTexMap()[0].m_pTextureInfo;
    SetupAglTextureDataByTextureInfo(&mTexture, *textureInfo);
}

// rDrawInfo supplies the render target and context; rCommands is unused by the agl drawing path.
void MassDrawPane::DrawSelf(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) {
    const auto* info = static_cast<DrawInfoEx&>(rDrawInfo).m_pRenderBufferInfo;
    if (!info || !mAlphas.getBufferPtr()) return;
    if (!mTextureInitialized) {
        initializeTextureData_();
        mTextureInitialized = true;
    }
    info->pGraphicsContext->apply(info->pDrawContext);
    const float width = mSizeX;
    const float height = mSizeY;
    const u32 textureWidth = mTexture.getWidth(0);
    const u32 textureHeight = mTexture.getHeight(0);
    sead::Vector2f uvOffset(0, 0);
    agl::TextureSampler sampler(mTexture);
    const float scaleX = mGlobalMtx[0];
    const float scaleY = mGlobalMtx[5];
    const float paneWidth = mSizeX;
    const float paneHeight = mSizeY;
    const float translateX = mGlobalMtx[3];
    const float translateY = mGlobalMtx[7];
    sead::OrthoProjection projection(0, 300, *info->pViewport);
    const u8* white = Pane::GetMaterial()->GetWhiteColor();
    sead::Color4f color(white[0] / 255.0f, white[1] / 255.0f, white[2] / 255.0f, white[2] / 255.0f);
    sead::Vector2f uvScale(width / textureWidth, height / textureHeight);
    const float indexCenter = float(textureWidth / u32(width) - 1) * 0.5f;
    sead::Matrix34f baseMatrix;
    baseMatrix.makeST(sead::Vector3f(scaleX * paneWidth, scaleY * paneHeight, 0),
                      sead::Vector3f(translateX, translateY, 0));
    const float paneAlpha = mAlphaInfluence / 255.0f;
    const auto* positions = mPositions.getBufferPtr();
    const auto* indices = mIndices.getBufferPtr();
    const auto* alphas = mAlphas.getBufferPtr();
    const u32 count = mAlphas.size();
    for (u32 i = 0; i < count; ++i) {
        const u8 alpha = alphas[i];
        if (!alpha) continue;
        sead::Matrix34f matrix = baseMatrix;
        matrix.setTranslation(scaleX * positions[i].x + matrix(0, 3),
                              scaleY * positions[i].y + matrix(1, 3), 0);
        color.a = paneAlpha * (alpha / 255.0f);
        uvOffset.x = float(indices[i]) - indexCenter;
        agl::utl::DevTools::drawTextureTexCoordMultColor(
            static_cast<agl::DrawContext*>(info->pDrawContext), sampler, matrix,
            projection.getDeviceProjectionMatrix(), uvScale, 0, uvOffset, color);
    }
    rDrawInfo.ResetDrawState();
}

}  // namespace eui
