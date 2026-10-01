#include "Project/PostProcessing/MangaDrawer.hpp"

#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <common/aglRenderTarget.h>
#include <common/aglShaderLocation.h>
#include <common/aglShaderProgram.h>
#include <common/aglTextureData.h>
#include <common/aglTextureSampler.h>
#include <g3d/aglNW4FToNN.h>
#include <g3d/aglTextureDataInitializerG3D.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <nvn/nvn_FuncPtrInline.h>
#include <utility/aglDynamicTextureAllocator.h>
#include <utility/aglImageFilter2D.h>
#include <utility/aglPrimitiveShape.h>
#include <utility/aglVertexAttributeHolder.h>

#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Library/Shader/ForwardRendering/ShaderSamplerSetter.hpp"

namespace {
const al::UniformBlockLayout cMipUboLayout[] = {
    {0, agl::UniformBlock::cType_Int, 1},
};

typedef agl::utl::ImageFilter2D::GaussianKernel GaussianKernel;

inline void drawQuadIndexStream(agl::DrawContext* pContext) {
    const agl::IndexStream& rStream =
        agl::utl::PrimitiveShape::instance()->getQuadTriangleIndexStream();
    u32 count = rStream.getCount();

    if (count != 0) {
        NVNdrawPrimitive primitive = rStream.getPrimitiveType();
        NVNcommandBuffer* pCommandBuffer = pContext->getNvnCommandBuffer();
        NVNbufferAddress address = nvnBufferGetAddress(rStream.getNvnBuffer());
        nvnCommandBufferDrawElements(pCommandBuffer, primitive, NVNindexType(rStream.getFormat()),
                                     count, address);
    }
}

inline void drawQuad(agl::DrawContext* pContext) {
    agl::utl::VertexAttributeHolder::instance()
        ->getVertexAttribute(agl::utl::VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
        .activate(pContext);
    drawQuadIndexStream(pContext);
}

template <typename T>
inline void setUniformValue(agl::DrawContext* pContext, const agl::UniformLocation& rLocation,
                            T value) {
    if (rLocation.isValid()) {
        rLocation.setUniformNVN(pContext, 1, &value);
    }
}

template <typename T>
inline void setUniform(agl::DrawContext* pContext, const agl::ShaderProgram* pProgram,
                       const char* pName, T value) {
    agl::UniformLocation location(pName);
    location.search(*pProgram);
    setUniformValue(pContext, location, value);
}

inline void activateSampler(agl::DrawContext* pContext, const agl::ShaderProgram* pProgram,
                            const char* pName, const agl::TextureData& rTexture) {
    agl::SamplerLocation location(pName);
    location.search(*pProgram);
    agl::TextureSampler sampler;
    sampler.applyTextureData(rTexture);
    sampler.activate(pContext, location, -1, false);
}
}  // namespace

namespace al {

/**
 * Constructs the manga drawing parameters.
 */
MangaDrawParam::MangaDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "有効", "", true);
    mIsOnlyNormalLineDraw = new ParameterBool(false, mParamObj, "IsOnlyNormalLineDraw",
                                              "法線を閾値とした線画のみ", "", true);
    mIsUseWhiteEdge =
        new ParameterBool(false, mParamObj, "IsUseWhiteEdge", "白線を描画する", "", true);
    mWhiteParam =
        new ParameterF32(0.03f, mParamObj, "WhiteParam", "白の閾値", "Min=0.f, Max=1.f", true);
    mBlackParam =
        new ParameterF32(0.03f, mParamObj, "BlackParam", "黒の閾値", "Min=0.f, Max=1.f", true);
    mMipLevelParam =
        new ParameterS32(10, mParamObj, "MipLevelParam", "ミップレベル", "Min=0, Max=15", true);
    mIsUseLinearDepth =
        new ParameterBool(false, mParamObj, "IsUseLinearDepth", "リニアデプスで判定", "", true);
    mKernelSize =
        new ParameterS32(0, mParamObj, "KernelSize", "カーネルサイズ", "Min=0, Max=2", true);
    mGaussianType = new ParameterS32(1, mParamObj, "GaussianType", "ガウシアンタイプ",
                                     "Min=-1, Max=5", true);
    mGaussianTypeForEdge = new ParameterS32(1, mParamObj, "GaussianTypeForEdge",
                                            "ガウシアンタイプ(エッジ用)", "Min=-1, Max=5", true);
    mThreshold = new ParameterF32(0.3f, mParamObj, "Threshold", "閾値", "Min=0, Max=1", true);
    mThresholdWhiteMin = new ParameterF32(0.7f, mParamObj, "ThresholdWhiteMin", "閾値Min(白枠)",
                                          "Min=0, Max=1", true);
    mThresholdWhiteMax = new ParameterF32(0.7f, mParamObj, "ThresholdWhiteMax", "閾値Max(白枠)",
                                          "Min=0, Max=1", true);
    mThresholdDepth =
        new ParameterF32(5.0f, mParamObj, "ThresholdDepth", "デプス閾値", "Min=0, Max=1", true);
    mScreenToneType =
        new ParameterS32(0, mParamObj, "ScreenToneType", "タイプ(薄)", "Min=0, Max=2", true);
    mScreenToneTypeDepth =
        new ParameterS32(0, mParamObj, "ScreenToneTypeDepth", "タイプ(濃)", "Min=0, Max=2", true);
    mNormalParam =
        new ParameterF32(0.7f, mParamObj, "NormalParam", "Normalの閾値", "Min=-1, Max=1", true);
    mNormalNearParam = new ParameterF32(0.1f, mParamObj, "NormalNearParam",
                                        "Normal(ズーム近)の閾値", "Min=-1, Max=1", true);
    mDepthZParam = new ParameterF32(5.0f, mParamObj, "DepthZParam",
                                    "法線を太くするティスプの閾値", "Min=0, Max=1", true);
    mWhiteEdgeDistanceNear = new ParameterF32(0.004f, mParamObj, "WhiteEdgeDistanceNear",
                                              "白枠を描画する距離(近)", "Min=0, Max=1", true);
    mWhiteEdgeDistanceFar = new ParameterF32(0.004f, mParamObj, "WhiteEdgeDistanceFar",
                                             "白枠を描画する距離(遠)", "Min=0, Max=1", true);
    mNearParam = new ParameterF32(0.00025f, mParamObj, "NearParam", "近いかどうかのパラメータ",
                                  "Min=0, Max=12", true);
    mWhiteAlphaBase = new ParameterF32(0.2f, mParamObj, "WhiteAlphaBase",
                                       "白枠をだんだん透過する(基底値)", "Min=0, Max=1", true);
    mWhiteAlphaAdd = new ParameterF32(0.2f, mParamObj, "WhiteAlphaAdd",
                                      "白枠をだんだん透過する(加算値)", "Min=0, Max=1", true);
    mRepeateNumScreenDot = new ParameterF32(100.0f, mParamObj, "RepeateNumScreenDot",
                                            "リピート数(ドット)", "Min=0, Max=100", true);
    mRepeateNumScreenLine = new ParameterF32(2.0f, mParamObj, "RepeateNumScreenLine",
                                             "リピート数(ライン)", "Min=0, Max=10", true);
    mWhiteBaseColor = new ParameterF32(0.97f, mParamObj, "WhiteBaseColor", "白のベース色RGB",
                                       "Min=0, Max=1", true);
    mBlackBaseColor = new ParameterF32(0.03f, mParamObj, "BlackBaseColor", "黒のベース色RGB",
                                       "Min=0, Max=1", true);
}

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool MangaDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Checks the OnlyNormalLineDraw flag.
 * @return Whether OnlyNormalLineDraw is set.
 */
bool MangaDrawParam::isOnlyNormalLineDraw() const {
    return mIsOnlyNormalLineDraw->getValue();
}

/**
 * Checks the UseWhiteEdge flag.
 * @return Whether UseWhiteEdge is set.
 */
bool MangaDrawParam::isUseWhiteEdge() const {
    return mIsUseWhiteEdge->getValue();
}

/**
 * Gets the WhiteParam parameter.
 * @return WhiteParam.
 */
f32 MangaDrawParam::getWhiteParam() const {
    return mWhiteParam->getValue();
}

/**
 * Gets the BlackParam parameter.
 * @return BlackParam.
 */
f32 MangaDrawParam::getBlackParam() const {
    return mBlackParam->getValue();
}

/**
 * Gets the MipLevelParam parameter.
 * @return MipLevelParam.
 */
s32 MangaDrawParam::getMipLevelParam() const {
    return mMipLevelParam->getValue();
}

/**
 * Checks the UseLinearDepth flag.
 * @return Whether UseLinearDepth is set.
 */
bool MangaDrawParam::isUseLinearDepth() const {
    return mIsUseLinearDepth->getValue();
}

/**
 * Gets the KernelSize parameter.
 * @return KernelSize.
 */
s32 MangaDrawParam::getKernelSize() const {
    return mKernelSize->getValue();
}

/**
 * Gets the GaussianType parameter.
 * @return GaussianType.
 */
s32 MangaDrawParam::getGaussianType() const {
    return mGaussianType->getValue();
}

/**
 * Gets the GaussianTypeForEdge parameter.
 * @return GaussianTypeForEdge.
 */
s32 MangaDrawParam::getGaussianTypeForEdge() const {
    return mGaussianTypeForEdge->getValue();
}

/**
 * Gets the Threshold parameter.
 * @return Threshold.
 */
f32 MangaDrawParam::getThreshold() const {
    return mThreshold->getValue();
}

/**
 * Gets the ThresholdWhiteMin parameter.
 * @return ThresholdWhiteMin.
 */
f32 MangaDrawParam::getThresholdWhiteMin() const {
    return mThresholdWhiteMin->getValue();
}

/**
 * Gets the ThresholdWhiteMax parameter.
 * @return ThresholdWhiteMax.
 */
f32 MangaDrawParam::getThresholdWhiteMax() const {
    return mThresholdWhiteMax->getValue();
}

/**
 * Gets the ThresholdDepth parameter.
 * @return ThresholdDepth.
 */
f32 MangaDrawParam::getThresholdDepth() const {
    return mThresholdDepth->getValue();
}

/**
 * Gets the ScreenToneType parameter.
 * @return ScreenToneType.
 */
s32 MangaDrawParam::getScreenToneType() const {
    return mScreenToneType->getValue();
}

/**
 * Gets the ScreenToneTypeDepth parameter.
 * @return ScreenToneTypeDepth.
 */
s32 MangaDrawParam::getScreenToneTypeDepth() const {
    return mScreenToneTypeDepth->getValue();
}

/**
 * Gets the NormalParam parameter.
 * @return NormalParam.
 */
f32 MangaDrawParam::getNormalParam() const {
    return mNormalParam->getValue();
}

/**
 * Gets the NormalNearParam parameter.
 * @return NormalNearParam.
 */
f32 MangaDrawParam::getNormalNearParam() const {
    return mNormalNearParam->getValue();
}

/**
 * Gets the DepthZParam parameter.
 * @return DepthZParam.
 */
f32 MangaDrawParam::getDepthZParam() const {
    return mDepthZParam->getValue();
}

/**
 * Gets the WhiteEdgeDistanceNear parameter.
 * @return WhiteEdgeDistanceNear.
 */
f32 MangaDrawParam::getWhiteEdgeDistanceNear() const {
    return mWhiteEdgeDistanceNear->getValue();
}

/**
 * Gets the WhiteEdgeDistanceFar parameter.
 * @return WhiteEdgeDistanceFar.
 */
f32 MangaDrawParam::getWhiteEdgeDistanceFar() const {
    return mWhiteEdgeDistanceFar->getValue();
}

/**
 * Gets the NearParam parameter.
 * @return NearParam.
 */
f32 MangaDrawParam::getNearParam() const {
    return mNearParam->getValue();
}

/**
 * Gets the WhiteAlphaBase parameter.
 * @return WhiteAlphaBase.
 */
f32 MangaDrawParam::getWhiteAlphaBase() const {
    return mWhiteAlphaBase->getValue();
}

/**
 * Gets the WhiteAlphaAdd parameter.
 * @return WhiteAlphaAdd.
 */
f32 MangaDrawParam::getWhiteAlphaAdd() const {
    return mWhiteAlphaAdd->getValue();
}

/**
 * Gets the RepeateNumScreenDot parameter.
 * @return RepeateNumScreenDot.
 */
f32 MangaDrawParam::getRepeateNumScreenDot() const {
    return mRepeateNumScreenDot->getValue();
}

/**
 * Gets the RepeateNumScreenLine parameter.
 * @return RepeateNumScreenLine.
 */
f32 MangaDrawParam::getRepeateNumScreenLine() const {
    return mRepeateNumScreenLine->getValue();
}

/**
 * Gets the WhiteBaseColor parameter.
 * @return WhiteBaseColor.
 */
f32 MangaDrawParam::getWhiteBaseColor() const {
    return mWhiteBaseColor->getValue();
}

/**
 * Gets the BlackBaseColor parameter.
 * @return BlackBaseColor.
 */
f32 MangaDrawParam::getBlackBaseColor() const {
    return mBlackBaseColor->getValue();
}

/**
 * Constructs the manga drawer, its parameter interpolation and the per-mip uniform blocks.
 * @param pShaderHolder Shader holder to get the shader programs from.
 */
MangaDrawer::MangaDrawer(ShaderHolder* pShaderHolder) : mShaderHolder(pShaderHolder) {
    mRenderLuminanceProgram = pShaderHolder->getShaderProgram("alRenderLuminance");
    mGenerateMipmapProgram = pShaderHolder->getShaderProgram("alGenerateMipmap");

    ParamRequestInterp* interp = new ParamRequestInterp();
    mRequestInterp = interp;
    interp->setCurrentParam(new MangaDrawParam());
    interp->setStartParam(new MangaDrawParam());
    interp->setEndParam(new MangaDrawParam());
    interp->setRequestParam(new MangaDrawParam());

    s32 mipNum = calcMaxMipLevelNum(256);
    mMipUniformBlocks = new UniformBlock*[mipNum];

    for (s32 i = 0; i < mipNum; i++) {
        mMipUniformBlocks[i] = createUniformBlock(cMipUboLayout, 1, nullptr, 2);
    }
}

/**
 * Destroys the uniform blocks and the screen tone textures.
 */
MangaDrawer::~MangaDrawer() {
    s32 mipNum = calcMaxMipLevelNum(256);

    for (s32 i = 0; i < mipNum; i++) {
        if (mMipUniformBlocks[i] != nullptr) {
            delete mMipUniformBlocks[i];
            mMipUniformBlocks[i] = nullptr;
        }
    }

    if (mComicDotTexture != nullptr) {
        delete mComicDotTexture;
        mComicDotTexture = nullptr;
    }

    if (mComicLineTexture != nullptr) {
        delete mComicLineTexture;
        mComicLineTexture = nullptr;
    }
}

/**
 * Creates the screen tone textures from the project resource.
 * @param pResFile Project resource file.
 */
void MangaDrawer::initProjectResource(nn::g3d::ResFile* pResFile) {
    nn::gfx::ResTexture* dotTexture = agl::g3d::ResFile::GetTexture(pResFile, "TextureComicDot");

    if (dotTexture != nullptr) {
        mComicDotTexture = new agl::TextureData();
        agl::g3d::TextureDataInitializerG3D::initialize(mComicDotTexture, *dotTexture);
        mComicDotTexture->flushCPUCache();
    }

    nn::gfx::ResTexture* lineTexture =
        agl::g3d::ResFile::GetTexture(pResFile, "TextureComicLine");

    if (lineTexture != nullptr) {
        mComicLineTexture = new agl::TextureData();
        agl::g3d::TextureDataInitializerG3D::initialize(mComicLineTexture, *lineTexture);
        mComicLineTexture->flushCPUCache();
    }
}

/**
 * Finishes initialization of the parameter interpolation.
 */
void MangaDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void MangaDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation and writes the mip level of every uniform block.
 */
void MangaDrawer::update() {
    mRequestInterp->updateInterp();
    s32 mipNum = calcMaxMipLevelNum(256);

    for (s32 i = 0; i < mipNum; i++) {
        UniformBlockSetter setter(mMipUniformBlocks[i], 0);
        mMipUniformBlocks[i]->setValue(0, i - 1);
    }
}

void MangaDrawer::draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer,
                       const agl::TextureData* pLinearDepth, const agl::TextureData* pLinearDepth2,
                       const agl::TextureData* pDepth, const agl::TextureData* pNormal, f32 near,
                       f32 far, f32 depthZScale) const {
    const MangaDrawParam* param = getCurrentParam();

    if (!param->isEnable()) {
        return;
    }

    const agl::TextureData* color = rBuffer.getRenderTargetColor();
    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();
    s32 mipNum = calcMaxMipLevelNum(256);
    agl::TextureData* colorCopy = allocator->alloc(
        pContext, "color_clamp_draw_texture", agl::TextureFormat(color->getTextureFormat()),
        color->getWidth(0), color->getHeight(0), mipNum, nullptr,
        agl::utl::DynamicTextureAllocator::AllocateType(0), true, false);
    agl::TextureData* luminance = agl::utl::DynamicTextureAllocator::instance()->alloc(
        pContext, "temp_lumi", agl::TextureFormat(color->getTextureFormat()), 256, 256, mipNum,
        nullptr, agl::utl::DynamicTextureAllocator::AllocateType(0), true, false);
    agl::TextureData* luminanceMax = agl::utl::DynamicTextureAllocator::instance()->alloc(
        pContext, "temp_lumi_max", agl::TextureFormat(color->getTextureFormat()), 256, 256, mipNum,
        nullptr, agl::utl::DynamicTextureAllocator::AllocateType(0), true, false);
    agl::TextureData* luminanceMin = agl::utl::DynamicTextureAllocator::instance()->alloc(
        pContext, "temp_lumi_min", agl::TextureFormat(color->getTextureFormat()), 256, 256, mipNum,
        nullptr, agl::utl::DynamicTextureAllocator::AllocateType(0), true, false);

    mRenderLuminanceProgram->activate(pContext, true);

    {
        const agl::ShaderProgram* program = mRenderLuminanceProgram;
        agl::SamplerLocation location("uFrameBuffer");
        location.search(*program);
        agl::TextureSampler sampler;
        sampler.applyTextureData(*color);
        sampler.setMagFilter(1);
        sampler.activate(pContext, location, -1, false);

        sead::GraphicsContext graphicsContext;
        graphicsContext.setDepthEnable(false, false);
        graphicsContext.setBlendEnable(false);
        graphicsContext.apply(pContext);

        {
            agl::RenderBuffer renderBuffer;
            RenderBufferAttacher attacher(&renderBuffer, luminance, nullptr, nullptr, nullptr,
                                          nullptr);
            drawQuad(pContext);
        }

        luminance->copyToAll(pContext, luminanceMax);
        luminance->copyToAll(pContext, luminanceMin);
    }

    {
        agl::TextureData* mipTextures[] = {luminanceMax, luminanceMin, luminance};

        for (s32 i = 0; i < 3; i++) {
            const char* macros[] = {"OUTPUT_TYPE"};
            const char* values[] = {"0"};

            switch (i) {
            case 0:
                values[0] = "0";
                break;
            case 1:
                values[0] = "1";
                break;
            case 2:
                values[0] = "2";
                break;
            default:
                break;
            }

            const agl::ShaderProgram* program =
                mGenerateMipmapProgram->searchVariation(1, macros, values);
            program->activate(pContext, true);

            agl::SamplerLocation location("uFrameBuffer");
            location.search(*program);
            agl::TextureSampler sampler;
            sampler.applyTextureData(*mipTextures[i]);
            sampler.activate(pContext, location, -1, false);

            sead::GraphicsContext graphicsContext;
            graphicsContext.setDepthEnable(false, false);
            graphicsContext.setBlendEnable(false);
            graphicsContext.apply(pContext);

            for (s32 mip = 1; mip < mipNum; mip++) {
                agl::RenderBuffer renderBuffer;
                agl::RenderTargetColor target;
                renderBuffer.setRenderTargetColorNullAll();
                renderBuffer.setRenderTargetDepth(nullptr);
                target.applyTextureData(*mipTextures[i]);
                target.setMipLevel(mip);

                s32 size = 256 >> mip;
                renderBuffer.setRenderTargetColor(&target);
                renderBuffer.setVirtualSize(sead::Vector2f(size, size));
                renderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, size, size));
                renderBuffer.bind(pContext);

                sead::Viewport viewport(renderBuffer);
                viewport.apply(pContext, renderBuffer);

                agl::UniformBlockLocation blockLocation("MipUbo");
                blockLocation.search(*program);
                mMipUniformBlocks[mip]->activate(pContext, blockLocation);
                drawQuadIndexStream(pContext);
                renderBuffer.getRenderTargetColor()->invalidateGPUCache(pContext);
            }
        }
    }

    pContext->barrierTexture(1);

    s32 gaussianType = param->getGaussianType();

    if (gaussianType != -1) {
        auto drawGaussian = [pContext](const agl::TextureData* pFrom, const agl::TextureData* pTo,
                                       s32 kernelType, bool isVertical) {
            agl::TextureSampler sampler;
            sampler.applyTextureData(*pFrom);
            sampler.setWrapX(7);
            sampler.setWrapY(7);

            agl::RenderTargetColor target;
            target.applyTextureData(*pTo);

            agl::RenderBuffer renderBuffer;
            renderBuffer.setVirtualSize(sead::Vector2f(pFrom->getWidth(0), pFrom->getHeight(0)));
            renderBuffer.setPhysicalArea(
                sead::BoundBox2f(0.0f, 0.0f, pFrom->getWidth(0), pFrom->getHeight(0)));
            renderBuffer.setRenderTargetColorNullAll();
            renderBuffer.setRenderTargetColor(&target);
            renderBuffer.bind(pContext);

            sead::Viewport viewport(renderBuffer);
            viewport.apply(pContext, renderBuffer);
            agl::utl::ImageFilter2D::drawGaussian(pContext, sampler, viewport,
                                                  GaussianKernel(kernelType), isVertical, true,
                                                  sead::Vector2f::zero);
            target.invalidateGPUCache(pContext);
        };

        agl::TextureData* work = allocator->alloc(
            pContext, "gaussian_x", agl::TextureFormat(color->getTextureFormat()),
            color->getWidth(0), color->getHeight(0), 1, nullptr,
            agl::utl::DynamicTextureAllocator::AllocateType(0), true, false);
        drawGaussian(color, work, gaussianType, true);
        drawGaussian(work, colorCopy, gaussianType, false);
        allocator->free(work);
    } else {
        color->copyToAll(pContext, colorCopy);
    }

    const char* macros[] = {"KERNEL_SIZE", "IS_USE_FRAME", "IS_USE_WHITE_EDGE",
                            "IS_USE_DUAL_DEPTH"};
    const char* values[] = {"0", "0", "1", "0"};

    switch (param->getKernelSize()) {
    case 0:
        values[0] = "0";
        break;
    case 1:
        values[0] = "1";
        break;
    case 2:
        values[0] = "2";
        break;
    default:
        break;
    }

    values[2] = param->isUseWhiteEdge() ? "1" : "0";

    if (param->isUseLinearDepth()) {
        values[3] = pLinearDepth2 != nullptr ? "1" : "0";
    }

    const agl::ShaderProgram* program = mShaderHolder->getShaderProgram("alRenderManga");
    program = program->searchVariation(4, macros, values);
    program->activate(pContext, true);

    const char* screenToneNames[] = {"uTexScreenTone", "uTexScreenToneDepth"};
    s32 screenToneTypes[] = {param->getScreenToneType(), param->getScreenToneTypeDepth()};
    f32 repeatNum;
    f32 repeatNumDepth;
    f32* repeatNums[] = {&repeatNum, &repeatNumDepth};

    for (s32 i = 0; i < 2; i++) {
        agl::SamplerLocation location(screenToneNames[i]);
        location.search(*program);
        agl::TextureSampler sampler;
        const agl::TextureData* texture;

        switch (screenToneTypes[i]) {
        case 0:
            texture = mComicDotTexture;
            *repeatNums[i] = param->getRepeateNumScreenDot();
            break;
        case 1:
            texture = mComicLineTexture;
            *repeatNums[i] = param->getRepeateNumScreenLine();
            break;
        default:
            texture = nullptr;
            break;
        }

        sampler.applyTextureData(*texture);
        sampler.setWrapX(1);
        sampler.setWrapY(1);
        sampler.activate(pContext, location, -1, false);
    }

    agl::SamplerLocation colorLocation("uOrgColor");
    colorLocation.search(*program);
    agl::TextureSampler colorSampler;
    colorSampler.applyTextureData(*colorCopy);
    colorSampler.activate(pContext, colorLocation, -1, false);

    agl::SamplerLocation depthLocation("uOrgDepth");
    depthLocation.search(*program);
    agl::TextureSampler depthSampler;

    if (param->isUseLinearDepth()) {
        depthSampler.applyTextureData(*pLinearDepth);

        if (pLinearDepth2 != nullptr) {
            activateSampler(pContext, program, "uOrgDepth2", *pLinearDepth2);
        }
    } else {
        depthSampler.applyTextureData(*pDepth);
    }

    depthSampler.activate(pContext, depthLocation, -1, false);

    activateSampler(pContext, program, "uLuminance", *luminance);
    activateSampler(pContext, program, "uLuminanceMax", *luminanceMax);
    activateSampler(pContext, program, "uLuminanceMin", *luminanceMin);
    activateSampler(pContext, program, "uTexNormal", *pNormal);

    setUniform(pContext, program, "uWhiteParam", param->getWhiteParam());
    setUniform(pContext, program, "uBlackParam", param->getBlackParam());
    setUniform(pContext, program, "uMipLevelParam", param->getMipLevelParam());

    {
        agl::UniformLocation location("uScreenToneRepeatNum");
        location.search(*program);
        setUniformValue(pContext, location, repeatNum);
    }

    {
        agl::UniformLocation location("uScreenToneRepeatNumDepth");
        location.search(*program);
        setUniformValue(pContext, location, repeatNumDepth);
    }

    sead::Vector2f texel(1.0f / color->getWidth(0), 1.0f / color->getHeight(0));
    setUniform(pContext, program, "uThreshold", param->getThreshold());
    setUniform(pContext, program, "uThresholdWhiteMin", param->getThresholdWhiteMin());
    setUniform(pContext, program, "uThresholdWhiteMax", param->getThresholdWhiteMax());
    setUniform(pContext, program, "uThresholdDepth", param->getThresholdDepth());

    {
        agl::UniformLocation location("uTexel");
        location.search(*program);
        location.setUniform(pContext, 2, &texel);
    }

    setUniform(pContext, program, "uNormalParam", param->getNormalParam());
    setUniform(pContext, program, "uNormalNearParam", param->getNormalNearParam());
    setUniform(pContext, program, "uDepthZParam", param->getDepthZParam() * depthZScale);
    setUniform(pContext, program, "uWhiteEdgeDistanceNear", param->getWhiteEdgeDistanceNear());
    setUniform(pContext, program, "uWhiteEdgeDistanceFar", param->getWhiteEdgeDistanceFar());
    setUniform(pContext, program, "uNearParam", param->getNearParam());
    setUniform(pContext, program, "uWhiteAlphaBase", param->getWhiteAlphaBase());
    setUniform(pContext, program, "uWhiteAlphaAdd", param->getWhiteAlphaAdd());
    setUniform(pContext, program, "uWhiteBaseColor", param->getWhiteBaseColor());
    setUniform(pContext, program, "uBlackBaseColor", param->getBlackBaseColor());

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(pContext);

    {
        agl::RenderBuffer renderBuffer;
        RenderBufferAttacher attacher(&renderBuffer, color, nullptr, nullptr, nullptr, nullptr);
        drawQuad(pContext);
    }

    allocator->free(colorCopy);
    allocator->free(luminance);
    allocator->free(luminanceMax);
    allocator->free(luminanceMin);
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const MangaDrawParam* MangaDrawer::getCurrentParam() const {
    return static_cast<const MangaDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void MangaDrawer::requestParam(s32 priority, s32 step, const MangaDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool MangaDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

/**
 * Advances the frame id (does nothing).
 */
void MangaDrawer::nextFrameId() {}

}  // namespace al
