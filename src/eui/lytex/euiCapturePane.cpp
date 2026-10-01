#include <eui/euiCapturePane.h>
#include <nn/ui2d/ui2d_BuildArgSet.h>
#include <nn/ui2d/ui2d_ExtUserData.h>
#include <utility/aglMultiFilter.h>
#include <eui/euiUtility.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiScreen.h>
#include <eui/euiDrawInfoEx.h>
#include <common/aglDrawContext.h>
#include <common/aglTextureSampler.h>
#include <common/aglTextureFormatInfo.h>
#include <utility/aglDynamicTextureAllocator.h>
#include <utility/aglImageFilter2D.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <math/seadBoundBox.hpp>
#include <nn/ui2d/ui2d_DrawInfo.h>
#include <gfx/seadProjection.h>
#include <math/seadMatrix.hpp>
#include <math/seadMathCalcCommon.hpp>
namespace eui {
namespace {
// format identifies the capture encoding; alpha selects the stored alpha channel.
inline bool NeedsCaptureChannelRemap(agl::TextureFormat format, agl::TextureCompSel alpha) {
    return format == agl::TextureFormat::cTextureFormat_R8_G8_uNorm || alpha == agl::cTextureCompSel_R;
}
}

// pResource supplies pane properties; rArgs supplies the owning layout and build context.
CapturePane::CapturePane(const nn::ui2d::ResPane* pResource, const nn::ui2d::BuildArgSet& rArgs)
    : Pane(pResource, rArgs), mFlags(0), mCaptureRequired(true), mAlwaysCapture(false), mCalculated(false), mClearColor(nullptr),
      mMultiFilter(nullptr), mTexture(nullptr) {
    initialize_(reinterpret_cast<LayoutEx*>(rArgs.m_pPartsLayout));
}

// rOther supplies pane properties and capture policy; pLayout owns the new capture resources.
CapturePane::CapturePane(const CapturePane& rOther, LayoutEx* pLayout)
    : Pane(rOther), mFlags(0), mCaptureRequired(true), mAlwaysCapture(rOther.mAlwaysCapture),
      mCalculated(false), mClearColor(nullptr), mMultiFilter(nullptr), mTexture(nullptr) {
    initialize_(pLayout);
}

// NON_MATCHING: the identity-matrix copy uses different load/store instructions.
// pLayout supplies the screen name, filter resources, and owning screen.
void CapturePane::initialize_(LayoutEx* pLayout) {
    sead::Heap* heap = GetNwAllocatorHeap();
    Pane::mFlags |= 1;
    mClearColor = setupClearColor_(heap, this, pLayout, &mFlags);

    if (FindExtUserDataByName("CaptureWorkFormat") != nullptr) {
        mFlags.set(4);
    }

    setupCaptureOutputAlpha255_(this, &mFlags);

    if (mClearColor == nullptr) {
        mFlags.set(0x20);
    }

    mMultiFilter = InitializeMultiFilter(heap, *this, pLayout);

    if (mMultiFilter != nullptr) {
        mMultiFilter->setUseTextureAlpha(true);
    }

    const char* name = (pLayout->getScreen() != nullptr) ? pLayout->getScreen()->getName().cstr() : pLayout->getLayoutName();
    initializeCaptureTextureData_(heap, name);
    mRenderBuffer.setRenderTargetColor(&mRenderTarget);
    const auto& source = reinterpret_cast<const nn::util::MatrixT4x3fType&>(sead::Matrix34f::ident);
    auto& dest = *reinterpret_cast<nn::util::MatrixT4x3fType*>(mGlobalMtx);
    dest._m.val[0] = source._m.val[0];
    dest._m.val[1] = source._m.val[1];
    dest._m.val[2] = source._m.val[2];
    Pane::mFlags |= 0x40;

    if (pLayout->getScreen() != nullptr) {
        pLayout->getScreen()->mFlags |= 0x10;
    }
}

// pHeap owns the optional color; pPane supplies metadata; pLayout is unused;
// pFlags receives the opaque-background flag when only an alpha of 255 is specified.
sead::Color4f* CapturePane::setupClearColor_(sead::Heap* pHeap, nn::ui2d::Pane* pPane,
                                          LayoutEx* pLayout, sead::BitFlag8* pFlags) {
    const auto* colorData = pPane->FindExtUserDataByName("CaptureBGColor");
    const auto* alphaData = pPane->FindExtUserDataByName("CaptureBGAlpha");

    if (colorData != nullptr && colorData->count == 3) {
        const auto* rgb = static_cast<const s32*>(colorData->GetData());
        auto* color = new (pHeap, 8) sead::Color4f(u32(u8(rgb[0])) / 255.0f, u32(u8(rgb[1])) / 255.0f,
                                                 u32(u8(rgb[2])) / 255.0f, 0);

        if (alphaData != nullptr) {
            color->a = *static_cast<const s32*>(alphaData->GetData()) / 255.0f;
        }

        return color;
    }

    if (alphaData != nullptr && *static_cast<const s32*>(alphaData->GetData()) == 255) {
        pFlags->set(2);
    }

    return nullptr;
}

// pHeap owns the texture and GPU memory; pName labels the capture for graphics debugging.
void CapturePane::initializeCaptureTextureData_(sead::Heap* pHeap, const char* pName) {
    if (mTexture != nullptr) {
        return;
    }

    using Format = agl::TextureFormat;
    Format format = Format::cTextureFormat_R8_G8_B8_A8_uNorm;
    bool alphaOnly = false;
    {
        const sead::SafeString formatName(static_cast<const char*>(FindExtUserDataByName("CaptureOn")->GetData()));

        if (formatName == "RGBA8") format = Format::cTextureFormat_R8_G8_B8_A8_uNorm;
        else if (formatName == "BC3") format = Format::cTextureFormat_BC3_uNorm;
        else if (formatName == "BC1") format = Format::cTextureFormat_BC1_uNorm;
        else if (formatName == "RGB565") format = Format::cTextureFormat_R8_G8_B8_A8_uNorm;
        else if (formatName == "R10G10B10A2") format = Format::cTextureFormat_R10_G10_B10_A2_uNorm;
        else if (formatName == "L8") format = Format::cTextureFormat_R8_uNorm;
        else if (formatName == "A8") {
            format = Format::cTextureFormat_R8_uNorm;
            alphaOnly = true;
        } else if (formatName == "LA8") format = Format::cTextureFormat_R8_G8_uNorm;
        else if (formatName == "BC4L") format = Format::cTextureFormat_BC4_uNorm;
        else if (formatName == "BC4A") {
            format = Format::cTextureFormat_BC4_uNorm;
            alphaOnly = true;
        } else if (formatName == "BC5") format = Format::cTextureFormat_BC5_uNorm;
    }

    float width = mSizeX, height = mSizeY;
    const auto* scaleData = FindExtUserDataByName("CaptureScale");

    if (scaleData != nullptr) {
        const float scale = *static_cast<const float*>(scaleData->GetData());

        if (scale > 0) {
            width *= scale;
            height *= scale;
        }
    }

    mTexture = new (pHeap, 8) agl::TextureData;
    mTexture->initialize_(agl::TextureType::cTextureType_2D, format, sead::Mathf::round(width),
                         sead::Mathf::round(height), 1, 1, agl::TextureAttribute(0),
                         agl::MultiSampleType(0), true);
    mTexture->setDebugLabel(pName);
    const u32 alignment = mTexture->getAlignment();
    const u32 size = mTexture->getImageByteSize();
    auto* block = new (pHeap, 8) agl::GPUMemBlock<u8>;
    block->allocBuffer_(size, pHeap, alignment, agl::MemoryAttribute(0));
    mTextureMemory = agl::GPUMemAddrBase(*block, 0);
    mTextureMemory.flushCPUCache(mTexture->getImageByteSize());
    const auto& components = mTexture->getSurface().mCompSel;
    auto red = agl::TextureCompSel(components.mR);
    auto green = agl::TextureCompSel(components.mG);
    auto blue = agl::TextureCompSel(components.mB);
    auto alpha = agl::TextureCompSel(components.mA);

    switch (format) {
    case Format::cTextureFormat_R8_uNorm:
    case Format::cTextureFormat_BC4_uNorm:
        red = green = blue = alphaOnly ? agl::cTextureCompSel_1 : agl::cTextureCompSel_R;
        alpha = alphaOnly ? agl::cTextureCompSel_R : agl::cTextureCompSel_1;
        break;
    case Format::cTextureFormat_R8_G8_uNorm:
    case Format::cTextureFormat_BC5_uNorm:
        red = green = blue = agl::cTextureCompSel_R;
        alpha = agl::cTextureCompSel_G;
        break;
    default: break;
    }

    if (mFlags.isOn(8)) {
        alpha = agl::cTextureCompSel_1;
    }

    mTexture->setImagePtr(mTextureMemory);
    mTexture->setCompSel(red, green, blue, alpha);
    SetupTextureInfoByAglTextureData(&mTextureInfo, *mTexture, nullptr);
}

// rDrawInfo provides projection state; rContext carries matrix-dirty state;
// force forwards the caller's calculation request to the captured pane tree.
void CapturePane::Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext, bool force) {
    if ((!mCaptureRequired && !mAlwaysCapture) || !(Pane::mFlags & 1) || !mAlpha) {
        return;
    }

    mCalculated = true;
    const bool forceDirty = rContext.forceGlobalMatrixDirty;
    rContext.forceGlobalMatrixDirty = false;
    rContext.globalMatrixDirty = false;
    {
        nn::util::MatrixT4x4fType savedProjection = rDrawInfo.m_ProjMtx;
        const float halfWidth = mSizeX * 0.5f;
        const float halfHeight = mSizeY * 0.5f;
        sead::OrthoProjection projection(0, 300, halfHeight, -halfHeight, -halfWidth, halfWidth);
        nn::util::MatrixT4x4fType matrix;
        const auto& deviceProjection = projection.getDeviceProjectionMatrix();
        const auto row0 = vld1q_f32(deviceProjection.m[0]);
        const auto row1 = vld1q_f32(deviceProjection.m[1]);
        const auto row2 = vld1q_f32(deviceProjection.m[2]);
        const auto row3 = vld1q_f32(deviceProjection.m[3]);
        matrix._m = {{row0, row1, row2, row3}};
        rDrawInfo.SetProjectionMtx(matrix);
        Pane::Calculate(rDrawInfo, rContext, force);
        rDrawInfo.SetProjectionMtx(savedProjection);
    }

    rContext.forceGlobalMatrixDirty = forceDirty;
    mFlags.change(0x40, rContext.globalMatrixDirty);
}

// NON_MATCHING: two format-dispatch branches are ordered differently.
// rDrawInfo supplies the current render state; rCommands receives the captured pane commands.
void CapturePane::Draw(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) {
    if (!mCalculated || static_cast<DrawInfoEx&>(rDrawInfo).isCapturing()) {
        return;
    }

    const auto* info = static_cast<DrawInfoEx&>(rDrawInfo).getRenderBufferInfo();

    if (info == nullptr) {
        return;
    }

    const auto* captured = drawCapture_(this, rDrawInfo, &mFlags, mMultiFilter, &mRenderBuffer,
                                        &mRenderTarget, mClearColor, rCommands);
    using Format = agl::TextureFormat;
    const auto format = Format(mTexture->getTextureFormat());
    const auto alpha = agl::TextureCompSel(mTexture->getSurface().mCompSel.mA);

    if (agl::TextureFormatInfo::isCompressed(format)) {
        bool rearrange = false;

        switch (format) {
        case Format::cTextureFormat_R8_uNorm:
        case Format::cTextureFormat_BC4_uNorm:
            rearrange = NeedsCaptureChannelRemap(format, alpha);
            break;
        case Format::cTextureFormat_R8_G8_uNorm:
        case Format::cTextureFormat_BC5_uNorm:
            rearrange = true;
            break;
        default: break;
        }

        if (rearrange) {
            agl::TextureData converted(*captured);

            if (alpha == agl::cTextureCompSel_R) {
                converted.setCompSel(agl::cTextureCompSel_A, agl::cTextureCompSel_1, agl::cTextureCompSel_1, agl::cTextureCompSel_1);
            } else {
                converted.setCompSel(agl::cTextureCompSel_R, agl::cTextureCompSel_A, agl::cTextureCompSel_1, agl::cTextureCompSel_1);
            }

            converted.compressTo(static_cast<agl::DrawContext*>(info->pDrawContext), mTexture, 0, 0);
        } else {
            captured->compressTo(static_cast<agl::DrawContext*>(info->pDrawContext), mTexture, 0, 0);
        }
    } else {
        const float width = mTexture->getWidth(0);
        const float height = mTexture->getHeight(0);
        mRenderTarget.applyTextureData(*mTexture);
        mRenderBuffer.setPhysicalArea(0, 0, width, height);
        mRenderBuffer.setVirtualSize(sead::Vector2f(width, height));
        mRenderBuffer.bind(info->pDrawContext);
        sead::GraphicsContext context;
        context.setBlendEnable(false);
        context.setDepthTestEnable(false);
        context.apply(info->pDrawContext);
        agl::TextureSampler sampler(*captured);

        switch (format) {
        case Format::cTextureFormat_R8_uNorm:
        case Format::cTextureFormat_BC4_uNorm:
            if (alpha == agl::cTextureCompSel_R) {
                sampler.setCompSel(agl::cTextureCompSel_A, agl::cTextureCompSel_1, agl::cTextureCompSel_1, agl::cTextureCompSel_1);
            }

            break;
        case Format::cTextureFormat_R8_G8_uNorm:
        case Format::cTextureFormat_BC5_uNorm:
            sampler.setCompSel(agl::cTextureCompSel_R, agl::cTextureCompSel_A, agl::cTextureCompSel_1, agl::cTextureCompSel_1);
            break;
        default: break;
        }

        if (captured->getWidth(0) != mTexture->getWidth(0) || captured->getHeight(0) != mTexture->getHeight(0)) {
            sead::Viewport viewport(mRenderBuffer);
            viewport.apply(info->pDrawContext, mRenderBuffer);
        }

        agl::utl::ImageFilter2D::drawTextureQuadTriangle(static_cast<agl::DrawContext*>(info->pDrawContext), sampler);
    }

    if (mMultiFilter != nullptr && mMultiFilter->getResultTexture() != nullptr) {
        mMultiFilter->freeResultTexture();
    } else {
        agl::utl::DynamicTextureAllocator::instance()->free(captured);
    }

    mRenderTarget.invalidateGPUCache(static_cast<agl::DrawContext*>(info->pDrawContext));
    DrawInfoEx::applyRenderBufferInfo(info);
    rDrawInfo.ResetDrawState();
    mFlags.set(1);
    mCaptureRequired = mFlags.isOn(0x40);
    mCalculated = false;
}

// NON_MATCHING: allocation setup and texture-coordinate register allocation differ.
// pPane supplies the captured subtree; rDrawInfo supplies projection and render state;
// pFlags selects capture sizing and alpha; pFilter optionally processes the result;
// pBuffer and pTarget receive the temporary attachment; pClearColor optionally clears it;
// rCommands receives the pane's draw commands. The caller owns the returned temporary texture.
const agl::TextureData* CapturePane::drawCapture_(nn::ui2d::Pane* pPane, nn::ui2d::DrawInfo& rDrawInfo,
    sead::BitFlag8* pFlags, agl::utl::MultiFilter* pFilter, agl::RenderBuffer* pBuffer,
    agl::RenderTargetColor* pTarget, sead::Color4f* pClearColor, nn::gfx::CommandBuffer& rCommands) {
    const auto* info = static_cast<DrawInfoEx&>(rDrawInfo).getRenderBufferInfo();
    u32 width, height;

    if (pFlags->isOn(0x10)) {
        width = info->pFrameBuffer->getPhysicalArea().getSizeX() * pPane->mSizeX / rDrawInfo.m_pLayoutInformation->size.width;
        height = info->pFrameBuffer->getPhysicalArea().getSizeY() * pPane->mSizeY / rDrawInfo.m_pLayoutInformation->size.height;
    } else if (pFlags->isOn(0x20)) {
        const u32 scaledWidth = info->pFrameBuffer->getPhysicalArea().getSizeX() * pPane->mSizeX / rDrawInfo.m_pLayoutInformation->size.width;
        const u32 scaledHeight = info->pFrameBuffer->getPhysicalArea().getSizeY() * pPane->mSizeY / rDrawInfo.m_pLayoutInformation->size.height;
        width = sead::Mathu::max(u32(pPane->mSizeX), scaledWidth);
        height = sead::Mathu::max(u32(pPane->mSizeY), scaledHeight);
    } else {
        width = pPane->mSizeX;
        height = pPane->mSizeY;
    }

    const auto format = pFlags->isOn(4) ? agl::TextureFormat::cTextureFormat_R10_G10_B10_A2_uNorm
                                      : agl::TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
    const agl::TextureData* texture = agl::utl::DynamicTextureAllocator::instance()->alloc(
        static_cast<agl::DrawContext*>(info->pDrawContext), "CapturePaneWork", format, width, height, 1,
        nullptr, agl::utl::DynamicTextureAllocator::cAllocateType_0, true, false);
    pTarget->applyTextureData(*texture);
    pBuffer->setPhysicalArea(0, 0, width, height);
    pBuffer->setVirtualSize(sead::Vector2f(pPane->mSizeX, pPane->mSizeY));
    pBuffer->bind(info->pDrawContext);
    sead::Viewport viewport(*pBuffer);

    if (pClearColor != nullptr) {
        pBuffer->fastClear(static_cast<agl::DrawContext*>(info->pDrawContext), 0, 1, *pClearColor, 0, 0, viewport, true);
    } else {
        viewport.apply(info->pDrawContext, *pBuffer);
        const auto* source = static_cast<const agl::RenderBuffer*>(info->pFrameBuffer)->getRenderTargetColor();
        source->invalidateGPUCache(static_cast<agl::DrawContext*>(info->pDrawContext));
        agl::TextureSampler sampler(*source);

        if (pFlags->isOn(2)) {
            sampler.setCompSel(agl::cTextureCompSel_R, agl::cTextureCompSel_G, agl::cTextureCompSel_B, agl::cTextureCompSel_1);
        } else {
            sampler.setCompSel(agl::cTextureCompSel_R, agl::cTextureCompSel_G, agl::cTextureCompSel_B, agl::cTextureCompSel_0);
        }

        sampler.setFilter(1, 1, 1);
        sampler.setWrap(7, 7, 7);
        sead::GraphicsContext context;
        context.setBlendEnable(false);
        context.setDepthTestEnable(false);
        context.apply(info->pDrawContext);
        const auto* parent = pPane->mParent;
        const float x = parent->mGlobalMtx[3] + (parent->mGlobalMtx[2] * pPane->mPositionZ +
            parent->mGlobalMtx[1] * pPane->mPositionY + parent->mGlobalMtx[0] * pPane->mPositionX);
        const float y = parent->mGlobalMtx[7] + (parent->mGlobalMtx[6] * pPane->mPositionZ +
            parent->mGlobalMtx[5] * pPane->mPositionY + parent->mGlobalMtx[4] * pPane->mPositionX);
        const sead::Vector2f scale(pPane->mSizeX / rDrawInfo.m_pLayoutInformation->size.width,
                                   pPane->mSizeY / rDrawInfo.m_pLayoutInformation->size.height);
        const sead::Vector2f offset(x / pPane->mSizeX, -y / pPane->mSizeY);
        const sead::Vector2f textureScale(pPane->mSizeX / source->getWidth(0), pPane->mSizeY / source->getHeight(0));
        agl::utl::ImageFilter2D::drawTextureTexCoord(static_cast<agl::DrawContext*>(info->pDrawContext),
            sampler, viewport, scale, 0, offset, textureScale, sead::Vector2f::zero);
    }

    info->pGraphicsContext->apply(info->pDrawContext);
    rDrawInfo.ResetDrawState();
    {
        const nn::util::MatrixT4x4fType savedProjection = rDrawInfo.m_ProjMtx;
        sead::OrthoProjection projection(0, 300, viewport);
        const auto& matrix = projection.getDeviceProjectionMatrix();
        const auto row0 = vld1q_f32(matrix.m[0]);
        const auto row1 = vld1q_f32(matrix.m[1]);
        const auto row2 = vld1q_f32(matrix.m[2]);
        const auto row3 = vld1q_f32(matrix.m[3]);
        const nn::util::MatrixT4x4fType deviceProjection = {{{row0, row1, row2, row3}}};
        rDrawInfo.SetProjectionMtx(deviceProjection);
        pPane->Pane::Draw(rDrawInfo, rCommands);
        rDrawInfo.SetProjectionMtx(savedProjection);
    }

    pTarget->invalidateGPUCache(static_cast<agl::DrawContext*>(info->pDrawContext));

    if (pFilter != nullptr) {
        pFilter->draw(static_cast<agl::DrawContext*>(info->pDrawContext), *texture);

        if (pFilter->getResultTexture() != nullptr) {
            agl::utl::DynamicTextureAllocator::instance()->free(texture);
            return pFilter->getResultTexture();
        }
    }

    pBuffer->drawFlipYGL_(static_cast<agl::DrawContext*>(info->pDrawContext), true, false);
    return texture;
}

CapturePane::~CapturePane() {
    if (mClearColor != nullptr) {
        delete mClearColor;
        mClearColor = nullptr;
    }

    if (mMultiFilter != nullptr) {
        delete mMultiFilter;
        mMultiFilter = nullptr;
    }

    if (mTextureMemory.isValid()) {
        mTextureMemory.deleteGPUMemBlock();
    }

    delete mTexture;
}

// pPane supplies the capture alpha setting; pFlags receives bit 3 when the output is opaque.
void CapturePane::setupCaptureOutputAlpha255_(nn::ui2d::Pane* pPane, sead::BitFlag8* pFlags) {
    const auto* data = pPane->FindExtUserDataByName("CaptureOutputAlpha");

    if (data != nullptr && *static_cast<const s32*>(data->GetData()) == 255) {
        pFlags->set(8);
    }
}
}
