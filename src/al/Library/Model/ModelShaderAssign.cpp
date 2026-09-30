#include "Library/Model/ModelShaderAssign.hpp"

#include <g3d/aglG3DDecl.h>
#include <heap/seadHeap.h>
#include <g3d/aglShaderUtilG3D.h>
#include <nn/g3d/g3d_ResShader.h>

namespace al {

/**
 * Constructs an empty shader assignment.
 */
ModelShaderAssign::ModelShaderAssign() {
    clear_();
}

/**
 * Clears the vertex attribute and sampler bindings.
 */
void ModelShaderAssign::clear_() {
    mAttribute.clear();
    mSampler.clear();
}

/**
 * Destroys the shader assignment and its fetch shader buffer.
 */
ModelShaderAssign::~ModelShaderAssign() {
    if (mFetchShaderBuffer) {
        delete[] mFetchShaderBuffer;
        mFetchShaderBuffer = nullptr;
    }
}

/**
 * Allocates the fetch shader buffer.
 * @param pHeap Heap to allocate from.
 */
void ModelShaderAssign::create(sead::Heap* pHeap) {
    mFetchShaderBuffer = new (pHeap, 8) u8[static_cast<u32>(mAttribute.getFetchShaderBufferSize())];
    mAttribute.setFetchShaderBuffer(mFetchShaderBuffer, mAttribute.getFetchShaderBufferSize());
}

/**
 * Sets an externally allocated fetch shader buffer.
 * @param pBuffer Fetch shader buffer.
 */
void ModelShaderAssign::setBuffer(void* pBuffer) {
    mAttribute.setFetchShaderBuffer(pBuffer, mAttribute.getFetchShaderBufferSize());
}

/**
 * Binds the samplers, vertex attributes and material block of a shader program to a shape.
 * @param pMaterial Material resource.
 * @param pShape Shape resource.
 * @param pShadingModel Shading model resource.
 * @param pProgram Shader program resource.
 */
void ModelShaderAssign::bind(const nn::g3d::ResMaterial* pMaterial, const nn::g3d::ResShape* pShape,
                             const nn::g3d::ResShadingModel* pShadingModel,
                             const nn::g3d::ResShaderProgram* pProgram) {
    clear_();

    if (!pProgram) {
        return;
    }

    mShaderProgram = nullptr;
    mResShaderProgram = pProgram;
    mSampler.bind(pMaterial, pShadingModel, pProgram);
    mAttribute.bind(pMaterial, pShape, pShadingModel, pProgram);
    updateLocation_(pMaterial, pShadingModel, nullptr);
}

/**
 * Searches the location of the material uniform block.
 * @param pMaterial Material resource.
 * @param pShadingModel Shading model resource.
 * @param pName Uniform block name used with an agl shader program.
 */
void ModelShaderAssign::updateLocation_(const nn::g3d::ResMaterial* pMaterial,
                                        const nn::g3d::ResShadingModel* pShadingModel,
                                        const char* pName) {
    if (mShaderProgram) {
        mMaterialBlockLocation.setName(pName);
        mMaterialBlockLocation.search(*mShaderProgram);
    } else if (mResShaderProgram) {
        s32 blockIndex = pShadingModel->GetMaterialBlockIndex();

        if (blockIndex != -1) {
            agl::g3d::ShaderUtilG3D::search(&mMaterialBlockLocation, pShadingModel,
                                            mResShaderProgram,
                                            pShadingModel->GetUniformBlockName(blockIndex));
        }
    }
}

/**
 * Activates the material uniform block of a material.
 * @param pContext Draw context.
 * @param pMaterial Material.
 * @param bufferIndex Index of the material block buffer.
 */
void ModelShaderAssign::activateMaterialUniformBlock(agl::DrawContext* pContext,
                                                     const nn::g3d::MaterialObj* pMaterial,
                                                     s32 bufferIndex) const {
    if (!mMaterialBlockLocation.isValid()) {
        return;
    }

    size_t blockSize = pMaterial->GetMaterialBlockSize();

    if (blockSize == 0) {
        return;
    }

    agl::g3d::ShaderUtilG3D::load(
        pContext, mMaterialBlockLocation,
        *reinterpret_cast<const nn::gfx::Buffer*>(
            const_cast<nn::g3d::MaterialObj*>(pMaterial)->GetMaterialBlock(bufferIndex)),
        blockSize, bufferIndex);
}

}  // namespace al
