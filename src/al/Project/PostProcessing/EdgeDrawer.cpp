#include "Project/PostProcessing/EdgeDrawer.hpp"

#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <common/aglRenderTarget.h>
#include <common/aglShaderLocation.h>
#include <common/aglShaderProgram.h>
#include <common/aglTextureData.h>
#include <common/aglTextureSampler.h>
#include <gfx/seadCamera.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/FullScreenQuadModel.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Camera/Info/SceneCameraInfo.hpp"

namespace {
const sead::Color4f cDefaultOffsetColor(-0.5f, -0.5f, -0.5f, 0.0f);

const al::UniformBlockLayout cEdgeUboLayout[] = {
    {0, agl::UniformBlock::cType_Float, 1},  {1, agl::UniformBlock::cType_Float, 1},
    {2, agl::UniformBlock::cType_Float, 1},  {3, agl::UniformBlock::cType_Float, 1},
    {4, agl::UniformBlock::cType_Float, 1},  {5, agl::UniformBlock::cType_Float, 1},
    {6, agl::UniformBlock::cType_Float, 1},  {7, agl::UniformBlock::cType_Float, 1},
    {8, agl::UniformBlock::cType_Float, 1},  {9, agl::UniformBlock::cType_Float, 1},
    {10, agl::UniformBlock::cType_Float, 1}, {11, agl::UniformBlock::cType_Vec4, 1},
    {12, agl::UniformBlock::cType_Vec4, 1},  {13, agl::UniformBlock::cType_Vec4, 1},
};

const al::UniformBlockLayout cReductionUboLayout[] = {
    {0, agl::UniformBlock::cType_Float, 1},
    {1, agl::UniformBlock::cType_Float, 1},
};

const char* sMacroValues[] = {"0", "0", "0", "0"};
const char* sMacroNames[] = {"REDUCTION_TYPE", "EDGE_TYPE", "RENDER_TYPE", "DEPTH_TYPE"};

template <typename T>
inline void setUniformData(const al::UniformBlock* pBlock, s32 memberIndex, T value) {
    pBlock->setValue(memberIndex, value);
}
}  // namespace

namespace al {
/**
 * Constructs the edge drawing parameters.
 */
EdgeDrawParam::EdgeDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "有効", "", true);
    mIsConstColor =
        new ParameterBool(false, mParamObj, "IsConstColor", "輪郭色を固定色にする", "", true);
    mIsBold = new ParameterBool(false, mParamObj, "IsBold", "太線", "", true);
    mBoundDepth = new ParameterF32(0.00015f, mParamObj, "BoundDepth", "深度エッジ境界値",
                                   "Min=0.f, Max=0.001f", true);
    mEdgeEndDepth = new ParameterF32(0.07f, mParamObj, "EdgeEndDepth", "エッジ有効深度",
                                     "Min=0.f, Max=1.f", true);
    mEdgePowerMin =
        new ParameterF32(0.1f, mParamObj, "EdgePowerMin", "エッジ濃さ", "Min=0.f, Max=1.f", true);
    mEdgeNormalEdgeBound = new ParameterF32(0.01f, mParamObj, "EdgeNormalEdgeBound",
                                            "法線エッジ境界値", "Min=0.f, Max=10.f", true);
    mOffsetColor = new ParameterC4f(cDefaultOffsetColor, mParamObj, "OffsetColor",
                                    "輪郭色スケーリング(固定色有効の場合は色)",
                                    "Min=-1.f, Max=1.f", true);
}

/**
 * Checks the IsEnable flag.
 * @return Whether edge drawing is enabled.
 */
bool EdgeDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Checks the IsConstColor flag.
 * @return Whether the edge color is constant.
 */
bool EdgeDrawParam::isConstColor() const {
    return mIsConstColor->getValue();
}

/**
 * Checks the IsBold flag.
 * @return Whether bold edges are drawn.
 */
bool EdgeDrawParam::isBold() const {
    return mIsBold->getValue();
}

/**
 * Gets the BoundDepth parameter.
 * @return BoundDepth.
 */
f32 EdgeDrawParam::getBoundDepth() const {
    return mBoundDepth->getValue();
}

/**
 * Gets the EdgeEndDepth parameter.
 * @return EdgeEndDepth.
 */
f32 EdgeDrawParam::getEdgeEndDepth() const {
    return mEdgeEndDepth->getValue();
}

/**
 * Gets the EdgePowerMin parameter.
 * @return EdgePowerMin.
 */
f32 EdgeDrawParam::getEdgePowerMin() const {
    return mEdgePowerMin->getValue();
}

/**
 * Gets the EdgeNormalEdgeBound parameter.
 * @return EdgeNormalEdgeBound.
 */
f32 EdgeDrawParam::getEdgeNormalEdgeBound() const {
    return mEdgeNormalEdgeBound->getValue();
}

/**
 * Gets the OffsetColor parameter.
 * @return OffsetColor.
 */
const sead::Color4f& EdgeDrawParam::getOffsetColor() const {
    return mOffsetColor->getValue();
}

/**
 * Constructs the edge drawer with room for one uniform block set per view.
 * @param pSystemInfo graphics system info
 * @param viewNum number of views
 */
EdgeDrawer::EdgeDrawer(const GraphicsSystemInfo* pSystemInfo, s32 viewNum)
    : mSystemInfo(pSystemInfo), mOffsetColor(cDefaultOffsetColor) {
    mQuadModel = new FullScreenQuadModel();
    mEdgeUniformBlocks.allocBuffer(viewNum, nullptr);
    mReductionUniformBlocks.allocBuffer(viewNum, nullptr);
}

/**
 * Destroys the uniform blocks and the quad model.
 */
EdgeDrawer::~EdgeDrawer() {
    while (!mReductionUniformBlocks.isEmpty()) {
        UniformBlock* pBlock = mReductionUniformBlocks.popBack();

        if (pBlock != nullptr) {
            delete pBlock;
        }
    }

    mReductionUniformBlocks.freeBuffer();

    while (!mEdgeUniformBlocks.isEmpty()) {
        UniformBlock* pBlock = mEdgeUniformBlocks.popBack();

        if (pBlock != nullptr) {
            delete pBlock;
        }
    }

    mEdgeUniformBlocks.freeBuffer();

    if (mQuadModel != nullptr) {
        delete mQuadModel;
        mQuadModel = nullptr;
    }
}

/**
 * Creates the uniform blocks and gets the edge shader.
 * @param pCameraInfo scene camera info
 * @param pShaderHolder shader holder to get the shader from
 * @param pAreaObjDirector area director used to find edge draw areas
 * @param pPlayerHolder player holder used to find edge draw areas
 */
void EdgeDrawer::init(const SceneCameraInfo* pCameraInfo, ShaderHolder* pShaderHolder,
                      AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder) {
    mCameraInfo = pCameraInfo;
    mAreaObjDirector = pAreaObjDirector;
    mPlayerHolder = pPlayerHolder;

    for (s32 i = 0; i < mEdgeUniformBlocks.capacity(); i++) {
        mEdgeUniformBlocks.pushBack(createUniformBlock(cEdgeUboLayout, 14, nullptr, 2));
    }

    for (s32 i = 0; i < mReductionUniformBlocks.capacity(); i++) {
        mReductionUniformBlocks.pushBack(createUniformBlock(cReductionUboLayout, 2, nullptr, 2));
    }

    mShaderProgram = pShaderHolder->getShaderProgram("EdgeEffect");
}

/**
 * Takes the edge parameters from the edge draw area the player or camera is in.
 */
void EdgeDrawer::update() {
    if (mPlayerHolder == nullptr || !mIsEnableArea) {
        return;
    }

    const EdgeDrawArea* pArea = nullptr;

    if (static_cast<s32>(mSystemInfo->getAreaTarget()) == GraphicsAreaTarget::Player) {
        pArea = static_cast<const EdgeDrawArea*>(
            tryFindAreaObjPlayerOne(this, "EdgeDrawArea", mPlayerHolder));
    } else if (static_cast<s32>(mSystemInfo->getAreaTarget()) == GraphicsAreaTarget::CameraPos) {
        sead::Vector3f pos = mCameraInfo->mLookAtCamera->getPos();
        pArea = static_cast<const EdgeDrawArea*>(tryFindAreaObj(this, "EdgeDrawArea", pos));
    } else if (static_cast<s32>(mSystemInfo->getAreaTarget()) ==
               GraphicsAreaTarget::CameraLookAt) {
        sead::Vector3f pos = mCameraInfo->mLookAtCamera->getAt();
        pArea = static_cast<const EdgeDrawArea*>(tryFindAreaObj(this, "EdgeDrawArea", pos));
    }

    if (pArea != nullptr) {
        mIsInArea = true;
        mIsConstColor = pArea->isConstColor();
        mBoundDepth = pArea->getBoundDepth();
        mEdgeEndDepth = pArea->getEdgeEndDepth();
        mEdgePowerMin = pArea->getEdgePowerMin();
        mOffsetColor = pArea->getOffsetColor();
    } else {
        mIsInArea = false;
        mIsConstColor = false;
        mBoundDepth = 0.00015f;
        mEdgeEndDepth = 0.07f;
        mEdgePowerMin = 0.1f;
        mOffsetColor = cDefaultOffsetColor;
    }
}

/**
 * Draws the edges if enabled or inside an edge draw area.
 * @param pRenderBuffer render buffer to draw into
 * @param pEdgeBuffer render buffer holding the previous edge result
 * @param pNormalTexture view normal texture
 * @param pDepthTexture view depth texture
 * @param mode shader mode to draw with
 * @param viewIndex index of the view
 * @param near near clip distance
 * @param far far clip distance
 * @return shader mode after drawing
 */
agl::ShaderMode EdgeDrawer::draw(const agl::RenderBuffer* pRenderBuffer,
                                 const agl::RenderBuffer* pEdgeBuffer,
                                 const agl::TextureData* pNormalTexture,
                                 const agl::TextureData* pDepthTexture, agl::ShaderMode mode,
                                 s32 viewIndex, f32 near, f32 far) const {
    if (mIsEnable || (mIsEnableArea && mIsInArea)) {
        return drawEdge(pRenderBuffer, pEdgeBuffer, pNormalTexture, pDepthTexture, mode, viewIndex,
                        near, far, true);
    }

    return mode;
}

/**
 * Draws the edges from the view normal and depth textures.
 * @param pRenderBuffer render buffer to draw into
 * @param pEdgeBuffer render buffer holding the previous edge result
 * @param pNormalTexture view normal texture
 * @param pDepthTexture view depth texture
 * @param mode shader mode to draw with
 * @param viewIndex index of the view
 * @param near near clip distance
 * @param far far clip distance
 * @param isKeepBuffer whether to keep the render buffer contents instead of clearing them
 * @return shader mode after drawing
 */
agl::ShaderMode EdgeDrawer::drawEdge(const agl::RenderBuffer* pRenderBuffer,
                                     const agl::RenderBuffer* pEdgeBuffer,
                                     const agl::TextureData* pNormalTexture,
                                     const agl::TextureData* pDepthTexture, agl::ShaderMode mode,
                                     s32 viewIndex, f32 near, f32 far, bool isKeepBuffer) const {
    sMacroValues[0] = "0";
    const char* edgeType = "0";

    if (mBoundDepth <= 0.0f) {
        edgeType = "1";
    }

    sMacroValues[1] = edgeType;

    if (isKeepBuffer) {
        sMacroValues[2] = mIsConstColor ? "5" : "0";
    } else {
        sMacroValues[2] = "2";
    }

    const agl::ShaderProgram* pProgram =
        mShaderProgram->searchVariation(4, sMacroNames, sMacroValues);

    pRenderBuffer->bind(GameFrameworkNx::getAglDrawContext());
    sead::Viewport viewport(*pRenderBuffer);
    viewport.apply(GameFrameworkNx::getAglDrawContext(), *pRenderBuffer);

    agl::TextureSampler normalSampler;
    agl::SamplerLocation normalLocation("cViewNormal");
    normalLocation.search(*pProgram);

    if (normalLocation.isValid()) {
        normalSampler.applyTextureData(*pNormalTexture);
        normalSampler.setMagFilter(0);
        normalSampler.activate(GameFrameworkNx::getAglDrawContext(), normalLocation, -1, false);
    }

    agl::TextureSampler depthSampler;
    agl::SamplerLocation depthLocation("cViewDepth");
    depthLocation.search(*pProgram);
    depthSampler.applyTextureData(*pDepthTexture);
    depthSampler.setMagFilter(0);
    depthSampler.activate(GameFrameworkNx::getAglDrawContext(), depthLocation, -1, false);

    agl::SamplerLocation edgeLocation("cTextureEdge");
    edgeLocation.search(*pProgram);

    if (edgeLocation.isValid()) {
        agl::TextureSampler edgeSampler(*pEdgeBuffer->getRenderTargetColor());
        edgeSampler.activate(GameFrameworkNx::getAglDrawContext(), edgeLocation, -1, false);
    }

    mEdgeUniformBlocks[viewIndex]->swap();
    setUniformData(mEdgeUniformBlocks(viewIndex), 0, near);
    setUniformData(mEdgeUniformBlocks(viewIndex), 1, near);
    setUniformData(mEdgeUniformBlocks(viewIndex), 2, far - near);
    setUniformData(mEdgeUniformBlocks(viewIndex), 3, mBoundDepth);
    setUniformData(mEdgeUniformBlocks(viewIndex), 4, 3.75f);
    setUniformData(mEdgeUniformBlocks(viewIndex), 9, mEdgeEndDepth);
    setUniformData(mEdgeUniformBlocks(viewIndex), 10, mEdgePowerMin);
    setUniformData(mEdgeUniformBlocks(viewIndex), 5, 1.0f / pDepthTexture->getWidth(0));
    setUniformData(mEdgeUniformBlocks[viewIndex], 6, 1.0f / pDepthTexture->getHeight(0));
    setUniformData(mEdgeUniformBlocks(viewIndex), 7, 1.0f / pNormalTexture->getWidth(0));
    setUniformData(mEdgeUniformBlocks[viewIndex], 8, 1.0f / pNormalTexture->getHeight(0));
    mEdgeUniformBlocks(viewIndex)->setData(11, &mEdgeColor, 0, 1);
    mEdgeUniformBlocks(viewIndex)->setData(12, &mEdgeScale, 0, 1);
    mEdgeUniformBlocks(viewIndex)->setData(13, &mOffsetColor, 0, 1);
    mEdgeUniformBlocks[viewIndex]->flushCurrentBuffer();

    agl::UniformBlockLocation uboLocation("EdgeEffect");
    uboLocation.search(*pProgram);
    mEdgeUniformBlocks[viewIndex]->activate(GameFrameworkNx::getAglDrawContext(), uboLocation);

    GameFrameworkNx::getAglDrawContext()->setShaderMode(mode);
    pProgram->activate(GameFrameworkNx::getAglDrawContext(), true);
    auto resultMode =
        static_cast<agl::ShaderMode>(GameFrameworkNx::getAglDrawContext()->getShaderMode());

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthFunc(8);
    graphicsContext.setBlendEnable(true);
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setAlphaTestEnable(false);
    graphicsContext.apply(GameFrameworkNx::getAglDrawContext());

    if (!isKeepBuffer) {
        pRenderBuffer->clear(GameFrameworkNx::getAglDrawContext(), 1,
                             sead::Color4f(0.0f, 0.0f, 0.0f, 0.0f), 0.0f, 0);
    }

    mQuadModel->drawQuad();
    return resultMode;
}

/**
 * Reduces the view depth texture into the render buffer.
 * @param rRenderBuffer render buffer to draw into
 * @param pDepthTexture view depth texture
 * @param viewIndex index of the view
 * @param mode shader mode to draw with
 * @param isMinDepth whether to keep the minimum depth
 * @return shader mode after drawing
 */
agl::ShaderMode EdgeDrawer::drawReduction(const agl::RenderBuffer& rRenderBuffer,
                                          const agl::TextureData* pDepthTexture, s32 viewIndex,
                                          agl::ShaderMode mode, bool isMinDepth) const {
    sMacroValues[0] = isMinDepth ? "2" : "0";
    sMacroValues[1] = "0";
    sMacroValues[2] = "1";

    const agl::ShaderProgram* pProgram =
        mShaderProgram->searchVariation(4, sMacroNames, sMacroValues);

    rRenderBuffer.bind(GameFrameworkNx::getAglDrawContext());
    sead::Viewport viewport(rRenderBuffer);
    viewport.apply(GameFrameworkNx::getAglDrawContext(), rRenderBuffer);

    agl::TextureSampler depthSampler;
    agl::SamplerLocation depthLocation("cViewDepth");
    depthLocation.search(*pProgram);
    depthSampler.applyTextureData(*pDepthTexture);
    depthSampler.setMagFilter(0);
    depthSampler.activate(GameFrameworkNx::getAglDrawContext(), depthLocation, -1, false);

    f32 width = pDepthTexture->getWidth(0);
    f32 height = pDepthTexture->getHeight(0);

    mReductionUniformBlocks[viewIndex]->swap();
    setUniformData(mReductionUniformBlocks(viewIndex), 0, 1.0f / width);
    setUniformData(mReductionUniformBlocks(viewIndex), 1, 1.0f / height);
    mReductionUniformBlocks[viewIndex]->flushCurrentBuffer();

    agl::UniformBlockLocation uboLocation("Reduction");
    uboLocation.search(*pProgram);
    mReductionUniformBlocks[viewIndex]->activate(GameFrameworkNx::getAglDrawContext(),
                                                 uboLocation);

    GameFrameworkNx::getAglDrawContext()->setShaderMode(mode);
    pProgram->activate(GameFrameworkNx::getAglDrawContext(), true);
    auto resultMode =
        static_cast<agl::ShaderMode>(GameFrameworkNx::getAglDrawContext()->getShaderMode());

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthFunc(8);
    graphicsContext.setBlendEnable(false);
    graphicsContext.setDepthEnable(true, false);
    graphicsContext.setAlphaTestEnable(false);
    graphicsContext.apply(GameFrameworkNx::getAglDrawContext());

    mQuadModel->drawQuad();
    return resultMode;
}

/**
 * Mixes the drawn edges into the render buffer.
 * @param rRenderBuffer render buffer to draw into
 * @param pDepthTexture view depth texture
 * @param mode shader mode to draw with
 * @return shader mode after drawing
 */
agl::ShaderMode EdgeDrawer::drawMix(const agl::RenderBuffer& rRenderBuffer,
                                    const agl::TextureData* pDepthTexture,
                                    agl::ShaderMode mode) const {
    sMacroValues[0] = "1";
    sMacroValues[1] = "0";
    sMacroValues[2] = "3";

    const agl::ShaderProgram* pProgram =
        mShaderProgram->searchVariation(4, sMacroNames, sMacroValues);

    rRenderBuffer.bind(GameFrameworkNx::getAglDrawContext());
    sead::Viewport viewport(rRenderBuffer);
    viewport.apply(GameFrameworkNx::getAglDrawContext(), rRenderBuffer);

    agl::TextureSampler depthSampler;
    agl::SamplerLocation depthLocation("cViewDepth");
    depthLocation.search(*pProgram);
    depthSampler.applyTextureData(*pDepthTexture);
    depthSampler.setMagFilter(0);
    depthSampler.activate(GameFrameworkNx::getAglDrawContext(), depthLocation, -1, false);

    GameFrameworkNx::getAglDrawContext()->setShaderMode(mode);
    pProgram->activate(GameFrameworkNx::getAglDrawContext(), true);
    auto resultMode =
        static_cast<agl::ShaderMode>(GameFrameworkNx::getAglDrawContext()->getShaderMode());

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthFunc(8);
    graphicsContext.setBlendFactorSrcRGB(0, 5);
    graphicsContext.setBlendFactorSrcA(0, 1);
    graphicsContext.setBlendFactorDstRGB(0, 6);
    graphicsContext.setBlendFactorDstA(0, 2);
    graphicsContext.setBlendEnable(true);
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setAlphaTestEnable(false);
    graphicsContext.apply(GameFrameworkNx::getAglDrawContext());

    mQuadModel->drawQuad();
    return resultMode;
}
}  // namespace al
