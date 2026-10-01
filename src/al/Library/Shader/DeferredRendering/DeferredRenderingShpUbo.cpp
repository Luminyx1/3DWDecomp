#include "Library/Shader/DeferredRendering/DeferredRenderingShpUbo.hpp"

#include <g3d/aglShaderUtilG3D.h>
#include <math/seadVector.h>

#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/DeferredRenderingShader.hpp"

namespace {
const al::UniformBlockLayout cShpUboLayout[] = {
    {0, agl::UniformBlock::cType_Vec4, 3},
    {1, agl::UniformBlock::cType_Int, 1},
};
}  // namespace

namespace al {

/**
 * @brief Creates the uniform blocks for the shape block of a deferred rendering shader.
 */
DeferredRenderingShpUbo::DeferredRenderingShpUbo(const DeferredRenderingShader* pShader,
                                                 s32 uboNum)
    : mUboNum(uboNum), mUbos(new UniformBlock*[uboNum]), mMtxUpdateCount(uboNum),
      mWeightNumUpdateCount(uboNum) {
    mMtx.makeIdentity();
    agl::g3d::ShaderUtilG3D::search(&mLocation, pShader->getShadingModel(),
                                    pShader->getShaderProgram(), "shape");
    for (u32 i = 0; i < mUboNum; i++) {
        mUbos[i] = createUniformBlock(cShpUboLayout, 2, nullptr, 2);
    }
}

/**
 * @brief Creates the uniform blocks for the shape block of a shading model program.
 */
DeferredRenderingShpUbo::DeferredRenderingShpUbo(const nn::g3d::ResShadingModel* pShadingModel,
                                                 const nn::g3d::ResShaderProgram* pShaderProgram,
                                                 s32 uboNum)
    : mUboNum(uboNum), mUbos(new UniformBlock*[uboNum]), mMtxUpdateCount(uboNum),
      mWeightNumUpdateCount(uboNum) {
    mMtx.makeIdentity();
    agl::g3d::ShaderUtilG3D::search(&mLocation, pShadingModel, pShaderProgram, "shape");

    for (u32 i = 0; i < mUboNum; i++) {
        mUbos[i] = createUniformBlock(cShpUboLayout, 2, nullptr, 2);
    }
}

/**
 * @brief Sets the model matrix, to be written to every block of the ring.
 */
void DeferredRenderingShpUbo::setMtx(const sead::Matrix34f* pMtx) {
    mMtxUpdateCount = mUboNum;
    mMtx = *pMtx;
}

/**
 * @brief Sets the skinning weight count, to be written to every block of the ring.
 */
void DeferredRenderingShpUbo::setWeightNum(s32 weightNum) {
    mWeightNum = weightNum;
    mWeightNumUpdateCount = mUboNum;
}

/**
 * @brief Writes the pending values to the current block and moves to the next one.
 */
void DeferredRenderingShpUbo::swap() {
    if (mMtxUpdateCount > 0) {
        mMtxUpdateCount--;

        for (s32 i = 0; i < 3; i++) {
            sead::Vector4f row;
            mMtx.getRow(row, i);
            mUbos[mCurrentIndex]->setData(0, &row, i, 1);
        }
    }

    if (mWeightNumUpdateCount > 0) {
        mWeightNumUpdateCount--;
        UniformBlock* ubo = mUbos[mCurrentIndex];
        s32 weightNum = mWeightNum;
        ubo->setData(1, &weightNum, 0, 1);
    }

    const UniformBlock* ubo = mUbos[mCurrentIndex];
    u32 offset = ubo->getCurrentBlockOffset(0);
    agl::GPUMemVoidAddr addr = ubo->getBuffer();
    agl::GPUMemVoidAddr(addr, offset).flushCPUCache(ubo->getBlockSize());
    mCurrentIndex = mCurrentIndex + 1 >= mUboNum ? 0 : mCurrentIndex + 1;
}

/**
 * @brief Binds the block written last.
 */
void DeferredRenderingShpUbo::activate() {
    s32 index = (mCurrentIndex + mUboNum - 1) % mUboNum;
    mUbos[index]->activate(
        GameFrameworkNx::getAglDrawContext(), mLocation);
}

}  // namespace al
