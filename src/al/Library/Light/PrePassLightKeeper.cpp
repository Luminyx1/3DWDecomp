#include "Library/Light/PrePassLightKeeper.hpp"

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <hostio/seadHostIOCurve.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>
#include <prim/seadMemUtil.h>

#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterCurve.hpp"
#include "utility/aglDevTools.h"
#include "utility/aglPrimitiveShape.h"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Light/LppBase.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"

namespace {

const f32 cSpecularCurve[12] = {0.0f,        1.0f,        0.4817518f,   0.956896484f,
                                0.684306622f, 0.866379321f, 0.824817479f, 0.685344815f,
                                0.937956214f, 0.435344785f, 1.0f,         0.202586204f};

const al::UniformBlockLayout cPointLightLayout[] = {
    {0, agl::UniformBlock::cType_Vec4, 1}, {1, agl::UniformBlock::cType_Vec4, 1},
    {2, agl::UniformBlock::cType_Vec4, 1}, {3, agl::UniformBlock::cType_Vec4, 1},
    {4, agl::UniformBlock::cType_Vec4, 4}, {5, agl::UniformBlock::cType_Vec4, 1},
};

const al::UniformBlockLayout cSpotLightLayout[] = {
    {0, agl::UniformBlock::cType_Vec4, 1}, {1, agl::UniformBlock::cType_Vec4, 1},
    {2, agl::UniformBlock::cType_Vec4, 1}, {3, agl::UniformBlock::cType_Vec4, 1},
    {4, agl::UniformBlock::cType_Vec4, 1}, {5, agl::UniformBlock::cType_Vec4, 4},
    {6, agl::UniformBlock::cType_Vec4, 1}, {7, agl::UniformBlock::cType_Vec4, 4},
    {8, agl::UniformBlock::cType_Vec2, 1},
};

const al::UniformBlockLayout cLineLightLayout[] = {
    {0, agl::UniformBlock::cType_Vec4, 1}, {1, agl::UniformBlock::cType_Vec4, 1},
    {2, agl::UniformBlock::cType_Vec4, 1}, {3, agl::UniformBlock::cType_Vec4, 4},
    {4, agl::UniformBlock::cType_Vec4, 1},
};

const al::UniformBlockLayout cProjLightLayout[] = {
    {0, agl::UniformBlock::cType_Vec4, 4},
    {1, agl::UniformBlock::cType_Vec4, 4},
    {2, agl::UniformBlock::cType_Vec4, 4},
    {3, agl::UniformBlock::cType_Vec4, 7},
};

/**
 * Copies a vector as a plain struct copy.
 * @param pDst destination
 * @param rSrc source
 */
void copyVec3(sead::Vector3f* pDst, const sead::Vector3f& rSrc) {
    static_cast<sead::BaseVec3<f32>&>(*pDst) = rSrc;
}

/**
 * Computes the model matrix of the cylinder drawn for a line light.
 * @param pMtx output model matrix
 * @param pLength output length of the cylinder
 * @param rLight light
 * @return false if the line is degenerate
 */
bool calcLineLightMtx(sead::Matrix34f* pMtx, f32* pLength, const al::LineLightParam& rLight) {
    sead::Vector3f center = (rLight.mStart + rLight.mEnd) * 0.5f;
    sead::Vector3f dir;
    sead::Vector3f axis = rLight.mEnd - rLight.mStart;
    al::separateScalarAndDirection(pLength, &dir, axis);

    if (al::isNearZero(*pLength, 0.001f)) {
        return false;
    }

    *pLength += rLight.mRadius + rLight.mRadius;
    copyVec3(&axis, sead::Vector3f::ez);

    if (al::isParallelDirection(axis, dir, 0.01f)) {
        copyVec3(&axis, sead::Vector3f::ex);
    }

    al::makeMtxUpFrontPos(pMtx, dir, axis, center);
    f32 diameter = rLight.mRadius + rLight.mRadius;
    al::preScaleMtx(pMtx, sead::Vector3f(diameter, *pLength, diameter));
    return true;
}

}  // namespace

namespace al {

/**
 * Returns the specular power written into the light uniform blocks.
 * @return specular power
 */
f32 PrePassLightKeeper::getSpecularPower() const {
    return 30.0f;
}

/**
 * Returns the fresnel factor written into the light uniform blocks.
 * @return fresnel factor
 */
f32 PrePassLightKeeper::getFlesnel() const {
    return 0.1f;
}

/**
 * Creates the albedo mode light managers.
 * @param pInfo graphics system info
 * @param viewNum number of views
 */
PrePassLightKeeper::PrePassLightKeeper(GraphicsSystemInfo* pInfo, s32 viewNum)
    : mGraphicsSystemInfo(pInfo), mViewNum(viewNum), mPointLightNum(0), mSpotLightNum(0),
      mProjLightNum(0), mLineLightNum(0) {
    mPointLightMgr = new AlbedoModePointLightMgr(this);
    mSpotLightMgr = new AlbedoModeSpotLightMgr(this);
    mLineLightMgr = new AlbedoModeLineLightMgr(this);
    mProjLightMgr = new AlbedoModeProjLightMgr(this);
    mIsPaused = false;
}

/**
 * Destroys the light pre-pass and the light managers.
 */
PrePassLightKeeper::~PrePassLightKeeper() {
    if (mLightPrePass != nullptr) {
        delete mLightPrePass;
        mLightPrePass = nullptr;
    }

    mPointLightMgr->destroy();
    delete mPointLightMgr;
    mPointLightMgr = nullptr;

    mSpotLightMgr->destroy();
    delete mSpotLightMgr;
    mSpotLightMgr = nullptr;

    mLineLightMgr->destroy();
    delete mLineLightMgr;
    mLineLightMgr = nullptr;

    mProjLightMgr->destroy();
    delete mProjLightMgr;
    mProjLightMgr = nullptr;
}

/**
 * Looks up the light shaders of every light manager.
 * @param pHolder shader holder
 */
void PrePassLightKeeper::initShader(ShaderHolder* pHolder) {
    mPointLightMgr->setShader(pHolder->getShaderProgram("LppPointLight"));
    mSpotLightMgr->setShader(pHolder);
    mLineLightMgr->setShader(pHolder->getShaderProgram("LppLineLight"));
    mProjLightMgr->setShader(pHolder->getShaderProgram("LppProjLight"));
}

/**
 * Sets the point light shader and searches its uniform blocks.
 * @param pProgram shader program
 */
void AlbedoModePointLightMgr::setShader(const agl::ShaderProgram* pProgram) {
    mShaderInfo->setShader(pProgram);
}

/**
 * Sets the spot light shader and searches its uniform blocks.
 * @param pHolder shader holder
 */
void AlbedoModeSpotLightMgr::setShader(ShaderHolder* pHolder) {
    mShaderInfo->setShader(pHolder->getShaderProgram("LppSpotLight"));
}

/**
 * Sets the line light shader and searches its uniform blocks.
 * @param pProgram shader program
 */
void AlbedoModeLineLightMgr::setShader(const agl::ShaderProgram* pProgram) {
    mShaderInfo->setShader(pProgram);
}

/**
 * Sets the projection light shader and searches its uniform blocks.
 * @param pProgram shader program
 */
void AlbedoModeProjLightMgr::setShader(const agl::ShaderProgram* pProgram) {
    mShaderInfo->setShader(pProgram);
}

/**
 * Creates agl's light pre-pass with the four albedo mode light managers as user light managers,
 * sizes them from the declared light counts and sets up the shared vertex attributes and the
 * graphics context used to draw the lights.
 */
void PrePassLightKeeper::endInit() {
    mLightPrePass = new agl::lght::LightPrePass();
    mLightPrePass->getFlags().reset(1 << 9);

    agl::lght::LightPrePass::CreateArg arg;
    arg.mViewNum = mViewNum;
    arg.mUserLightMgrNum = 4;
    arg.mPointLightNum = 1;
    arg.mSpotLightNum = 1;
    arg.mProjLightNum = 1;
    mLightPrePass->initialize(arg, getCurrentHeap());
    mLightPrePass->getFlags().set(1 << 8);
    mLightPrePass->setSpecPowScale(mSpecularPowerScale);

    agl::utl::ParameterCurve<2> curve;

    for (u32 i = 0; i < 2; i++) {
        curve.getCurve(i).setData(&curve.getCurveData(i), sead::hostio::CurveType::Linear2D, 30,
                                  12);
        sead::MemUtil::copy(curve.getCurveData(i).f, cSpecularCurve, sizeof(cSpecularCurve));
    }

    mLightPrePass->setSpecularCurve(curve);
    mLightPrePass->getPointLightMgr()->setValidNum(0);
    mLightPrePass->getSpotLightMgr()->setValidNum(0);
    mLightPrePass->getProjLightMgr()->setValidNum(0);
    mLightPrePass->setUserLightMgr(0, mPointLightMgr);
    mLightPrePass->setUserLightMgr(1, mSpotLightMgr);
    mLightPrePass->setUserLightMgr(2, mLineLightMgr);
    mLightPrePass->setUserLightMgr(3, mProjLightMgr);

    mPointLightMgr->initialize(mLightPrePass, mPointLightNum > 1 ? mPointLightNum : 1, mViewNum,
                               nullptr);
    mPointLightMgr->setValidNum(0);
    mSpotLightMgr->initialize(mLightPrePass, mSpotLightNum > 1 ? mSpotLightNum : 1, mViewNum,
                              nullptr);
    mSpotLightMgr->setValidNum(0);
    mLineLightMgr->initialize(mLightPrePass, mLineLightNum > 1 ? mLineLightNum : 1, mViewNum,
                              nullptr);
    mLineLightMgr->setValidNum(0);
    mProjLightMgr->initialize(mLightPrePass, mProjLightNum > 1 ? mProjLightNum : 1, mViewNum,
                              nullptr);
    mProjLightMgr->setValidNum(0);

    mQuadAttribute.create(1, getCurrentHeap());
    mQuadAttribute.setVertexStream(
        0, &agl::utl::PrimitiveShape::instance()->getQuadVertexBuffer(), 0);
    mQuadAttribute.setUp();

    mSphereAttribute.create(1, getCurrentHeap());
    mSphereAttribute.setVertexStream(
        0, &agl::utl::PrimitiveShape::instance()->getSphereVertexBuffer(), 0);
    mSphereAttribute.setUp();

    mCylinderAttribute.create(1, getCurrentHeap());
    mCylinderAttribute.setVertexStream(
        0, &agl::utl::PrimitiveShape::instance()->getCylinderVertexBuffer(), 0);
    mCylinderAttribute.setUp();

    mConeAttribute.create(1, getCurrentHeap());
    mConeAttribute.setVertexStream(
        0, &agl::utl::PrimitiveShape::instance()->getConeVertexBuffer(), 0);
    mConeAttribute.setUp();

    mCubeAttribute.create(1, getCurrentHeap());
    mCubeAttribute.setVertexStream(
        0, &agl::utl::PrimitiveShape::instance()->getCubeVertexBuffer(), 0);
    mCubeAttribute.setUp();

    mGraphicsContext.setBlendEnable(true);
    mGraphicsContext.setBlendFactorSrcRGB(0, 2);
    mGraphicsContext.setBlendFactorDstRGB(0, 2);
    mGraphicsContext.setColorMask(true, true, true, true);
    mGraphicsContext.setDepthEnable(true, false);
    mGraphicsContext.setDepthFunc(5);
    mGraphicsContext.setCullingMode(1);
}

/**
 * Resets the per-frame light requests of every light manager.
 */
void PrePassLightKeeper::clear() {
    mPointLightRequestNum = 0;
    mSpotLightRequestNum = 0;
    mProjLightRequestNum = 0;
    mLineLightRequestNum = 0;
    mProjLightMgr->setValidNum(0);
    mLineLightMgr->setValidNum(0);
    mSpotLightMgr->setValidNum(0);
    mPointLightMgr->setValidNum(0);
}

/**
 * Registers a light to be executed every frame.
 * @param pLight light to register
 */
void PrePassLightKeeper::pushBackLight(PrePassLightBase* pLight) {
    mLightList.pushBack(pLight);
}

/**
 * Unregisters a light.
 * @param pLight light to unregister
 */
void PrePassLightKeeper::eraseLight(PrePassLightBase* pLight) {
    mLightList.erase(pLight);
}

/**
 * Sets the shadow map of a spot light.
 * @param index light index
 * @param view view index
 * @param pShadowMap shadow map sampler
 * @param rShadowMtx shadow matrix
 * @param shadowType shadow type
 * @param shadowParam shadow parameter
 * @param isBlackBorder whether the shadow border is black instead of white
 */
void PrePassLightKeeper::setSpotLightShadowMap(s32 index, s32 view,
                                               const agl::TextureSampler* pShadowMap,
                                               const sead::Matrix44f& rShadowMtx,
                                               agl::lght::LightPrePass::ShadowType shadowType,
                                               f32 shadowParam, bool isBlackBorder) {
    mSpotLightMgr->setSpotLightShadowMap(index, view, pShadowMap, rShadowMtx, shadowType,
                                         shadowParam, isBlackBorder);
}

/**
 * Sets the shadow map of a spot light.
 * @param index light index
 * @param view view index
 * @param pShadowMap shadow map sampler
 * @param rShadowMtx shadow matrix
 * @param shadowType shadow type
 * @param shadowParam shadow parameter
 * @param isBlackBorder whether the shadow border is black instead of white
 */
void AlbedoModeSpotLightMgr::setSpotLightShadowMap(s32 index, s32 view,
                                                   const agl::TextureSampler* pShadowMap,
                                                   const sead::Matrix44f& rShadowMtx,
                                                   agl::lght::LightPrePass::ShadowType shadowType,
                                                   f32 shadowParam, bool isBlackBorder) {
    SpotLightData& rLight = getLight(index);
    rLight.mView[view].mShadowMap = pShadowMap;
    rLight.mView[view].mShadowMtx = rShadowMtx;
    rLight.mView[view].mShadowSampler->setBorderColorDirect(
        isBlackBorder ? sead::Color4f::cBlack : sead::Color4f::cWhite);
    rLight.mShadowType = shadowType;
    rLight.mShadowParam = shadowParam;
}

/**
 * Sets the shadow map of a projection light.
 * @param index light index
 * @param view view index
 * @param pShadowMap shadow map sampler
 * @param rShadowMtx shadow matrix
 * @param shadowType shadow type
 * @param shadowParam shadow parameter
 * @param isBlackBorder whether the shadow border is black instead of white
 */
void PrePassLightKeeper::setProjLightShadowMap(s32 index, s32 view,
                                               const agl::TextureSampler* pShadowMap,
                                               const sead::Matrix44f& rShadowMtx,
                                               agl::lght::LightPrePass::ShadowType shadowType,
                                               f32 shadowParam, bool isBlackBorder) {
    mProjLightMgr->setProjLightShadowMap(index, view, pShadowMap, rShadowMtx, shadowType,
                                         shadowParam, isBlackBorder);
}

/**
 * Sets the shadow map of a projection light.
 * @param index light index
 * @param view view index
 * @param pShadowMap shadow map sampler
 * @param rShadowMtx shadow matrix
 * @param shadowType shadow type
 * @param shadowParam shadow parameter
 * @param isBlackBorder whether the shadow border is black instead of white
 */
void AlbedoModeProjLightMgr::setProjLightShadowMap(s32 index, s32 view,
                                                   const agl::TextureSampler* pShadowMap,
                                                   const sead::Matrix44f& rShadowMtx,
                                                   agl::lght::LightPrePass::ShadowType shadowType,
                                                   f32 shadowParam, bool isBlackBorder) {
    ProjLightData& rLight = getLight(index);
    rLight.mView[view].mShadowMap = pShadowMap;
    rLight.mView[view].mShadowMtx = rShadowMtx;
    rLight.mView[view].mShadowSampler->setBorderColorDirect(
        isBlackBorder ? sead::Color4f::cBlack : sead::Color4f::cWhite);
    rLight.mShadowType = shadowType;
    rLight.mShadowParam = shadowParam;
}

/**
 * Executes every registered light.
 * @param pDirector shadow director (unused)
 * @param isPaused whether the scene is paused
 */
void PrePassLightKeeper::execute(ShadowDirector* pDirector, bool isPaused) {
    mIsPaused = isPaused;

    if (mLightPrePass == nullptr) {
        return;
    }

    for (auto& rNode : mLightList.robustRange()) {
        rNode.mData->execute();
    }
}

/**
 * Updates the light pre-pass before drawing.
 */
void PrePassLightKeeper::preDrawGraphics() {
    if (mLightPrePass == nullptr) {
        return;
    }

    mLightPrePass->calc();
    mLightPrePass->updateGPU();
}

/**
 * Sets up the light shadows and updates the light uniform blocks of a view.
 * @param view view index
 * @param pDirector shadow director
 */
void PrePassLightKeeper::updateViewGPU(s32 view, ShadowDirector* pDirector) {
    if (pDirector != nullptr) {
        for (auto* pLight : mLightList) {
            pLight->trySetupShadow(view, this, pDirector);
        }
    }

    mLightPrePass->updateViewGPU(view);
}

/**
 * Requests a point light for the current frame.
 * @param rPos position
 * @param radius radius
 * @param rColor color
 * @param attnPow attenuation power
 * @param attnStart attenuation start
 * @param isEnableSpecular whether the light has specular
 * @param isUseSpecularColor whether rSpecularColor is used instead of rColor for specular
 * @param rSpecularColor specular color
 * @param lightShaderFunc light shader function
 */
void PrePassLightKeeper::requestPointLight(const sead::Vector3f& rPos, f32 radius,
                                           const sead::Color4f& rColor, f32 attnPow,
                                           f32 attnStart, bool isEnableSpecular,
                                           bool isUseSpecularColor,
                                           const sead::Color4f& rSpecularColor,
                                           s32 lightShaderFunc) {
    if (mPointLightMgr->getLightMax() <= mPointLightRequestNum) {
        return;
    }

    if (rColor.r == 0.0f && rColor.g == 0.0f && rColor.b == 0.0f) {
        return;
    }

    PointLightData& rLight = mPointLightMgr->getLight(mPointLightRequestNum);
    copyVec3(&rLight.mPos, rPos);
    rLight.mAttnPow = sead::Mathf::clampMin(attnPow, 0.001f);
    bool isInvalidRadius = radius <= 0.0f;
    rLight.mRadius = isInvalidRadius ? 1.0f : radius;
    rLight.mAttnStart = attnStart;
    rLight.mColor = isInvalidRadius ? sead::Color4f::cBlack : rColor;
    rLight.mSpecColor = isUseSpecularColor ? rSpecularColor : rColor;
    rLight.mFlags.change(2, isEnableSpecular);
    rLight.mFlags.change(4, lightShaderFunc != 0);
    mPointLightRequestNum++;
    mPointLightMgr->setValidNum(mPointLightRequestNum);
}

/**
 * Requests a spot light for the current frame.
 * @param rPos position
 * @param rDir direction
 * @param angle cone angle
 * @param length cone length
 * @param rColor color
 * @param attnPow attenuation power
 * @param angleAttnPow angle attenuation power
 * @param angleAttnStart angle attenuation start
 * @param isEnableSpecular whether the light has specular
 * @param isUseSpecularColor whether rSpecularColor is used instead of rColor for specular
 * @param rSpecularColor specular color
 * @return index of the light, -1 if it was not added
 */
s32 PrePassLightKeeper::requestSpotLight(const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                                         f32 angle, f32 length, const sead::Color4f& rColor,
                                         f32 attnPow, f32 angleAttnPow, f32 angleAttnStart,
                                         bool isEnableSpecular, bool isUseSpecularColor,
                                         const sead::Color4f& rSpecularColor) {
    if (mSpotLightMgr->getLightMax() <= mSpotLightRequestNum ||
        (rColor.r == 0.0f && rColor.g == 0.0f && rColor.b == 0.0f)) {
        return -1;
    }

    s32 index = mSpotLightRequestNum;

    SpotLightData& rLight = mSpotLightMgr->getLight(index);
    copyVec3(&rLight.mPos, rPos);
    copyVec3(&rLight.mDir, rDir);
    rLight.mLength = length;
    rLight.mAngle = angle;
    rLight.mAngleAttnStart = angleAttnStart;
    rLight.mAttnPow = attnPow;
    rLight.mAngleAttnPow = angleAttnPow;
    rLight.mColor = rColor;
    rLight.mSpecColor = isUseSpecularColor ? rSpecularColor : rColor;
    rLight.calcAuxiliary();
    rLight.mFlags.change(2, isEnableSpecular);
    rLight.mShadowType = agl::lght::LightPrePass::cShadowType_None;
    rLight.mShadowParam = 0.5f;
    rLight.clearShadowMap();
    mSpotLightRequestNum++;
    mSpotLightMgr->setValidNum(mSpotLightRequestNum);
    return index;
}

/**
 * Requests a perspective projection light for the current frame.
 * @param rPos position
 * @param rDir direction
 * @param rUp up vector
 * @param rColor color
 * @param near near clip distance
 * @param far far clip distance
 * @param fovy vertical field of view
 * @param aspect aspect ratio
 * @param attnPow attenuation power
 * @param isEnableSpecular whether the light has specular
 * @param isUseSpecularColor whether rSpecularColor is used instead of rColor for specular
 * @param rSpecularColor specular color
 * @param pTexture projected texture, nullptr for none
 * @param isTextureWrap whether the projected texture repeats instead of being clamped
 * @param rTexScale texture scale
 * @param rTexOffset texture offset
 * @return index of the light, -1 if it was not added
 */
s32 PrePassLightKeeper::requestProjLight(const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                                         const sead::Vector3f& rUp, const sead::Color4f& rColor,
                                         f32 near, f32 far, f32 fovy, f32 aspect, f32 attnPow,
                                         bool isEnableSpecular, bool isUseSpecularColor,
                                         const sead::Color4f& rSpecularColor,
                                         agl::TextureSampler* pTexture, bool isTextureWrap,
                                         const sead::Vector2f& rTexScale,
                                         const sead::Vector2f& rTexOffset) {
    if (mProjLightMgr->getLightMax() <= mProjLightRequestNum ||
        (rColor.r == 0.0f && rColor.g == 0.0f && rColor.b == 0.0f)) {
        return -1;
    }

    s32 index = mProjLightRequestNum;

    ProjLightData& rLight = mProjLightMgr->getLight(index);
    copyVec3(&rLight.mPos, rPos);
    copyVec3(&rLight.mDir, rDir);
    copyVec3(&rLight.mUp, rUp);
    rLight.mColor = rColor;
    rLight.mSpecColor = isUseSpecularColor ? rSpecularColor : rColor;
    rLight.mParam[0] = near;
    rLight.mParam[1] = far;
    rLight.mParam[2] = fovy;
    rLight.mParam[3] = aspect;
    rLight.mAttnPow = attnPow;
    rLight.mTexScale = rTexScale;
    rLight.mTexOffset = rTexOffset;
    rLight.mFlags.change(2, isEnableSpecular);
    rLight.mFlags.reset(4);
    rLight.setVisibleAll(true);

    if (pTexture != nullptr) {
        rLight.mHasTexture = true;
        rLight.mTexture = *pTexture;

        if (isTextureWrap) {
            rLight.mFlags.set(0x10);
        } else {
            rLight.mFlags.reset(0x10);
            rLight.mTexture.setWrap(5, 5, 5);
            rLight.mTexture.setBorderColorAsColor(sead::Color4f(0.0f, 0.0f, 0.0f, 0.0f));
        }
    } else {
        rLight.mHasTexture = false;
    }

    rLight.setNormVec();
    rLight.mShadowType = agl::lght::LightPrePass::cShadowType_None;
    rLight.mShadowParam = 0.5f;
    rLight.clearShadowMap();
    mProjLightMgr->updateParameters_(rLight);
    mProjLightRequestNum++;
    mProjLightMgr->setValidNum(mProjLightRequestNum);
    return index;
}

/**
 * Recomputes the frustum size and the world bounding box of a projection light.
 * @param rLight light
 */
void AlbedoModeProjLightMgr::updateParameters_(ProjLightData& rLight) {
    sead::Vector3f side;
    side.setCross(rLight.mNormDir, rLight.mNormUp);
    side.normalize();
    sead::Vector3f up;
    up.setCross(side, rLight.mNormDir);
    up.normalize();

    if (rLight.mFlags.isOff(4)) {
        f32 tanHalfFovy = sead::Mathf::tan(rLight.mParam[2] * 0.5f);
        f32 near = rLight.mParam[0];
        f32 far = rLight.mParam[1];
        const sead::Vector3f& rDir = rLight.mNormDir;
        sead::Vector3f nearCenter;
        nearCenter.setScaleAdd(near, rDir, rLight.mPos);
        f32 ratio = near / far;
        rLight.mTanHalfFovy = tanHalfFovy;
        f32 halfHeight = tanHalfFovy * far;
        f32 halfWidth = halfHeight * rLight.mParam[3];
        sead::Vector3f farSide = side * halfWidth;
        rLight.mDebugParam0 = halfWidth;
        rLight.mDebugParam1 = halfHeight;
        sead::Vector3f farUp = up * halfHeight;
        sead::Vector3f nearSide = farSide * ratio;
        sead::Vector3f nearUp = farUp * ratio;
        copyVec3(&rLight.mDebugPos, rLight.mPos);
        rLight.mBoundBox.set(nearCenter, nearCenter);
        rLight.mBoundBox.addPoint(nearCenter + nearSide + nearUp);
        rLight.mBoundBox.addPoint(nearCenter - nearSide + nearUp);
        rLight.mBoundBox.addPoint(nearCenter + nearSide - nearUp);
        rLight.mBoundBox.addPoint(nearCenter - nearSide - nearUp);
        sead::Vector3f farDir = rDir * (far - near);
        rLight.mBoundBox.addPoint(farDir + (nearCenter + farSide + farUp));
        rLight.mBoundBox.addPoint(farDir + (nearCenter - farSide + farUp));
        rLight.mBoundBox.addPoint(farDir + (nearCenter + farSide - farUp));
        rLight.mBoundBox.addPoint(farDir + (nearCenter - farSide - farUp));
    } else {
        f32 halfHeight = (rLight.mParam[4] - rLight.mParam[5]) * 0.5f;
        f32 centerY = rLight.mParam[5] + halfHeight;
        f32 halfWidth = (rLight.mParam[7] - rLight.mParam[6]) * 0.5f;
        rLight.mParam[3] = halfWidth / halfHeight;
        f32 centerX = rLight.mParam[6] + halfWidth;
        f32 near = rLight.mParam[0];
        f32 far = rLight.mParam[1];
        rLight.mTanHalfFovy = halfHeight / far;
        const sead::Vector3f& rDir = rLight.mNormDir;
        rLight.mDebugPos = rLight.mPos + side * centerX + up * centerY;
        rLight.mDebugParam0 = halfWidth;
        rLight.mDebugParam1 = halfHeight;
        sead::Vector3f corner = rLight.mDebugPos + rDir * near - side * halfWidth - up * halfHeight;
        sead::Vector3f farDir = rDir * (far - near);
        sead::Vector3f height = up * (halfHeight + halfHeight);
        sead::Vector3f width = side * (halfWidth + halfWidth);
        rLight.mBoundBox.set(corner, corner);
        rLight.mBoundBox.addPoint(farDir + corner);
        rLight.mBoundBox.addPoint(height + corner);
        rLight.mBoundBox.addPoint(farDir + (height + corner));
        sead::Vector3f widthCorner = width + corner;
        rLight.mBoundBox.addPoint(widthCorner);
        rLight.mBoundBox.addPoint(farDir + widthCorner);
        rLight.mBoundBox.addPoint(height + widthCorner);
        rLight.mBoundBox.addPoint(farDir + (height + widthCorner));
    }
}

/**
 * Requests an orthographic projection light for the current frame.
 * @param rPos position
 * @param rDir direction
 * @param rUp up vector
 * @param rColor color
 * @param near near clip distance
 * @param far far clip distance
 * @param top top of the projection volume
 * @param bottom bottom of the projection volume
 * @param left left of the projection volume
 * @param right right of the projection volume
 * @param attnPow attenuation power
 * @param isEnableSpecular whether the light has specular
 * @param isUseSpecularColor whether rSpecularColor is used instead of rColor for specular
 * @param rSpecularColor specular color
 * @param pTexture projected texture, nullptr for none
 * @param isTextureWrap whether the projected texture repeats instead of being clamped
 * @param rTexScale texture scale
 * @param rTexOffset texture offset
 * @return index of the light, -1 if it was not added
 */
s32 PrePassLightKeeper::requestProjLightOrtho(
    const sead::Vector3f& rPos, const sead::Vector3f& rDir, const sead::Vector3f& rUp,
    const sead::Color4f& rColor, f32 near, f32 far, f32 top, f32 bottom, f32 left, f32 right,
    f32 attnPow, bool isEnableSpecular, bool isUseSpecularColor,
    const sead::Color4f& rSpecularColor, agl::TextureSampler* pTexture, bool isTextureWrap,
    const sead::Vector2f& rTexScale, const sead::Vector2f& rTexOffset) {
    if (mProjLightMgr->getLightMax() <= mProjLightRequestNum ||
        (rColor.r == 0.0f && rColor.g == 0.0f && rColor.b == 0.0f)) {
        return -1;
    }

    s32 index = mProjLightRequestNum;

    ProjLightData& rLight = mProjLightMgr->getLight(index);
    copyVec3(&rLight.mPos, rPos);
    copyVec3(&rLight.mDir, rDir);
    copyVec3(&rLight.mUp, rUp);
    rLight.mColor = rColor;
    rLight.mSpecColor = isUseSpecularColor ? rSpecularColor : rColor;
    rLight.mParam[0] = near;
    rLight.mParam[1] = far;
    rLight.mParam[4] = top;
    rLight.mParam[5] = bottom;
    rLight.mParam[6] = left;
    rLight.mParam[7] = right;
    rLight.mAttnPow = attnPow;
    rLight.mTexScale = rTexScale;
    rLight.mTexOffset = rTexOffset;
    rLight.mFlags.change(2, isEnableSpecular);
    rLight.mFlags.set(4);
    rLight.setVisibleAll(true);

    if (pTexture != nullptr) {
        rLight.mHasTexture = true;
        rLight.mTexture = *pTexture;

        if (isTextureWrap) {
            rLight.mFlags.set(0x10);
        } else {
            rLight.mFlags.reset(0x10);
            rLight.mTexture.setWrap(5, 5, 5);
            rLight.mTexture.setBorderColorAsColor(sead::Color4f(0.0f, 0.0f, 0.0f, 0.0f));
        }
    } else {
        rLight.mHasTexture = false;
    }

    rLight.setNormVec();
    rLight.mShadowType = agl::lght::LightPrePass::cShadowType_None;
    rLight.mShadowParam = 0.5f;
    rLight.clearShadowMap();
    mProjLightMgr->updateParameters_(rLight);
    mProjLightRequestNum++;
    mProjLightMgr->setValidNum(mProjLightRequestNum);
    return index;
}

/**
 * Requests a line light for the current frame.
 * @param rStart start of the line
 * @param rEnd end of the line
 * @param radius radius
 * @param rColor color
 * @param attnPow attenuation power
 * @param isEnableSpecular whether the light has specular
 */
void PrePassLightKeeper::requestLineLight(const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                                          f32 radius, const sead::Color4f& rColor, f32 attnPow,
                                          bool isEnableSpecular) {
    if (mLineLightMgr->getLightMax() <= mLineLightRequestNum) {
        return;
    }

    if (rColor.r == 0.0f && rColor.g == 0.0f && rColor.b == 0.0f) {
        return;
    }

    LineLightData& rLight = mLineLightMgr->getLight(mLineLightRequestNum);
    copyVec3(&rLight.mStart, rStart);
    copyVec3(&rLight.mEnd, rEnd);
    bool isInvalidRadius = radius <= 0.0f;
    rLight.mRadius = isInvalidRadius ? 1.0f : radius;
    rLight.mAttnPow = attnPow;
    rLight.mColor = isInvalidRadius ? sead::Color4f::cBlack : rColor;
    rLight.mFlags.change(2, isEnableSpecular);
    mLineLightRequestNum++;
    mLineLightMgr->setValidNum(mLineLightRequestNum);
}

/**
 * Draws the light pre-pass of a view into its light buffer.
 * @param view view index
 * @param pGBuffer G-buffer of the view
 * @param rDepthTarget depth target
 * @param shaderMode current shader mode
 * @return shader mode after drawing
 */
agl::ShaderMode PrePassLightKeeper::drawLpp(s32 view, const GBufferArray* pGBuffer,
                                            const agl::RenderTargetDepth& rDepthTarget,
                                            agl::ShaderMode shaderMode) const {
    agl::SamplerLocation albedoLocation(0, "cAlbedo");
    agl::SamplerLocation normalLocation(1, "cNormal");
    agl::SamplerLocation depthLocation(2, "cDepth");
    pGBuffer->activateSamplerAlbedo(albedoLocation);
    pGBuffer->activateSamplerNrmView(normalLocation);
    pGBuffer->activateSamplerDepthView(depthLocation);
    mLightPrePass->draw(GameFrameworkNx::getAglDrawContext(), view,
                        *pGBuffer->getGBufNrmViewTex(), rDepthTarget,
                        *pGBuffer->getGBufDepthViewTex());
    return shaderMode;
}

/**
 * Creates the point light manager.
 * @param pKeeper owning keeper
 */
AlbedoModePointLightMgr::AlbedoModePointLightMgr(PrePassLightKeeper* pKeeper) : mKeeper(pKeeper) {
    mShaderInfo = new LppShaderInfo("PointLightView");
}

/**
 * Does nothing.
 * @param pHeap heap (unused)
 */
void AlbedoModePointLightMgr::initPrepareImpl_(sead::Heap* pHeap) {}

/**
 * Resets a point light to its defaults.
 * @param rLight light
 * @param pHeap heap (unused)
 */
void AlbedoModePointLightMgr::initImpl_(PointLightData& rLight, sead::Heap* pHeap) {
    rLight.mFlags = 1;
    rLight.mPos.set(0.0f, 0.0f, 0.0f);
    rLight.mAttnPow = 1.0f;
    rLight.mRadius = 0.0f;
    rLight.mAttnStart = 0.0f;
    rLight.mColor = sead::Color4f::cBlack;
    rLight.mSpecColor = sead::Color4f::cBlack;

    for (auto& rView : rLight.mView) {
        rView.mShadowMap = nullptr;
        rView.mShadowSampler = nullptr;
    }
}

/**
 * Declares the members of a point light uniform block.
 * @param pUbo uniform block
 * @param pHeap heap to allocate from
 */
void AlbedoModePointLightMgr::initViewUboImpl_(agl::UniformBlock* pUbo, sead::Heap* pHeap) {
    declareUniformBlock(pUbo, cPointLightLayout, 6, pHeap);
}

/**
 * Checks whether a point light is inside the view frustum.
 * @param rLight light
 * @param view view index (unused)
 * @param rContext light pre-pass context of the view
 * @return true if the light is visible
 */
bool AlbedoModePointLightMgr::calcViewImpl_(PointLightData& rLight, s32 view,
                                            const LppContext& rContext) {
    return rContext.mCulling.isInside(rLight.mPos, rLight.mRadius);
}

/**
 * Writes the point light parameters of a view into the light's uniform block.
 * @param rLight light
 * @param view view index
 * @param rContext light pre-pass context of the view
 */
void AlbedoModePointLightMgr::updateUBO_(const PointLightData& rLight, s32 view,
                                         const LppContext& rContext) const {
    const sead::Vector3f& rPos = rLight.mPos;
    const sead::Matrix44f& rViewProjMtx = rContext.mViewProjMtx;
    f32 radius = rLight.mRadius;
    f32 attnStart = rLight.mAttnStart;
    sead::Vector3f viewPos;
    viewPos.setMul(rContext.mViewMtx, rPos);
    f32 x = rViewProjMtx(0, 0) * rPos.x + rViewProjMtx(0, 1) * rPos.y +
            rViewProjMtx(0, 2) * rPos.z + rViewProjMtx(0, 3);
    f32 y = rViewProjMtx(1, 0) * rPos.x + rViewProjMtx(1, 1) * rPos.y +
            rViewProjMtx(1, 2) * rPos.z + rViewProjMtx(1, 3);
    f32 w = rViewProjMtx(3, 0) * rPos.x + rViewProjMtx(3, 1) * rPos.y +
            rViewProjMtx(3, 2) * rPos.z + rViewProjMtx(3, 3);
    f32 screenX = -(rContext.mProjScale * (x / w)) - rContext.mScreenScaleX;
    f32 scale = (1.0f / (radius * radius)) * (w * w);
    f32 screenY =
        -(rContext.mProjSign * ((y / w) * (rContext.mProjSign * rContext.mCulling.mTanHalfFovy))) -
        rContext.mScreenScaleY;

    const agl::UniformBlock& rUbo = rLight.mView[view].mUbo;
    rUbo.dcbz(0);

    {
        sead::Vector4f data(viewPos.x, viewPos.y, viewPos.z, 1.0f / radius);
        rUbo.setData(0, &data, 0, 1);
    }

    {
        sead::Vector4f data(rLight.mColor.r, rLight.mColor.g, rLight.mColor.b, attnStart);
        rUbo.setData(1, &data, 0, 1);
    }

    {
        sead::Vector4f data(rLight.mSpecColor.r, rLight.mSpecColor.g, rLight.mSpecColor.b, 0.0f);
        rUbo.setData(2, &data, 0, 1);
    }

    {
        sead::Vector4f data(screenX, screenY, scale, 0.0f);
        rUbo.setData(3, &data, 0, 1);
    }

    f32 size = radius + radius;
    sead::Matrix44f modelMtx(size, 0.0f, 0.0f, rPos.x, 0.0f, size, 0.0f, rPos.y, 0.0f, 0.0f, size,
                             rPos.z, 0.0f, 0.0f, 0.0f, 1.0f);
    sead::Matrix44f mtx;
    agl::pfx::detail::multiplyMtx44(mtx, rViewProjMtx, modelMtx);
    rUbo.setData(4, &mtx, 0, 4);

    {
        sead::Vector4f data(mKeeper->getSpecularPower(), mKeeper->getFlesnel(), rLight.mAttnPow,
                            0.0f);
        rUbo.setData(5, &data, 0, 1);
    }
}

/**
 * Draws a point light into the light buffer.
 * @param pDrawContext draw context
 * @param rLight light
 * @param view view index
 * @param rContext light pre-pass context of the view (unused)
 * @param rArg callback argument holding the shared view uniform block
 */
void AlbedoModePointLightMgr::drawImpl_(agl::DrawContext* pDrawContext,
                                        const PointLightData& rLight, s32 view,
                                        const LppContext& rContext, LppCallbackArg& rArg) const {
    const char* macros[] = {"LPP_ENABLE_SPECULAR", "LPP_ENABLE_BACK"};
    const char* values[] = {"1", "0"};

    if (rLight.mFlags.isOff(2)) {
        values[0] = "0";
    }

    if (rLight.mFlags.isOn(4)) {
        values[1] = "1";
    }

    const agl::ShaderProgram* pProgram = mShaderInfo->mProgram->searchVariation(2, macros, values);
    pProgram->activate(pDrawContext, true);
    mKeeper->getSphereAttribute().activate(pDrawContext);
    agl::SamplerLocation specPowLocation(4, "cSpecPowTable");
    rArg.mSpecPowSampler->activate(pDrawContext, specPowLocation, -1, false);
    rArg.mViewUbo->activate(pDrawContext, mShaderInfo->mContextLocation);
    rLight.mView[view].mUbo.activate(pDrawContext, mShaderInfo->mViewLocation);
    mKeeper->mGraphicsSystemInfo->getLightEnvUbo()->activate(pDrawContext,
                                                             mShaderInfo->mLightEnvLocation);
    agl::pfx::detail::drawIndexStream(
        pDrawContext,
        agl::utl::PrimitiveShape::instance()->getSphereIndexStream(mLightPrePass->getQuality()));
}

/**
 * Draws the debug shapes of a point light.
 * @param pDrawContext draw context
 * @param rLight light
 * @param view view index (unused)
 * @param rContext light pre-pass context of the view
 */
void AlbedoModePointLightMgr::drawDebugImpl_(agl::DrawContext* pDrawContext,
                                             const PointLightData& rLight, s32 view,
                                             const LppContext& rContext) const {
    agl::utl::DevTools::drawPointLight(pDrawContext, rLight.mPos, rLight.mRadius, rLight.mColor,
                                       rContext.mCulling.mViewMtx, rContext.mCulling.mProjMtx);
    agl::utl::DevTools::drawPointLight(pDrawContext, rLight.mPos, rLight.mAttnStart, rLight.mColor,
                                       rContext.mCulling.mViewMtx, rContext.mCulling.mProjMtx);
    agl::utl::DevTools::beginDrawImm(pDrawContext, rContext.mCulling.mViewMtx,
                                     rContext.mCulling.mProjMtx);
    sead::Matrix34f mtx(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    mtx.setTranslation(rLight.mPos);
    agl::utl::DevTools::drawAxisImm(pDrawContext, mtx, rLight.mRadius, 1.0f, 1.0f);
}

/**
 * Computes the cosine of the cone angle.
 */
void SpotLightParam::calcAuxiliary() {
    mCosAngle = sead::Mathf::cos(mAngle);
}

/**
 * Creates the spot light manager.
 * @param pKeeper owning keeper
 */
AlbedoModeSpotLightMgr::AlbedoModeSpotLightMgr(PrePassLightKeeper* pKeeper) : mKeeper(pKeeper) {
    mShaderInfo = new LppShaderInfo("SpotLightView");
}

/**
 * Does nothing.
 * @param pHeap heap (unused)
 */
void AlbedoModeSpotLightMgr::initPrepareImpl_(sead::Heap* pHeap) {}

/**
 * Resets a spot light to its defaults and creates its shadow samplers.
 * @param rLight light
 * @param pHeap heap to allocate from
 */
void AlbedoModeSpotLightMgr::initImpl_(SpotLightData& rLight, sead::Heap* pHeap) {
    rLight.mFlags = 1;
    copyVec3(&rLight.mPos, sead::Vector3f::zero);
    copyVec3(&rLight.mDir, sead::Vector3f::ey);
    rLight.mLength = 0.0f;
    rLight.mAngle = 0.0f;
    rLight.mAngleAttnStart = 0.0f;
    rLight.mAttnPow = 1.0f;
    rLight.mAngleAttnPow = 1.0f;
    rLight.mColor = sead::Color4f::cBlack;
    rLight.mSpecColor = sead::Color4f::cBlack;
    rLight.mCosAngle = 1.0f;

    for (auto& rView : rLight.mView) {
        rView.mShadowMap = nullptr;
        rView.mShadowSampler = new (pHeap) agl::TextureSampler();
        rView.mShadowSampler->setWrap(5, 5, 5);
        rView.mShadowSampler->setBorderColorAsColor(sead::Color4f::cWhite);
        rView.mShadowSampler->setDepthCompareEnable(true);
        rView.mShadowSampler->setDepthCompareFunc(4);
    }
}

/**
 * Declares the members of a spot light uniform block.
 * @param pUbo uniform block
 * @param pHeap heap to allocate from
 */
void AlbedoModeSpotLightMgr::initViewUboImpl_(agl::UniformBlock* pUbo, sead::Heap* pHeap) {
    declareUniformBlock(pUbo, cSpotLightLayout, 9, pHeap);
}

/**
 * Deletes the shadow samplers of a spot light.
 * @param rLight light
 */
void AlbedoModeSpotLightMgr::destroyImpl_(SpotLightData& rLight) {
    for (auto& rView : rLight.mView) {
        delete rView.mShadowSampler;
    }
}

/**
 * Checks whether a spot light is inside the view frustum.
 * @param rLight light
 * @param view view index (unused)
 * @param rContext light pre-pass context of the view
 * @return true if the light is visible
 */
bool AlbedoModeSpotLightMgr::calcViewImpl_(SpotLightData& rLight, s32 view,
                                           const LppContext& rContext) {
    return rContext.mCulling.isInside(rLight.mPos, rLight.mLength);
}

/**
 * Writes the spot light parameters of a view into the light's uniform block.
 * @param rLight light
 * @param view view index
 * @param rContext light pre-pass context of the view
 */
void AlbedoModeSpotLightMgr::updateUBO_(const SpotLightData& rLight, s32 view,
                                        const LppContext& rContext) const {
    const sead::Matrix34f& rViewMtx = rContext.mViewMtx;
    f32 length = rLight.mLength;
    f32 cosAngle = rLight.mCosAngle;
    f32 angleAttnStart = rLight.mAngleAttnStart;
    sead::Vector3f viewPos;
    viewPos.setMul(rViewMtx, rLight.mPos);
    sead::Vector3f viewDir;
    viewDir.setRotated(rViewMtx, rLight.mDir);
    normalizeOrZero(&viewDir);

    const agl::UniformBlock& rUbo = rLight.mView[view].mUbo;
    rUbo.dcbz(0);

    {
        sead::Vector4f data(viewPos.x, viewPos.y, viewPos.z, 1.0f / length);
        rUbo.setData(0, &data, 0, 1);
    }

    {
        sead::Vector4f data(rLight.mColor.r, rLight.mColor.g, rLight.mColor.b, angleAttnStart);
        rUbo.setData(1, &data, 0, 1);
    }

    {
        sead::Vector4f data(rLight.mSpecColor.r, rLight.mSpecColor.g, rLight.mSpecColor.b, 0.0f);
        rUbo.setData(2, &data, 0, 1);
    }

    {
        sead::Vector4f data(viewDir.x, viewDir.y, viewDir.z,
                            rLight.mAttnPow > 0.0f ? rLight.mAttnPow : 0.0001f);
        rUbo.setData(3, &data, 0, 1);
    }

    {
        f32 angleScale = 1.0 / (1.0 - cosAngle);
        f32 angleOffset = cosAngle / (1.0 - cosAngle);
        sead::Vector4f data(angleScale, angleOffset, 1.0f / rLight.mLength,
                            rLight.mAngleAttnPow > 0.0f ? rLight.mAngleAttnPow : 0.0001f);
        rUbo.setData(4, &data, 0, 1);
    }

    f32 coneScale = (1.0f / sead::Mathf::cos(rLight.mAngle) - 1.0f) * 1.3f;
    f32 coneLength = rLight.mLength;
    f32 angle = rLight.mAngle;
    f32 coneRadius = coneLength * sead::Mathf::tan(angle);
    sead::Vector3f dir = rLight.mDir;
    dir.normalize();
    sead::Quatf quat;

    if (!quat.makeVectorRotation(-sead::Vector3f::ey, dir)) {
        quat.setAxisAngle(sead::Vector3f::ex, 180.0f);
    }

    sead::Matrix34f modelMtx;
    modelMtx.fromQuat(quat);
    f32 height = coneLength * sead::Mathf::cos(angle);
    f32 diameter = coneRadius + coneRadius;
    modelMtx.scaleBases(diameter, 1.0f, diameter);
    // the cone axis is scaled with the scale on the left-hand side, unlike scaleBases
    modelMtx.m[0][1] = height * modelMtx.m[0][1];
    modelMtx.m[1][1] = height * modelMtx.m[1][1];
    modelMtx.m[2][1] = height * modelMtx.m[2][1];
    modelMtx.setTranslation(rLight.mPos);
    const sead::Matrix34f offsetMtx(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -0.5f, 0.0f, 0.0f,
                                    1.0f, 0.0f);
    sead::Matrix34f coneMtx;
    coneMtx.setMul(modelMtx, offsetMtx);
    sead::Matrix44f coneViewProjMtx;
    coneViewProjMtx.setMul(rContext.mViewProjMtx, coneMtx);
    rUbo.setData(5, &coneViewProjMtx, 0, 4);
    sead::Vector4f specParam(mKeeper->getSpecularPower(), mKeeper->getFlesnel(), coneScale, 0.0f);
    rUbo.setData(6, &specParam, 0, 1);

    if (rLight.mView[view].mShadowMap == nullptr) {
        return;
    }

    sead::Matrix34f invViewMtx = rContext.mCulling.mViewMtx;
    invViewMtx.invert();
    sead::Matrix44f shadowMtx;
    shadowMtx.setMul(rLight.mView[view].mShadowMtx, invViewMtx);
    rUbo.setData(7, &shadowMtx, 0, 4);
    f32 shadowParam = rLight.mShadowParam;
    const agl::TextureData& rShadowTex = rLight.mView[view].mShadowMap->getTextureData();
    sead::Vector2f shadowTexelSize(shadowParam / rShadowTex.getWidth(0),
                                   shadowParam / rShadowTex.getHeight(0));
    rUbo.setData(8, &shadowTexelSize, 0, 1);
}

/**
 * Draws a spot light into the light buffer.
 * @param pDrawContext draw context
 * @param rLight light
 * @param view view index
 * @param rContext light pre-pass context of the view (unused)
 * @param rArg callback argument holding the shared view uniform block
 */
void AlbedoModeSpotLightMgr::drawImpl_(agl::DrawContext* pDrawContext, const SpotLightData& rLight,
                                       s32 view, const LppContext& rContext,
                                       LppCallbackArg& rArg) const {
    const char* macros[] = {"LPP_ENABLE_SPECULAR", "LPP_ENABLE_SHADOW"};
    const char* values[] = {"1", "0"};

    if (rLight.mFlags.isOff(2)) {
        values[0] = "0";
    }

    if (rLight.mView[view].mShadowMap != nullptr) {
        switch (rLight.mShadowType) {
        case agl::lght::LightPrePass::cShadowType_Normal:
            values[1] = "1";
            break;
        case 2:
            values[1] = "2";
            break;
        default:
            break;
        }
    }

    const agl::ShaderProgram* pProgram = mShaderInfo->mProgram->searchVariation(2, macros, values);
    pProgram->activate(pDrawContext, true);
    mKeeper->getConeAttribute().activate(pDrawContext);
    rArg.mViewUbo->activate(pDrawContext, mShaderInfo->mContextLocation);
    rLight.mView[view].mUbo.activate(pDrawContext, mShaderInfo->mViewLocation);
    mKeeper->mGraphicsSystemInfo->getLightEnvUbo()->activate(pDrawContext,
                                                             mShaderInfo->mLightEnvLocation);

    if (rLight.mView[view].mShadowMap != nullptr) {
        agl::SamplerLocation shadowLocation(5, "cDepthShadow");
        rLight.mView[view].mShadowSampler->applyTextureData(
            rLight.mView[view].mShadowMap->getTextureData());
        rLight.mView[view].mShadowSampler->activate(pDrawContext, shadowLocation, -1, false);
    }

    agl::pfx::detail::drawIndexStream(
        pDrawContext, agl::utl::PrimitiveShape::instance()->getConeTriangleIndexStream(
                          mLightPrePass->getQuality()));
}

/**
 * Draws the debug shape of a spot light.
 * @param pDrawContext draw context (unused)
 * @param rLight light
 * @param view view index (unused)
 * @param rContext light pre-pass context of the view
 */
void AlbedoModeSpotLightMgr::drawDebugImpl_(agl::DrawContext* pDrawContext,
                                            const SpotLightData& rLight, s32 view,
                                            const LppContext& rContext) const {
    agl::utl::DevTools::drawSpotLight(GameFrameworkNx::getAglDrawContext(), rLight.mPos,
                                      rLight.mDir, rLight.mColor, rLight.mAngle, rLight.mLength,
                                      rContext.mCulling.mViewMtx, rContext.mCulling.mProjMtx);
}

/**
 * Creates the line light manager.
 * @param pKeeper owning keeper
 */
AlbedoModeLineLightMgr::AlbedoModeLineLightMgr(PrePassLightKeeper* pKeeper) : mKeeper(pKeeper) {
    mShaderInfo = new LppShaderInfo("LineLightView");
}

/**
 * Does nothing.
 * @param pHeap heap (unused)
 */
void AlbedoModeLineLightMgr::initPrepareImpl_(sead::Heap* pHeap) {}

/**
 * Resets a line light to its defaults.
 * @param rLight light
 * @param pHeap heap (unused)
 */
void AlbedoModeLineLightMgr::initImpl_(LineLightData& rLight, sead::Heap* pHeap) {
    rLight.mFlags = 1;
    rLight.mStart.set(0.0f, 0.0f, 0.0f);
    rLight.mEnd.set(0.0f, 0.0f, 0.0f);
    rLight.mRadius = 0.0f;
    rLight.mAttnPow = 0.0f;
    rLight.mColor = sead::Color4f::cBlack;

    for (auto& rView : rLight.mView) {
        rView.mShadowMap = nullptr;
        rView.mShadowSampler = nullptr;
    }
}

/**
 * Declares the members of a line light uniform block.
 * @param pUbo uniform block
 * @param pHeap heap to allocate from
 */
void AlbedoModeLineLightMgr::initViewUboImpl_(agl::UniformBlock* pUbo, sead::Heap* pHeap) {
    declareUniformBlock(pUbo, cLineLightLayout, 5, pHeap);
}

/**
 * Line lights are never culled.
 * @param rLight light (unused)
 * @param view view index (unused)
 * @param rContext light pre-pass context of the view (unused)
 * @return always true
 */
bool AlbedoModeLineLightMgr::calcViewImpl_(LineLightData& rLight, s32 view,
                                           const LppContext& rContext) {
    return true;
}

/**
 * Writes the line light parameters of a view into the light's uniform block.
 * @param rLight light
 * @param view view index
 * @param rContext light pre-pass context of the view
 */
void AlbedoModeLineLightMgr::updateUBO_(const LineLightData& rLight, s32 view,
                                        const LppContext& rContext) const {
    const sead::Matrix34f& rViewMtx = rContext.mViewMtx;
    f32 squaredLength = (rLight.mStart - rLight.mEnd).squaredLength();
    sead::Vector3f viewStart;
    viewStart.setMul(rViewMtx, rLight.mStart);
    f32 radius = rLight.mRadius;
    f32 attnPow = rLight.mAttnPow;
    sead::Vector3f viewEnd;
    viewEnd.setMul(rViewMtx, rLight.mEnd);

    const agl::UniformBlock& rUbo = rLight.mView[view].mUbo;
    rUbo.dcbz(0);

    {
        sead::Vector4f data(viewStart.x, viewStart.y, viewStart.z, 1.0f / radius);
        rUbo.setData(0, &data, 0, 1);
    }

    {
        sead::Vector4f data(rLight.mColor.r, rLight.mColor.g, rLight.mColor.b, attnPow);
        rUbo.setData(1, &data, 0, 1);
    }

    {
        sead::Vector4f data(viewEnd.x, viewEnd.y, viewEnd.z, 1.0f / squaredLength);
        rUbo.setData(2, &data, 0, 1);
    }

    sead::Matrix34f modelMtx;
    f32 length;

    if (!calcLineLightMtx(&modelMtx, &length, rLight)) {
        return;
    }

    sead::Matrix44f mtx;
    mtx.setMul(rContext.mViewProjMtx, modelMtx);
    rUbo.setData(3, &mtx, 0, 4);

    {
        sead::Vector4f data(mKeeper->getSpecularPower(), mKeeper->getFlesnel(), 0.0f, 0.0f);
        rUbo.setData(4, &data, 0, 1);
    }
}

/**
 * Draws a line light into the light buffer.
 * @param pDrawContext draw context
 * @param rLight light
 * @param view view index
 * @param rContext light pre-pass context of the view (unused)
 * @param rArg callback argument holding the shared view uniform block
 */
void AlbedoModeLineLightMgr::drawImpl_(agl::DrawContext* pDrawContext, const LineLightData& rLight,
                                       s32 view, const LppContext& rContext,
                                       LppCallbackArg& rArg) const {
    const char* macros[] = {"LPP_ENABLE_SPECULAR"};
    const char* values[] = {"1"};

    if (rLight.mFlags.isOff(2)) {
        values[0] = "0";
    }

    const agl::ShaderProgram* pProgram = mShaderInfo->mProgram->searchVariation(1, macros, values);
    pProgram->activate(pDrawContext, true);
    mKeeper->getCylinderAttribute().activate(pDrawContext);
    rArg.mViewUbo->activate(pDrawContext, mShaderInfo->mContextLocation);
    rLight.mView[view].mUbo.activate(pDrawContext, mShaderInfo->mViewLocation);
    mKeeper->mGraphicsSystemInfo->getLightEnvUbo()->activate(pDrawContext,
                                                             mShaderInfo->mLightEnvLocation);
    agl::pfx::detail::drawIndexStream(
        pDrawContext, agl::utl::PrimitiveShape::instance()->getCylinderTriangleIndexStream(
                          mLightPrePass->getQuality()));
}

/**
 * Draws the debug shapes of a line light.
 * @param pDrawContext draw context (unused)
 * @param rLight light
 * @param view view index (unused)
 * @param rContext light pre-pass context of the view
 */
void AlbedoModeLineLightMgr::drawDebugImpl_(agl::DrawContext* pDrawContext,
                                            const LineLightData& rLight, s32 view,
                                            const LppContext& rContext) const {
    sead::Matrix34f modelMtx;
    f32 length;

    if (!calcLineLightMtx(&modelMtx, &length, rLight)) {
        return;
    }

    sead::Color4f color = rLight.mColor;
    color.a *= 0.5f;
    agl::utl::DevTools::drawPointLight(GameFrameworkNx::getAglDrawContext(), rLight.mStart,
                                       rLight.mRadius, color, rContext.mCulling.mViewMtx,
                                       rContext.mCulling.mProjMtx);
    agl::utl::DevTools::drawPointLight(GameFrameworkNx::getAglDrawContext(), rLight.mStart,
                                       rLight.mRadius, color, rContext.mCulling.mViewMtx,
                                       rContext.mCulling.mProjMtx);
    sead::Vector3f center = (rLight.mStart + rLight.mEnd) * 0.5f;
    agl::utl::DevTools::drawArrow(GameFrameworkNx::getAglDrawContext(), center, rLight.mStart,
                                  color, color, 0.3f, rContext.mCulling.mViewMtx,
                                  rContext.mCulling.mProjMtx);
    agl::utl::DevTools::drawArrow(GameFrameworkNx::getAglDrawContext(), center, rLight.mEnd, color,
                                  color, 0.3f, rContext.mCulling.mViewMtx,
                                  rContext.mCulling.mProjMtx);
    agl::utl::DevTools::beginDrawImm(GameFrameworkNx::getAglDrawContext(),
                                     rContext.mCulling.mViewMtx, rContext.mCulling.mProjMtx);
    agl::utl::DevTools::drawAxisImm(GameFrameworkNx::getAglDrawContext(), modelMtx, 1.0f, 1.0f,
                                    1.0f);
}

/**
 * Creates the projection light manager.
 * @param pKeeper owning keeper
 */
AlbedoModeProjLightMgr::AlbedoModeProjLightMgr(PrePassLightKeeper* pKeeper) : mKeeper(pKeeper) {
    mShaderInfo = new LppShaderInfo("ProjLightView");
}

/**
 * Does nothing.
 * @param pHeap heap (unused)
 */
void AlbedoModeProjLightMgr::initPrepareImpl_(sead::Heap* pHeap) {}

/**
 * Resets a projection light to its defaults and creates its shadow samplers.
 * @param rLight light
 * @param pHeap heap to allocate from
 */
void AlbedoModeProjLightMgr::initImpl_(ProjLightData& rLight, sead::Heap* pHeap) {
    rLight.mFlags = 0xb;
    rLight.mPos = sead::Vector3f::zero;
    rLight.mDir = -sead::Vector3f::ey;
    rLight.mUp = sead::Vector3f::ey;
    rLight.mColor = sead::Color4f::cWhite;
    rLight.mSpecColor = sead::Color4f::cWhite;
    rLight.mParam[2] = 0.7853982f;
    rLight.mParam[3] = 0.7f;
    rLight.mParam[0] = 0.4f;
    rLight.mParam[1] = 2.0f;
    rLight.mParam[4] = 0.5f;
    rLight.mParam[5] = -0.5f;
    rLight.mParam[6] = -0.8f;
    rLight.mParam[7] = 0.8f;
    rLight.mAttnPow = 1.0f;
    rLight.mTexScale = sead::Vector2f::zero;
    rLight.mHasTexture = false;
    rLight.mTexOffset.set(1.0f, 1.0f);

    for (auto& rView : rLight.mView) {
        rView.mShadowMap = nullptr;
        rView.mShadowSampler = new (pHeap) agl::TextureSampler();
        rView.mShadowSampler->setWrap(5, 5, 5);
        rView.mShadowSampler->setBorderColorAsColor(sead::Color4f::cWhite);
        rView.mShadowSampler->setDepthCompareEnable(true);
        rView.mShadowSampler->setDepthCompareFunc(4);
    }
}

/**
 * Declares the members of a projection light uniform block.
 * @param pUbo uniform block
 * @param pHeap heap to allocate from
 */
void AlbedoModeProjLightMgr::initViewUboImpl_(agl::UniformBlock* pUbo, sead::Heap* pHeap) {
    declareUniformBlock(pUbo, cProjLightLayout, 4, pHeap);
}

/**
 * Deletes the shadow samplers of a projection light.
 * @param rLight light
 */
void AlbedoModeProjLightMgr::destroyImpl_(ProjLightData& rLight) {
    for (auto& rView : rLight.mView) {
        delete rView.mShadowSampler;
    }
}

/**
 * Checks whether a projection light is inside the view frustum.
 * @param rLight light
 * @param view view index (unused)
 * @param rContext light pre-pass context of the view
 * @return true if the light is visible
 */
bool AlbedoModeProjLightMgr::calcViewImpl_(ProjLightData& rLight, s32 view,
                                           const LppContext& rContext) {
    return rContext.mCulling.isInside(rLight.mBoundBox.getMin(), rLight.mBoundBox.getMax());
}

/**
 * Writes the projection light parameters of a view into the light's uniform block.
 * @param rLight light
 * @param view view index
 * @param rContext light pre-pass context of the view
 */
void AlbedoModeProjLightMgr::updateUBO_(const ProjLightData& rLight, s32 view,
                                        const LppContext& rContext) const {
    const agl::UniformBlock& rUbo = rLight.mView[view].mUbo;
    rUbo.dcbz(0);

    f32 near = rLight.mParam[0];
    f32 far = rLight.mParam[1];
    f32 nearFarRatio = near / far;
    f32 invRange = 1.0f / (far - near);

    {
        sead::Vector4f data(rLight.mColor.r, rLight.mColor.g, rLight.mColor.b,
                            rLight.mAttnPow <= 0.0f ? 0.001f : rLight.mAttnPow);
        rUbo.setData(3, &data, 0, 1);
    }

    {
        sead::Vector4f data(invRange * rLight.mParam[0] + 1.0f, nearFarRatio, invRange, 0.0f);
        rUbo.setData(3, &data, 4, 1);
    }

    if (rLight.mFlags.isOn(2)) {
        sead::Vector3f specColor(rLight.mSpecColor.r, rLight.mSpecColor.g, rLight.mSpecColor.b);

        if (!mLightPrePass->getFlags().isOn(1 << 8) && !rLight.mHasTexture) {
            specColor.x = rLight.mSpecColor.a *
                          (rLight.mSpecColor.r * 0.298912f + rLight.mSpecColor.g * 0.586611f +
                           rLight.mSpecColor.b * 0.114478f);
        }

        sead::Vector4f data(specColor.x, specColor.y, specColor.z, 0.0f);
        rUbo.setData(3, &data, 1, 1);
    }

    const sead::Matrix34f& rViewMtx = rContext.mCulling.mViewMtx;
    sead::Vector3f viewPos;
    viewPos.setMul(rViewMtx, rLight.mDebugPos);
    sead::Vector3f viewDir;
    viewDir.setRotated(rViewMtx, rLight.mNormDir);
    viewDir.normalize();

    if (rLight.mFlags.isOff(4)) {
        sead::Vector4f data(viewPos.x, viewPos.y, viewPos.z, 0.0f);
        rUbo.setData(3, &data, 2, 1);
    } else {
        sead::Vector4f data(viewDir.x, viewDir.y, viewDir.z, 0.0f);
        rUbo.setData(3, &data, 3, 1);
    }

    sead::Matrix34f lightMtx;

    {
        sead::LookAtCamera camera(rLight.mDebugPos, rLight.mDebugPos + rLight.mNormDir,
                                  rLight.mNormUp);
        camera.updateViewMatrix();
        sead::Matrix34f invCameraMtx = camera.getMatrix();
        invCameraMtx.invert();
        const sead::Matrix34f identityMtx(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                          0.0f, 1.0f, 0.0f);
        lightMtx.setMul(identityMtx, invCameraMtx);
    }

    {
        f32 frustumFar = rLight.mParam[1];
        const sead::Matrix34f farMtx(frustumFar, 0.0f, 0.0f, 0.0f, 0.0f, frustumFar, 0.0f, 0.0f,
                                     0.0f, 0.0f, frustumFar, 0.0f);
        sead::Matrix34f farScaledMtx;
        farScaledMtx.setMul(lightMtx, farMtx);
        f32 frustumHeight = rLight.mTanHalfFovy + rLight.mTanHalfFovy;
        const sead::Matrix34f sizeMtx(frustumHeight * rLight.mParam[3], 0.0f, 0.0f, 0.0f, 0.0f,
                                      frustumHeight, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        sead::Matrix34f sizeScaledMtx;
        sizeScaledMtx.setMul(farScaledMtx, sizeMtx);
        const sead::Matrix34f offsetMtx(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, -0.5f);
        sead::Matrix34f modelMtx;
        modelMtx.setMul(sizeScaledMtx, offsetMtx);
        sead::Matrix34f modelViewMtx;
        modelViewMtx.setMul(rViewMtx, modelMtx);
        sead::Matrix44f mtx;
        mtx.setMul(rContext.mCulling.mProjMtx, modelViewMtx);
        rUbo.setData(0, &mtx, 0, 4);
    }

    sead::Matrix34f lightViewMtx;

    {
        sead::Vector3f viewUp;
        viewUp.setRotated(rViewMtx, rLight.mNormUp);
        sead::LookAtCamera camera(viewPos, viewPos + viewDir, viewUp);
        camera.updateViewMatrix();
        lightViewMtx = camera.getMatrix();
    }

    {
        sead::Matrix44f lightViewProjMtx;

        if (rLight.mFlags.isOff(4)) {
            sead::PerspectiveProjection projection(rLight.mParam[0], rLight.mParam[1],
                                                   rLight.mParam[2], rLight.mParam[3]);
            lightViewProjMtx.setMul(projection.getProjectionMatrix(), lightViewMtx);
        } else {
            sead::OrthoProjection projection(rLight.mParam[0], rLight.mParam[1],
                                             rLight.mDebugParam1, -rLight.mDebugParam1,
                                             rLight.mDebugParam0, -rLight.mDebugParam0);
            lightViewProjMtx.setMul(projection.getProjectionMatrix(), lightViewMtx);
        }

        const sead::Matrix44f biasMtx(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, -0.5f, 0.0f, 0.5f, 0.0f, 0.0f,
                                      1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        agl::pfx::detail::multiplyMtx44(lightViewProjMtx, biasMtx, lightViewProjMtx);
        rUbo.setData(1, &lightViewProjMtx, 0, 4);
    }

    if (rLight.mFlags.isOn(0x10)) {
        f32 invOffsetX = rLight.mTexOffset.x != 0.0f ? 1.0f / rLight.mTexOffset.x : 10000.0f;
        f32 invOffsetY = rLight.mTexOffset.y != 0.0f ? 1.0f / rLight.mTexOffset.y : 10000.0f;
        sead::Vector4f data(-rLight.mTexScale.x, -rLight.mTexScale.y, invOffsetX, invOffsetY);
        rUbo.setData(3, &data, 6, 1);
    }

    if (rLight.mView[view].mShadowMap == nullptr) {
        return;
    }

    sead::Matrix34f invViewMtx = rContext.mCulling.mViewMtx;
    invViewMtx.invert();
    sead::Matrix44f shadowMtx;
    shadowMtx.setMul(rLight.mView[view].mShadowMtx, invViewMtx);
    rUbo.setData(2, &shadowMtx, 0, 4);
    f32 shadowParam = rLight.mShadowParam;
    const agl::TextureData& rShadowTex = rLight.mView[view].mShadowMap->getTextureData();
    sead::Vector4f shadowTexelSize(shadowParam / rShadowTex.getWidth(0),
                                   shadowParam / rShadowTex.getHeight(0), 0.0f, 0.0f);
    rUbo.setData(3, &shadowTexelSize, 5, 1);
}

/**
 * Draws a projection light into the light buffer.
 * @param pDrawContext draw context
 * @param rLight light
 * @param view view index
 * @param rContext light pre-pass context of the view (unused)
 * @param rArg callback argument holding the shared view uniform block
 */
void AlbedoModeProjLightMgr::drawImpl_(agl::DrawContext* pDrawContext, const ProjLightData& rLight,
                                       s32 view, const LppContext& rContext,
                                       LppCallbackArg& rArg) const {
    const char* macros[] = {"LPP_LIGHT_TYPE", "LPP_ENABLE_SHADOW", "LPP_ENABLE_TEX"};
    const char* values[] = {"0", "0", "0"};

    if (rLight.mFlags.isOn(4)) {
        values[0] = "1";
    }

    if (rLight.mView[view].mShadowMap != nullptr) {
        switch (rLight.mShadowType) {
        case agl::lght::LightPrePass::cShadowType_Normal:
            values[1] = "1";
            break;
        case 2:
            values[1] = "2";
            break;
        default:
            break;
        }
    }

    if (rLight.mHasTexture) {
        values[2] = rLight.mFlags.isOn(0x10) ? "2" : "1";
    }

    const agl::ShaderProgram* pProgram = mShaderInfo->mProgram->searchVariation(3, macros, values);
    pProgram->activate(pDrawContext, true);
    mKeeper->getCubeAttribute().activate(pDrawContext);
    rArg.mViewUbo->activate(pDrawContext, mShaderInfo->mContextLocation);
    rLight.mView[view].mUbo.activate(pDrawContext, mShaderInfo->mViewLocation);
    mKeeper->mGraphicsSystemInfo->getLightEnvUbo()->activate(pDrawContext,
                                                             mShaderInfo->mLightEnvLocation);
    agl::SamplerLocation texLocation(3, "cProjTex");
    agl::SamplerLocation shadowLocation(5, "cDepthShadow");

    if (rLight.mHasTexture) {
        rLight.mTexture.activate(pDrawContext, texLocation, -1, false);
    }

    if (rLight.mView[view].mShadowMap != nullptr) {
        rLight.mView[view].mShadowSampler->applyTextureData(
            rLight.mView[view].mShadowMap->getTextureData());
        rLight.mView[view].mShadowSampler->activate(pDrawContext, shadowLocation, -1, false);
    }

    agl::pfx::detail::drawIndexStream(pDrawContext,
                                      agl::utl::PrimitiveShape::instance()->getCubeIndexStream());
}

/**
 * Draws the debug frustum of a projection light.
 * @param pDrawContext draw context (unused)
 * @param rLight light
 * @param view view index (unused)
 * @param rContext light pre-pass context of the view
 */
void AlbedoModeProjLightMgr::drawDebugImpl_(agl::DrawContext* pDrawContext,
                                            const ProjLightData& rLight, s32 view,
                                            const LppContext& rContext) const {
    agl::utl::DevTools::drawProjLight(
        GameFrameworkNx::getAglDrawContext(), rLight.mDebugPos, rLight.mNormDir, rLight.mNormUp,
        rLight.mColor, rLight.mParam[2], rLight.mParam[3], rLight.mParam[0], rLight.mParam[1],
        rLight.mDebugParam1, rLight.mDebugParam0, rLight.mPos, rLight.mFlags.isOn(4),
        rContext.mCulling.mViewMtx, rContext.mCulling.mProjMtx);
}

}  // namespace al

namespace LightPrePassFunction {

/**
 * Declares how many point lights an actor will request.
 * @param pActor actor
 * @param num number of point lights
 */
void declareUsingPointLight(const al::LiveActor* pActor, s32 num) {
    pActor->getSceneInfo()->graphicsSystemInfo->getPrePassLightKeeper()->mPointLightNum += num;
}

/**
 * Declares how many spot lights an actor will request.
 * @param pActor actor
 * @param num number of spot lights
 */
void declareUsingSpotLight(const al::LiveActor* pActor, s32 num) {
    pActor->getSceneInfo()->graphicsSystemInfo->getPrePassLightKeeper()->mSpotLightNum += num;
}

/**
 * Requests a point light for the current frame.
 * @param pActor actor
 * @param rPos position
 * @param radius radius
 * @param rColor color
 * @param attnPow attenuation power
 * @param attnStart attenuation start
 * @param isEnableSpecular whether the light has specular
 * @param isUseSpecularColor whether rSpecularColor is used instead of rColor for specular
 * @param rSpecularColor specular color
 * @param lightShaderFunc light shader function
 */
void requestPointLight(const al::LiveActor* pActor, const sead::Vector3f& rPos, f32 radius,
                       const sead::Color4f& rColor, f32 attnPow, f32 attnStart,
                       bool isEnableSpecular, bool isUseSpecularColor,
                       const sead::Color4f& rSpecularColor, s32 lightShaderFunc) {
    pActor->getSceneInfo()->graphicsSystemInfo->getPrePassLightKeeper()->requestPointLight(
        rPos, radius, rColor, attnPow, attnStart, isEnableSpecular, isUseSpecularColor,
        rSpecularColor, lightShaderFunc);
}

/**
 * Requests a spot light for the current frame.
 * @param pActor actor
 * @param rPos position
 * @param rDir direction
 * @param angle cone angle
 * @param length cone length
 * @param rColor color
 * @param attnPow attenuation power
 * @param angleAttnPow angle attenuation power
 * @param angleAttnStart angle attenuation start
 * @param isEnableSpecular whether the light has specular
 * @param isUseSpecularColor whether rSpecularColor is used instead of rColor for specular
 * @param rSpecularColor specular color
 */
void requestSpotLight(const al::LiveActor* pActor, const sead::Vector3f& rPos,
                      const sead::Vector3f& rDir, f32 angle, f32 length,
                      const sead::Color4f& rColor, f32 attnPow, f32 angleAttnPow,
                      f32 angleAttnStart, bool isEnableSpecular, bool isUseSpecularColor,
                      const sead::Color4f& rSpecularColor) {
    pActor->getSceneInfo()->graphicsSystemInfo->getPrePassLightKeeper()->requestSpotLight(
        rPos, rDir, angle, length, rColor, attnPow, angleAttnPow, angleAttnStart,
        isEnableSpecular, isUseSpecularColor, rSpecularColor);
}

}  // namespace LightPrePassFunction
