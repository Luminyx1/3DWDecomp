#include "Project/Model/SimpleModelG3D.hpp"

#include <common/aglGPUMemAddr.h>
#include <common/aglGPUMemBlock.h>
#include <common/aglShaderLocation.h>
#include <driver/aglGraphicsDriverMgr.h>
#include <gfx/seadColor.h>
#include <gfx/seadGraphics.h>
#include <heap/seadHeapMgr.h>
#include <limits>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResFile.h>
#include <nn/g3d/g3d_ResShader.h>
#include <nn/g3d/g3d_SkeletonObj.h>
#include <nn/util/util_MatrixApi.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Light/DirectionalLightKeeper.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/ModelShaderAssign.hpp"
#include "Library/Model/SimpleModelEnv.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDirector.hpp"
#include "Library/Shader/ForwardRendering/CubeMapKeeper.hpp"
#include "Library/Shader/ForwardRendering/EnvTextureKeeper.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GpuMemAllocator.hpp"
#include "Project/Draw/RenderState.hpp"
#include "Project/Model/ModelAdditionalInfo.hpp"
#include "Project/Model/UniformBlockAssign.hpp"

namespace al {

static const UniformBlockLayout sModelAdditionalInfoLayout[] = {
    {0, agl::UniformBlock::cType_Vec4, 1},
    {1, agl::UniformBlock::cType_Float, 1},
    {2, agl::UniformBlock::cType_Float, 1},
    {3, agl::UniformBlock::cType_Float, 1},
};

/**
 * Flushes the CPU cache of the current buffer of a uniform block.
 * @param pBlock Uniform block.
 */
static void flushUniformBlock(const UniformBlock* pBlock) {
    s32 offset = pBlock->getCurrentBlockOffset(0);
    agl::GPUMemVoidAddr(pBlock->getBuffer(), offset).flushCPUCache(pBlock->getBlockSize());
}

/**
 * Checks a bit of a packed bit array.
 * @param pArray Bit array.
 * @param index Bit index.
 * @return Whether the bit is set.
 */
static bool isBitOn(const u32* pArray, s32 index) {
    return (pArray[index >> 5] & (1 << index)) != 0;
}

/**
 * Gets the graphics device.
 * @return The graphics device.
 */
static nn::gfx::Device* getGfxDevice() {
    return static_cast<nn::gfx::Device*>(
        agl::driver::GraphicsDriverMgr::instance()->getGfxDevice());
}

/**
 * Gets the name of the shading model assigned to a material.
 * @param pMaterial Material.
 * @return The shading model name.
 */
static const char* getShadingModelName(const nn::g3d::MaterialObj* pMaterial) {
    return pMaterial->GetResource()->GetShaderAssign()->GetShadingModelName();
}

/**
 * Constructs a shape with its own render state.
 */
SimpleModelG3D::Shape::Shape() {
    mRenderState = new RenderState();
}

/**
 * Creates a model sharing the model object, shapes and uniform blocks of another model.
 * @param pOther Model to share the resources of.
 * @return The new model.
 */
SimpleModelG3D* SimpleModelG3D::createFromOtherModel(SimpleModelG3D* pOther) {
    SimpleModelG3D* model = new SimpleModelG3D();
    model->mIsCreatedFromOther = true;
    model->mModelObj = pOther->mModelObj;
    model->mShapes = pOther->mShapes;
    model->mUniformBlockAssignArray = pOther->mUniformBlockAssignArray;
    model->mCurrentBufferIndex = pOther->mCurrentBufferIndex;
    model->mGlobalAlpha = pOther->mGlobalAlpha;
    model->mGlobalYOffset = pOther->mGlobalYOffset;
    model->_28 = pOther->_28;
    model->_45 = pOther->_45;

    if (!model->_45) {
        model->tryInitFixedMatUbo();
    }

    model->mGpuMemAllocator = pOther->mGpuMemAllocator;
    return model;
}

/**
 * Calculates the material blocks of all buffers once, unless a shape needs them every frame.
 * @return Whether the material blocks were fixed.
 */
bool SimpleModelG3D::tryInitFixedMatUbo() {
    for (s32 i = 0; i < mModelObj->GetNumShapes(); i++) {
        if ((*mShapes)[i]._2a) {
            return false;
        }
    }

    _45 = false;

    for (s32 i = 0; i < mBufferNum; i++) {
        mModelObj->CalculateMaterial(i);
    }

    return true;
}

/**
 * Constructs an empty model.
 */
SimpleModelG3D::SimpleModelG3D() {
    mLodNum = 0;
    mLodDistanceSq[0] = 15000.0f * 15000.0f;
    mLodDistanceSq[1] = 30000.0f * 30000.0f;
    mLodDistanceSq[2] = std::numeric_limits<f32>::max();
    mLodDistanceSq[3] = std::numeric_limits<f32>::max();
}

/**
 * Destroys the shapes and uniform blocks unless they are shared with another model.
 */
SimpleModelG3D::~SimpleModelG3D() {
    if (mIsCreatedFromOther) {
        return;
    }

    Shape* shapes = mShapes->getBufferPtr();

    for (s32 i = 0; i != mShapes->size(); i++) {
        Shape& shape = shapes[i];

        if (shape.mEnvTexInfo != nullptr) {
            delete shape.mEnvTexInfo;
            shape.mEnvTexInfo = nullptr;
        }

        if (shape.mShaderAssign != nullptr) {
            delete shape.mShaderAssign;
            shape.mShaderAssign = nullptr;
        }

        if (shape.mShaderSelector != nullptr) {
            delete shape.mShaderSelector;
            shape.mShaderSelector = nullptr;
        }

        if (shape.mRenderState != nullptr) {
            delete shape.mRenderState;
            shape.mRenderState = nullptr;
        }
    }

    if (mUniformBlockAssignArray != nullptr) {
        for (auto it = mUniformBlockAssignArray->begin(); it != mUniformBlockAssignArray->end();
             ++it) {
            if (it->mLocation != nullptr) {
                delete it->mLocation;
                it->mLocation = nullptr;
            }

            if (it->mUniformBlock != nullptr) {
                delete it->mUniformBlock;
                it->mUniformBlock = nullptr;
            }
        }

        mUniformBlockAssignArray->freeBuffer();
        delete mUniformBlockAssignArray;
    }

    delete mCurrentBufferIndex;
    delete mShapes;
}

/**
 * Initializes the model from the first model of a resource.
 * @param pResource Resource holding the model.
 * @param viewNum Number of views.
 * @param pHeap Heap to allocate from.
 * @param pAllocator Allocator of the GPU memory.
 */
void SimpleModelG3D::initResource(Resource* pResource, s32 viewNum, sead::Heap* pHeap,
                                  GpuMemAllocator* pAllocator) {
    sead::Graphics::instance()->lockDrawContext();
    initialize(pResource->getResFile()->GetModel(0), viewNum, pHeap, pAllocator);
    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * Creates the model object, its GPU blocks, the shapes and the uniform blocks of the model.
 * @param pResModel Model resource.
 * @param viewNum Number of views.
 * @param pHeap Heap to allocate from.
 * @param pAllocator Allocator of the GPU memory.
 */
void SimpleModelG3D::initialize(nn::g3d::ResModel* pResModel, s32 viewNum, sead::Heap* pHeap,
                                GpuMemAllocator* pAllocator) {
    mGpuMemAllocator = pAllocator;
    sead::ScopedCurrentHeapSetter setter(pHeap);
    mModelObj = new nn::g3d::ModelObj();
    mShapes = new sead::Buffer<Shape>();
    mCurrentBufferIndex = new s32(0);
    const ShaderHolder* shaderHolder = ShaderHolder::sInstance;
    alModelFunction::bindShaderParamAndConvertParamCallback(pResModel, shaderHolder);

    nn::g3d::ModelObj::InitializeArgument argument(pResModel);
    argument.boundingEnabled = true;
    argument.skeletonBufferCount = mBufferNum;
    argument.shapeBufferCount = mBufferNum;
    argument.materialBufferCount = mBufferNum;
    argument.viewCount = viewNum;
    argument.CalculateMemorySize();
    size_t memorySize = argument.memorySize;
    u8* buffer = new (0x10) u8[memorySize];
    mModelObj->Initialize(argument, buffer, memorySize);

    u32 blockBufferSize = mModelObj->CalculateBlockBufferSize(getGfxDevice());

    if (blockBufferSize != 0) {
        s32 alignment = mModelObj->GetBlockBufferAlignment(getGfxDevice());
        agl::GPUMemAddrBase addr =
            mGpuMemAllocator->allocMemory("ModelUBO", blockBufferSize, alignment);
        nn::gfx::MemoryPool* memoryPool = mGpuMemAllocator->allocMemoryPool();
        addr.getMemoryBlock()->initializeGfxMemoryPool(memoryPool);
        mModelObj->SetupBlockBuffer(getGfxDevice(), memoryPool, addr.getByteOffset(),
                                    blockBufferSize);
        mGpuMemAllocator->registerModelObj(mModelObj);
    }

    mShapes->tryAllocBuffer(mModelObj->GetNumShapes(), pHeap);
    tryBindShader(shaderHolder);

    mUniformBlockAssignArray = new UniformBlockAssignArray();
    mUniformBlockAssignArray->tryAllocBuffer(4, nullptr);
    UniformBlockAssign* assign =
        tryCreateUniformBlockAssign(this, "cModelAdditionalInfo", sModelAdditionalInfoLayout, 4);
    _28 = assign;

    if (assign != nullptr) {
        setModelGlobalAlpha();
        assign->mUniformBlock->swap();
        setModelGlobalAlpha();
    }
}

/**
 * Creates the shader selectors, shader assigns and environment texture infos of all shapes.
 * @param pShaderHolder Holder of the shading models.
 * @return Always true.
 */
bool SimpleModelG3D::tryBindShader(const ShaderHolder* pShaderHolder) {
    Shape* shapes = mShapes->getBufferPtr();

    for (s32 i = 0; i != mShapes->size(); i++) {
        Shape& shape = shapes[i];
        nn::g3d::ShapeObj* shapeObj = mModelObj->GetShape(i);
        nn::g3d::MaterialObj* materialObj =
            mModelObj->GetMaterial(shapeObj->GetResource()->GetMaterialIndex());
        nn::g3d::ResShadingModel* shadingModel =
            pShaderHolder->getShadingModel(getShadingModelName(materialObj));
        shape.mIsRenderMaterial =
            isEqualSubString(getShadingModelName(materialObj), "RenderMaterial");
        shape.mIsRenderCloudLayer =
            isEqualSubString(getShadingModelName(materialObj), "alRenderCloudLayer");

        if (shape.mIsRenderMaterial) {
            alModelFunction::updateRenderMaterialUbo(materialObj);
        } else if (shape.mIsRenderCloudLayer) {
            alModelFunction::updateRenderCloudLayerUbo(materialObj);
        }

        shape.mShaderSelector = alModelFunction::createShaderSelector(
            mGpuMemAllocator, shapeObj, materialObj, shadingModel, 0, nullptr, nullptr, false);
        shape.mShadingModelObj = shape.mShaderSelector->GetShadingModel();
        shape.mShaderAssign = new ModelShaderAssign();
        shape.mShaderAssign->create(nullptr);
        shape.mShaderAssign->bind(materialObj->GetResource(), shapeObj->GetResource(),
                                  shadingModel, shape.mShaderSelector->GetProgram());
        shape.mEnvTexInfo = new EnvTexInfo(*materialObj->GetResource(), *shape.mShadingModelObj);
    }

    return true;
}

/**
 * Writes the global alpha and Y offset into the model additional info uniform block.
 */
void SimpleModelG3D::setModelGlobalAlpha() const {
    UniformBlockAssign* assign = _28;

    if (assign == nullptr) {
        return;
    }

    f32 alpha = mGlobalAlpha != nullptr ? *mGlobalAlpha : 1.0f;
    assign->mUniformBlock->setValue(2, alpha);
    f32 yOffset = mGlobalYOffset != nullptr ? *mGlobalYOffset : 0.0f;
    assign->mUniformBlock->setValue(3, yOffset);
    flushUniformBlock(assign->mUniformBlock);
}

/**
 * Calculates the bounding of the model and the sub mesh bounding of all shapes.
 */
void SimpleModelG3D::calcBounding() {
    mModelObj->CalculateBounding(0);
    s32 shapeNum = mModelObj->GetNumShapes();

    for (s32 i = 0; i < shapeNum; i++) {
        mModelObj->GetShape(i)->CalculateSubMeshBounding(mModelObj->GetSkeleton(), 0);
    }
}

/**
 * Calculates the bounding for the depth shadow if it was not updated with the world matrix.
 */
void SimpleModelG3D::calcBoundingForDepth() {
    if (_44 || !mIsDirtyBoundingForDepth) {
        return;
    }

    calcBounding();
    mIsDirtyBoundingForDepth = false;
}

/**
 * Calculates the world matrices of the skeleton.
 * @param rMtx Base matrix.
 * @param rScale Scale applied to the base matrix.
 */
void SimpleModelG3D::updateWorldMatrix(const sead::Matrix34f& rMtx, const sead::Vector3f& rScale) {
    sead::Matrix34f mtx = rMtx;
    mtx.scaleBases(rScale.x, rScale.y, rScale.z);
    nn::util::Matrix4x3fType worldMtx;
    nn::util::MatrixLoad(&worldMtx, reinterpret_cast<const nn::util::FloatColumnMajor4x3&>(mtx));
    mModelObj->GetSkeleton()->CalculateWorldMtx(worldMtx);
    mIsDirtyBoundingForDepth = true;

    if (_44) {
        calcBounding();
    }

    if (mPostUpdateWorldMatrixCallback != nullptr) {
        (*mPostUpdateWorldMatrixCallback)();
    }
}

/**
 * Advances to the next GPU buffer.
 */
void SimpleModelG3D::swapGPUBuffer() {
    *mCurrentBufferIndex = (*mCurrentBufferIndex + 1) % mBufferNum;

    if (_28 != nullptr) {
        _28->mUniformBlock->swap();
    }

    UniformBlockAssign* invincible =
        findUniformBlockAssign(mUniformBlockAssignArray, "cInvincible");

    if (invincible != nullptr) {
        invincible->mUniformBlock->swap();
    }
}

/**
 * Calculates the view, skeleton, shape and material blocks of the current GPU buffer.
 * @param pViewMtx View matrix.
 */
void SimpleModelG3D::updateGPUBuffer(const sead::Matrix34f* pViewMtx) {
    s32 bufferIndex = *mCurrentBufferIndex;

    for (s32 i = 0; i < mModelObj->GetViewCount(); i++) {
        nn::util::Matrix4x3fType viewMtx;
        nn::util::MatrixLoad(&viewMtx,
                             *reinterpret_cast<const nn::util::FloatColumnMajor4x3*>(pViewMtx));
        mModelObj->CalculateView(i, viewMtx, bufferIndex);
    }

    mModelObj->CalculateSkeleton(bufferIndex);
    mModelObj->CalculateShape(bufferIndex);

    if (_45) {
        mModelObj->CalculateMaterial(bufferIndex);
    }
}

/**
 * Checks whether the material and the bone of a shape are visible.
 * @param index Shape index.
 * @return Whether the shape is visible.
 */
bool SimpleModelG3D::isShapeVisible(s32 index) const {
    const nn::g3d::ResShape* shape = mModelObj->GetShape(index)->GetResource();
    return isBitOn(mModelObj->GetMaterialVisibilityArray(), shape->GetMaterialIndex()) &&
           isBitOn(mModelObj->GetBoneVisibilityArray(), shape->GetBoneIndex());
}

/**
 * Writes the light color, cube map index, global alpha and Y offset into the model additional
 * info uniform block.
 * @param rInfo Additional info of the model.
 */
void SimpleModelG3D::setModelAdditionalInfo(const ModelAdditionalInfo& rInfo) const {
    UniformBlockAssign* assign = _28;

    if (assign == nullptr) {
        return;
    }

    sead::Color4f color = sead::Color4f::cWhite;
    const sead::Color4f& lightColor =
        rInfo.getGraphicsSystemInfo()->getDirectionalLightKeeper()->getCurrentColor();
    const CategoryLightInfo* lightInfo =
        static_cast<const CategoryLightInfo*>(rInfo.getLightInfo());

    if (lightInfo != nullptr) {
        lightInfo->calcApplyModelLightColor(&color, lightColor);
    } else {
        color = lightColor;
    }

    assign->mUniformBlock->setData(0, &color, 0, 1);
    assign->mUniformBlock->setValue<f32>(1, rInfo.getGraphicsSystemInfo()
                                                ->getCubeMapDirector()
                                                ->getShaderCubeMapKeeper()
                                                ->getModelLightIntensity());
    f32 alpha = mGlobalAlpha != nullptr ? *mGlobalAlpha : 1.0f;
    assign->mUniformBlock->setValue(2, alpha);
    f32 yOffset = mGlobalYOffset != nullptr ? *mGlobalYOffset : 0.0f;
    assign->mUniformBlock->setValue(3, yOffset);
    flushUniformBlock(assign->mUniformBlock);
}

/**
 * Overrides the cube map of all shapes.
 * @param index Cube map index.
 */
void SimpleModelG3D::setCubeMapIndexAllShape(s32 index) {
    Shape* shapes = mShapes->getBufferPtr();

    for (s32 i = 0; i != mShapes->size(); i++) {
        shapes[i].mEnvTexInfo->setOverrideCubeMapId(index);
        shapes[i].mEnvTexInfo->setOverrideIrradiance(index);
        shapes[i].mEnvTexInfo->setOverrideRefractCubeMapId(index);
    }
}

/**
 * Gets the render state of a shape.
 * @param index Shape index.
 * @return The render state.
 */
const RenderState* SimpleModelG3D::getResRenderState(s32 index) const {
    return (*mShapes)[index].mRenderState;
}

/**
 * Gets the render state of a shape.
 * @param index Shape index.
 * @return The render state.
 */
RenderState* SimpleModelG3D::getResRenderStatePtr(s32 index) {
    return (*mShapes)[index].mRenderState;
}

/**
 * Applies the render state of a shape if it is enabled.
 * @param pContext Draw context.
 * @param pMaterial Material of the shape.
 * @param index Shape index.
 */
void SimpleModelG3D::useCustomRenderState(agl::DrawContext* pContext,
                                          nn::g3d::MaterialObj* pMaterial, s32 index) const {
    const RenderState* renderState = getResRenderState(index);

    if (renderState->isEnable()) {
        renderState->Use(pContext, pMaterial);
    }
}

/**
 * Does nothing.
 * @param index Shape index.
 */
void SimpleModelG3D::resetResRenderState(s32 index) {}

/**
 * Creates the render state of a shape if it does not exist yet.
 * @param index Shape index.
 */
void SimpleModelG3D::createResRenderState(s32 index) {
    if ((*mShapes)[index].mRenderState != nullptr) {
        return;
    }

    (*mShapes)[index].mRenderState = new RenderState();
}

/**
 * Checks whether the render state of a shape was created on demand.
 * @param index Shape index.
 * @return Always false.
 */
bool SimpleModelG3D::isCreateResRenderState(s32 index) const {
    return false;
}

/**
 * Gets the material object of a shape.
 * @param index Shape index.
 * @return The material object.
 */
nn::g3d::MaterialObj* SimpleModelG3D::getMaterialObj(s32 index) const {
    return mModelObj->GetMaterial(mModelObj->GetShape(index)->GetResource()->GetMaterialIndex());
}

/**
 * Sets the callback called after the world matrices were updated.
 * @param rFunctor Callback, which is cloned.
 */
void SimpleModelG3D::setPostUpdateWorldMatrixCallback(const FunctorBase& rFunctor) {
    mPostUpdateWorldMatrixCallback = rFunctor.clone();
}

/**
 * Selects the level of detail by the distance to the LOD position.
 * @param rPos Position of the viewer.
 * @param updateCount Update counter of the LOD.
 */
void SimpleModelG3D::updateLod(const sead::Vector3f& rPos, s32 updateCount) {
    mLodUpdateCount = updateCount;
    f32 distanceSq = (rPos - *mLodPos).squaredLength();
    s32 lodIndex = 0;

    for (s32 i = mLodNum - 1; i >= 0; i--) {
        if (distanceSq > mLodDistanceSq[i]) {
            lodIndex = i + 1;
            break;
        }
    }

    s32 lodCount = mModelObj->GetLodCount();
    mLodIndex = lodCount > lodIndex ? lodIndex : lodCount - 1;
}

/**
 * Sets the LOD distances.
 * @param pLodDistances Distances at which the next level of detail is used.
 * @param lodNum Number of distances.
 */
void SimpleModelG3D::setLodParams(const f32* pLodDistances, s32 lodNum) {
    mLodNum = lodNum;

    for (s32 i = 0; i < lodNum; i++) {
        mLodDistanceSq[i] = pLodDistances[i] * pLodDistances[i];
    }

    if (lodNum <= 0) {
        mIsLodDisabled = true;
    } else if (mModelObj->GetLodCount() <= 1) {
        mIsLodDisabled = true;
    }
}

}  // namespace al
