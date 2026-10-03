#include "Library/Model/SimpleModelEnv.hpp"

#include <arm_neon.h>
#include <attributes.h>
#include <math/seadMathCalcCommon.h>
#include "common/aglUniformBlock.h"
#include "utility/aglResParameter.h"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Fog/FogDirector.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Light/DirectionalLightKeeper.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/ShaderEnvTextureKeeper.hpp"
#include "Library/Shadow/ShadowDirector.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace {

/**
 * @brief Layout of the model environment uniform block (view, lights, fog and screen values).
 */
const al::UniformBlockLayout cMdlEnvViewUboLayout[] = {
    {0, agl::UniformBlock::cType_Vec4, 3},   {1, agl::UniformBlock::cType_Vec4, 3},
    {2, agl::UniformBlock::cType_Vec4, 4},   {3, agl::UniformBlock::cType_Vec4, 3},
    {4, agl::UniformBlock::cType_Vec4, 1},   {5, agl::UniformBlock::cType_Vec4, 1},
    {6, agl::UniformBlock::cType_Float, 1},  {7, agl::UniformBlock::cType_Float, 1},
    {8, agl::UniformBlock::cType_Vec3, 1},   {9, agl::UniformBlock::cType_Vec3, 1},
    {10, agl::UniformBlock::cType_Vec4, 1},  {11, agl::UniformBlock::cType_Vec3, 1},
    {12, agl::UniformBlock::cType_Vec3, 1},  {13, agl::UniformBlock::cType_Float, 1},
    {14, agl::UniformBlock::cType_Float, 1}, {15, agl::UniformBlock::cType_Float, 1},
    {16, agl::UniformBlock::cType_Float, 1}, {17, agl::UniformBlock::cType_Vec2, 1},
    {18, agl::UniformBlock::cType_Vec2, 1},  {19, agl::UniformBlock::cType_Vec2, 1},
    {20, agl::UniformBlock::cType_Vec4, 1},  {21, agl::UniformBlock::cType_Vec4, 1},
    {22, agl::UniformBlock::cType_Float, 1}, {23, agl::UniformBlock::cType_Float, 1},
    {24, agl::UniformBlock::cType_Vec4, 1},  {25, agl::UniformBlock::cType_Vec4, 1},
    {26, agl::UniformBlock::cType_Float, 1}, {27, agl::UniformBlock::cType_Float, 1},
    {28, agl::UniformBlock::cType_Vec3, 1},  {29, agl::UniformBlock::cType_Vec4, 4},
    {30, agl::UniformBlock::cType_Vec2, 1},  {31, agl::UniformBlock::cType_Vec2, 1},
};

/**
 * @brief Multiplies a 3x4 matrix (with an implicit last row of 0, 0, 0, 1) by a 4x4 matrix,
 * keeping the first three rows.
 * @param pOut Output matrix.
 * @param rA Left 3x4 matrix.
 * @param rB Right 4x4 matrix.
 */
void multiplyMtx34Mtx44(sead::Matrix34f* pOut, const sead::Matrix34f& rA,
                        const sead::Matrix44f& rB) {
    float32x4_t a0 = vld1q_f32(rA.m[0]);
    float32x4_t a1 = vld1q_f32(rA.m[1]);
    float32x4_t a2 = vld1q_f32(rA.m[2]);

    float32x4_t b0 = vld1q_f32(rB.m[0]);
    float32x4_t b1 = vld1q_f32(rB.m[1]);
    float32x4_t b2 = vld1q_f32(rB.m[2]);
    float32x4_t b3 = vld1q_f32(rB.m[3]);

    float32x4_t c0 = vmulq_laneq_f32(b0, a0, 0);
    c0 = vfmaq_laneq_f32(c0, b1, a0, 1);
    c0 = vfmaq_laneq_f32(c0, b2, a0, 2);
    c0 = vfmaq_laneq_f32(c0, b3, a0, 3);

    float32x4_t c1 = vmulq_laneq_f32(b0, a1, 0);
    c1 = vfmaq_laneq_f32(c1, b1, a1, 1);
    c1 = vfmaq_laneq_f32(c1, b2, a1, 2);
    c1 = vfmaq_laneq_f32(c1, b3, a1, 3);

    float32x4_t c2 = vmulq_laneq_f32(b0, a2, 0);
    c2 = vfmaq_laneq_f32(c2, b1, a2, 1);
    c2 = vfmaq_laneq_f32(c2, b2, a2, 2);
    c2 = vfmaq_laneq_f32(c2, b3, a2, 3);

    vst1q_f32(pOut->m[0], c0);
    vst1q_f32(pOut->m[1], c1);
    vst1q_f32(pOut->m[2], c2);
}

/**
 * @brief Rotates a vector by the rotation part of a matrix.
 * @param pVec Vector to rotate.
 * @param rMtx Matrix.
 */
void rotateVector(sead::Vector3f* pVec, const sead::Matrix34f& rMtx) {
    sead::Vector3f vec = *pVec;
    pVec->x = vec.x * rMtx.m[0][0] + vec.y * rMtx.m[0][1] + vec.z * rMtx.m[0][2];
    pVec->y = vec.x * rMtx.m[1][0] + vec.y * rMtx.m[1][1] + vec.z * rMtx.m[1][2];
    pVec->z = vec.x * rMtx.m[2][0] + vec.y * rMtx.m[2][1] + vec.z * rMtx.m[2][2];
}

/**
 * @brief Sets a fog start and the inverse of the fog range to a uniform block.
 * @param pBlock Uniform block.
 * @param index Member index of the start; the inverse range is set to the next member.
 * @param start Fog start.
 * @param end Fog end.
 */
void setFogRange(const al::UniformBlock* pBlock, s32 index, f32 start, f32 end) {
    pBlock->setValue(index, start);

    if (end == start) {
        pBlock->setValue(index + 1, 1.0f / ((end + 1.0f) - start));
    } else {
        pBlock->setValue(index + 1, 1.0f / (end - start));
    }
}

}  // namespace

namespace al {

/**
 * Constructs an empty model environment.
 */
SimpleModelEnv::SimpleModelEnv() = default;

/**
 * Deletes the uniform blocks.
 */
SimpleModelEnv::~SimpleModelEnv() {
    while (!mUniformBlocks.isEmpty()) {
        delete mUniformBlocks.popBack();
    }

    mUniformBlocks.freeBuffer();
}

/**
 * Creates one uniform block per buffer.
 * @param bufferNum Number of buffers (views).
 * @param pInfo Graphics system info.
 * @param pHeap Heap to allocate the block array from.
 */
void SimpleModelEnv::initialize(s32 bufferNum, const GraphicsSystemInfo* pInfo,
                                sead::Heap* pHeap) {
    mUniformBlocks.allocBuffer(bufferNum, pHeap);

    for (s32 i = 0; i < bufferNum; i++) {
        mUniformBlocks.pushBack(createUniformBlock(cMdlEnvViewUboLayout, 32, nullptr, 2));
    }

    mGraphicsSystemInfo = pInfo;
}

/**
 * Updates the uniform block of a view with the camera, light and fog environment.
 * @param index View index.
 * @param rViewMtx View matrix.
 * @param rProjMtx Projection matrix.
 * @param rProjOffset Projection center offset.
 * @param rScreenSize Screen size.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param fovy Vertical field of view.
 * @param aspect Aspect ratio.
 * @param orthoSize Size of the area covered by the environment texture.
 * @param pSkyLightInfo Sky light applied to white, or nullptr.
 * @param pSky2LightInfo Second sky light applied to white, or nullptr.
 * @param pDirLightInfo Light applied to the directional light color, or nullptr.
 * @param rate Light rate.
 * @param pDirLightName Name of the directional light, or nullptr for the current one.
 */
void SimpleModelEnv::updateEnv(s32 index, const sead::Matrix34f& rViewMtx,
                               const sead::Matrix44f& rProjMtx, const sead::Vector2f& rProjOffset,
                               const sead::Vector2f& rScreenSize, f32 near, f32 far, f32 fovy,
                               f32 aspect, sead::Vector2f orthoSize,
                               const LightInfo* pSkyLightInfo, const LightInfo* pSky2LightInfo,
                               const LightInfo* pDirLightInfo, f32 rate,
                               const char* pDirLightName) {
    sead::Matrix34f invViewMtx;
    invViewMtx.setInverse(rViewMtx);

    UniformBlock* block = mUniformBlocks.at(index);
    UniformBlockSetter setter(block, 0);

    sead::Matrix44f viewProjMtx;
    viewProjMtx.setMul(rProjMtx, rViewMtx);
    block->setData(0, &rViewMtx, 0, 3);
    block->setData(1, &invViewMtx, 0, 3);
    block->setData(2, &viewProjMtx, 0, 4);

    {
        sead::Matrix44f invProjMtx;
        invProjMtx.setInverse(rProjMtx);
        sead::Matrix34f invViewProjMtx;
        multiplyMtx34Mtx44(&invViewProjMtx, invViewMtx, invProjMtx);
        block->setData(3, &invViewProjMtx, 0, 3);
    }

    block->setValue(13, near);
    block->setValue(14, far);
    f32 range = far - near;
    block->setValue(15, range);
    block->setValue(16, 1.0f / range);

    f32 tanY = sead::Mathf::tan(fovy * 0.5f);
    f32 tanX = tanY * aspect;
    sead::Vector2f projOffset(tanX * (rProjOffset.x * 2.0f), tanY * (rProjOffset.y * 2.0f));
    block->setData(18, &projOffset, 0, 1);

    {
        sead::Vector2f invScreenSize(1.0f / rScreenSize.x, 1.0f / rScreenSize.y);
        block->setData(19, &invScreenSize, 0, 1);
    }

    {
        sead::Vector2f tanFov(tanX, -tanY);
        block->setData(17, &tanFov, 0, 1);
    }

    sead::Vector3f up = sead::Vector3f::ey;
    rotateVector(&up, rViewMtx);

    const DirectionalLightKeeper* dirLightKeeper =
        mGraphicsSystemInfo->getDirectionalLightKeeper();
    sead::Color4f skyColor = sead::Color4f::cWhite;
    sead::Color4f sky2Color = sead::Color4f::cWhite;

    if (pSkyLightInfo != nullptr) {
        pSkyLightInfo->calcApplyColor(&skyColor, sead::Color4f::cWhite);
    }

    if (pSky2LightInfo != nullptr) {
        pSky2LightInfo->calcApplyColor(&sky2Color, sead::Color4f::cWhite);
    }

    block->setData(4, &skyColor, 0, 1);
    block->setData(5, &sky2Color, 0, 1);
    block->setValue(6, rate);
    block->setValue(7, mGraphicsSystemInfo->getShadowDirector()->_420);

    if (dirLightKeeper != nullptr) {
        const DirLightParam* param =
            pDirLightName != nullptr ? dirLightKeeper->tryGetNamedOrDefaultDirLight(pDirLightName) :
                                       &dirLightKeeper->getCurrentParam();

        sead::Color4f color = param->getColor();

        if (pDirLightInfo != nullptr) {
            pDirLightInfo->calcApplyColor(&color, color);
        }

        block->setData(8, &color, 0, 1);

        sead::Vector3f dir;
        dir.setRotated(rViewMtx, -param->getDirectionFrom());
        normalizeOrZero(&dir);
        block->setData(9, &dir, 0, 1);

        sead::Color4f spc = param->getSpcColor();
        spc.a = param->getSpcPower();
        block->setData(10, &spc, 0, 1);

        sead::Vector3f spcDir;
        spcDir.setRotated(rViewMtx, -param->getSpcDirectionFrom());
        normalizeOrZero(&spcDir);
        block->setData(11, &spcDir, 0, 1);
    } else {
        block->setData(9, &sead::Vector3f::ez, 0, 1);
        block->setData(11, &sead::Vector3f::ez, 0, 1);
        block->setData(8, &sead::Color4f::cBlue, 0, 1);
    }

    block->setData(12, &up, 0, 1);

    f32 size = orthoSize.x > orthoSize.y ? orthoSize.y : orthoSize.x;
    sead::Matrix44f scaleMtx = sead::Matrix44f::ident;
    scaleMtx.m[0][0] /= size;
    scaleMtx.m[1][1] /= size;
    scaleMtx.m[2][2] = 0.0f;
    sead::Matrix44f envMtx = sead::Matrix44f::ident;
    const ShaderEnvTextureKeeper* envTextureKeeper =
        mGraphicsSystemInfo->getShaderEnvTextureKeeper();

    if (envTextureKeeper != nullptr) {
        if (envTextureKeeper->isUseViewMtx()) {
            envMtx.setMul(scaleMtx, rViewMtx);
        } else {
            sead::Matrix34f frontMtx = sead::Matrix34f::ident;
            sead::Vector3f front = envTextureKeeper->getFrontDir();

            if (normalizeOrZero(&front)) {
                front.set(0.0f, 0.0f, -1.0f);
            }

            makeMtxFrontNoSupport(&frontMtx, front);
            envMtx.setMul(scaleMtx, frontMtx);
        }
    }

    block->setData(29, &envMtx, 0, 4);

    sead::Vector3f cameraPos(invViewMtx.m[0][3], invViewMtx.m[1][3], invViewMtx.m[2][3]);
    block->setData(28, &cameraPos, 0, 1);

    if (mGraphicsSystemInfo->getFogDirector() != nullptr) {
        f32 fogEnd = -mGraphicsSystemInfo->getFogDirector()->getFogParam().getEnd();
        f32 fogStart = -mGraphicsSystemInfo->getFogDirector()->getFogParam().getStart();
        const FogParam& fogParam = mGraphicsSystemInfo->getFogDirector()->getFogParam();
        sead::Color4f fogColor = fogParam.mColor.ref();
        fogColor.a = *fogParam.mIntensityMax;
        block->setData(20, &fogColor, 0, 1);
        block->setData(21, &mGraphicsSystemInfo->getFogDirector()->getFogParam().mMulColor.ref(),
                       0, 1);
        setFogRange(block, 22, fogStart, fogEnd);

        const YFogParam& yFogParam = mGraphicsSystemInfo->getFogDirector()->getYFogParam();
        sead::Color4f yFogColor = yFogParam.mColor.ref();
        yFogColor.a = *yFogParam.mIntensityMax;
        f32 yFogEnd = mGraphicsSystemInfo->getFogDirector()->getYFogParam().getEnd();
        f32 yFogStart = mGraphicsSystemInfo->getFogDirector()->getYFogParam().getStart();
        block->setData(24, &yFogColor, 0, 1);
        block->setData(25, &mGraphicsSystemInfo->getFogDirector()->getYFogParam().mMulColor.ref(),
                       0, 1);
        sead::Vector3f startPos(0.0f, yFogStart, 0.0f);
        sead::Vector3f endPos(0.0f, yFogEnd, 0.0f);
        startPos.mul(rViewMtx);
        endPos.mul(rViewMtx);
        setFogRange(block, 26, startPos.dot(up), endPos.dot(up));
    }

    block->setData(30, &rScreenSize, 0, 1);
    sead::Vector2f texelSize(1.0f / 16.0f, 1.0f / 16.0f);
    block->setData(31, &texelSize, 0, 1);
}

/**
 * Swaps the buffers of all uniform blocks.
 */
void SimpleModelEnv::swapBuffer() {
    for (s32 i = 0; i < mUniformBlocks.size(); i++) {
        mUniformBlocks.at(i)->swap();
    }
}

/**
 * Activates the uniform block of a view for model drawing.
 * @param index View index.
 */
void SimpleModelEnv::prepareModelDraw(s32 index) const {
    UniformBlock* block = mUniformBlocks.at(index);
    block->activate(GameFrameworkNx::getAglDrawContext(), getUniformBlockLocationMdlEnvView());
}

/**
 * Creates the color and apply mode parameters of a light.
 * @param rColor Default color.
 * @param applyMode Default apply mode (0: add, 1: multiply).
 * @param pParamObj Parameter object the parameters are added to.
 * @param pName Light name.
 * @param pLabel Unused label.
 * @param pCategoryName Light category name.
 */
LightInfo::LightInfo(const sead::Color4f& rColor, s32 applyMode,
                     agl::utl::ParameterObj* pParamObj, const char* pName, const char* pLabel,
                     const char* pCategoryName) {
    mColorLabel.format("%s: Color", pName);
    mApplyModeLabel.format("%s：　適用式(0 = 加算、1 = 乗算)", pName);
    StringTmp<128> applyModeName("apply_mode_%s_%s", pName, pCategoryName);
    StringTmp<128> colorName("apply_color_%s_%s", pName, pCategoryName);
    mApplyMode = new agl::utl::Parameter<s32>(applyMode, applyModeName.cstr(),
                                              mApplyModeLabel.cstr(), "Min=0,Max=1", pParamObj);
    mColor = new agl::utl::Parameter<sead::Color4f>(rColor, colorName.cstr(), mColorLabel.cstr(),
                                                    pParamObj);
}

/**
 * Applies the light color to a color.
 * @param pOut Output color.
 * @param rColor Base color.
 */
NOINLINE void LightInfo::calcApplyColor(sead::Color4f* pOut, const sead::Color4f& rColor) const {
    applyColor(pOut, rColor);
}

/**
 * Creates the parameters and the lights of a light category.
 * @param isDefault Whether this is the default category.
 * @param pName Name of the category set.
 * @param dirIntensity Default intensity of the directional light.
 */
CategoryLightInfo::CategoryLightInfo(bool isDefault, const char* pName, f32 dirIntensity)
    : mParamObj(new agl::utl::ParameterObj), mCategoryName(pName),
      mIllumiCoef(1.0f, "illumi_coef",
                  "Simulated light source luminance coefficient (currently only multiplication)",
                  "Min=0,Max=100", mParamObj),
      mZeroIllumiCoef(0.0f, "zero_illumi_coef",
                      "Brightness specification of pseudo-light source with zero brightness",
                      "Min=0,Max=100", mParamObj),
      mModelLightScale(sead::Color4f::cWhite, "model_light_scale", "Model Light Scale",
                       mParamObj),
      mUsingModelLightScale(0, "using_model_light_scale", "モデルライトのスケール",
                            "Min=0, Max=100", mParamObj),
      mIsDefault(isDefault) {
    mSkyLightInfo = new LightInfo(sead::Color4f::cWhite, 1, mParamObj, "sky", nullptr,
                                  mCategoryName);
    mSky2LightInfo = new LightInfo(sead::Color4f::cWhite, 1, mParamObj, "sky2", nullptr,
                                   mCategoryName);
    mDirLightInfo = new LightInfo(
        sead::Color4f(dirIntensity, dirIntensity, dirIntensity, sead::Color4f::cElementMax), 1,
        mParamObj, "dir", nullptr, mCategoryName);
    mLightName.init(sead::FixedSafeString<64>("Default"), "Name", "Name", mParamObj);
}

/**
 * Calculates the model light color of the category.
 * @param pOut Output color.
 * @param rColor Base light color.
 */
void CategoryLightInfo::calcApplyModelLightColor(sead::Color4f* pOut,
                                                 const sead::Color4f& rColor) const {
    switch (*mUsingModelLightScale) {
    case 0:
        mDirLightInfo->applyColor(pOut, rColor);
        break;
    case 1:
        *pOut = rColor;
        break;
    case 2:
        *pOut = *mModelLightScale;
        break;
    case 3:
        *pOut = sead::Color4f::cWhite;
        break;
    default:
        break;
    }
}

/**
 * Creates the default category light info and the named ones.
 * @param pName Name of the category set.
 * @param dirIntensity Default intensity of the directional light.
 */
CategoryLightInfoHolder::CategoryLightInfoHolder(const char* pName, f32 dirIntensity) {
    mParamFilePath = new GraphicsParamFilePath("CategoryLightInfo",
                                               StringTmp<256>("agllitinfo%s", pName).cstr());
    mDefaultInfo = new CategoryLightInfo(true, pName, dirIntensity);
    addObj(mDefaultInfo->getParamObj(), "category_light_info_array");

    for (s32 i = 0; i < mInfos.capacity(); i++) {
        CategoryLightInfo* info = new CategoryLightInfo(false, pName, dirIntensity);
        mInfos.pushBack(info);
        info->getName().format("category_light_info%02d", i);
        info->getLightNameString().format("LightInfo%02d", i);
        addObj(info->getParamObj(), info->getName().cstr());
    }
}

/**
 * Loads the category light infos from a stage resource.
 * @param pResource Stage resource, or nullptr.
 */
void CategoryLightInfoHolder::initStageResource(const Resource* pResource) {
    StringTmp<256> path;
    mParamFilePath->makeBinaryPath(&path);

    if (pResource != nullptr && pResource->isExistFile(path)) {
        applyResParameterArchive(
            agl::utl::ResParameterArchive(pResource->getOtherFile(path, nullptr)));
    }
}

/**
 * Gets the category light info with a light name.
 * @param pName Light name, or nullptr.
 * @return Matching category light info, or the default one if none matches.
 */
const CategoryLightInfo* CategoryLightInfoHolder::tryGetLightInfo(const char* pName) const {
    if (pName != nullptr) {
        s32 infoNum = mInfos.size();

        for (s32 i = 0; i < infoNum; i++) {
            if (isEqualString(pName, mInfos.at(i)->getLightName())) {
                return mInfos.at(i);
            }
        }
    }

    return mDefaultInfo;
}

}  // namespace al
