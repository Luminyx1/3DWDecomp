#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace nn::g3d {
class MaterialObj;
class ModelObj;
class ResModel;
class ShaderSelector;
class ShadingModelObj;
}  // namespace nn::g3d

namespace sead {
class Heap;
}

namespace agl {
class DrawContext;
}

namespace al {
class EnvTexInfo;
class FunctorBase;
class GpuMemAllocator;
class ModelAdditionalInfo;
class ModelShaderAssign;
class RenderState;
class Resource;
class ShaderHolder;
struct UniformBlockAssign;
class UniformBlockAssignArray;

class SimpleModelG3D {
public:
    struct Shape {
        Shape();

        ModelShaderAssign* mShaderAssign = nullptr;
        RenderState* mRenderState = nullptr;
        const nn::g3d::ShadingModelObj* mShadingModelObj = nullptr;
        nn::g3d::ShaderSelector* mShaderSelector = nullptr;
        EnvTexInfo* mEnvTexInfo = nullptr;
        bool mIsRenderMaterial = true;
        bool mIsRenderCloudLayer = false;
        bool _2a = false;  // Material has to be recalculated every frame.
    };

    static_assert(sizeof(Shape) == 0x30);

    static SimpleModelG3D* createFromOtherModel(SimpleModelG3D* pOther);

    SimpleModelG3D();
    ~SimpleModelG3D();

    bool tryInitFixedMatUbo();
    void initResource(Resource* pResource, s32 viewNum, sead::Heap* pHeap,
                      GpuMemAllocator* pAllocator);
    void initialize(nn::g3d::ResModel* pResModel, s32 viewNum, sead::Heap* pHeap,
                    GpuMemAllocator* pAllocator);
    bool tryBindShader(const ShaderHolder* pShaderHolder);
    void setModelGlobalAlpha() const;
    void calcBounding();
    void calcBoundingForDepth();
    void updateWorldMatrix(const sead::Matrix34f& rMtx, const sead::Vector3f& rScale);
    void swapGPUBuffer();
    void updateGPUBuffer(const sead::Matrix34f* pViewMtx);
    bool isShapeVisible(s32 index) const;
    void setModelAdditionalInfo(const ModelAdditionalInfo& rInfo) const;
    void setCubeMapIndexAllShape(s32 index);
    const RenderState* getResRenderState(s32 index) const;
    RenderState* getResRenderStatePtr(s32 index);
    void useCustomRenderState(agl::DrawContext* pContext, nn::g3d::MaterialObj* pMaterial,
                              s32 index) const;
    void resetResRenderState(s32 index);
    void createResRenderState(s32 index);
    bool isCreateResRenderState(s32 index) const;
    nn::g3d::MaterialObj* getMaterialObj(s32 index) const;
    void setPostUpdateWorldMatrixCallback(const FunctorBase& rFunctor);
    void updateLod(const sead::Vector3f& rPos, s32 updateCount);
    void setLodParams(const f32* pLodDistances, s32 lodNum);

    nn::g3d::ModelObj* getModelObj() const { return mModelObj; }

    bool isVisible() const { return mIsVisible; }

    const Shape& getShape(s32 index) const { return (*mShapes)[index]; }

    bool isDisableDraw() const { return mIsDisableDraw; }

    void setDisableDraw(bool isDisable) { mIsDisableDraw = isDisable; }

    bool isDisableDepthShadow() const { return mIsDisableDepthShadow || mIsDisableDraw; }

    void setDisableDepthShadow(bool isDisable) { mIsDisableDepthShadow = isDisable; }

    s32 getLodNum() const { return mLodNum; }

    void setGlobalAlphaPtr(f32* pAlpha) { mGlobalAlpha = pAlpha; }

    void setGlobalYOffsetPtr(f32* pYOffset) { mGlobalYOffset = pYOffset; }

    GpuMemAllocator* getGpuMemAllocator() const { return mGpuMemAllocator; }

    UniformBlockAssignArray* getUniformBlockAssignArray() const { return mUniformBlockAssignArray; }

    s32 getCurrentBufferIndex() const { return *mCurrentBufferIndex; }

    bool isForceActivateTexture() const { return mIsForceActivateTexture; }

    bool isLodDisabled() const { return mIsLodDisabled; }

    s32 getLodIndex() const { return mLodIndex; }

    s32 getLodUpdateCount() const { return mLodUpdateCount; }

    bool mIsCreatedFromOther = false;
    nn::g3d::ModelObj* mModelObj = nullptr;
    sead::Buffer<Shape>* mShapes = nullptr;
    GpuMemAllocator* mGpuMemAllocator = nullptr;
    UniformBlockAssignArray* mUniformBlockAssignArray = nullptr;
    UniformBlockAssign* _28 = nullptr;  // "cModelAdditionalInfo" uniform block.
    bool mIsVisible = true;
    bool mIsDisableDepthShadow = false;
    bool mIsDisableDraw = false;
    s32* mCurrentBufferIndex = nullptr;
    s32 mBufferNum = 2;
    bool _44 = true;  // Bounding is calculated in updateWorldMatrix.
    bool _45 = true;  // Material is calculated in updateGPUBuffer.
    bool mIsDirtyBoundingForDepth = false;
    bool mIsForceActivateTexture = false;
    FunctorBase* mPostUpdateWorldMatrixCallback = nullptr;
    s32 mLodNum;
    f32 mLodDistanceSq[4];
    s32 mLodIndex = 0;
    s32 mLodUpdateCount = -1;
    const sead::Vector3f* mLodPos = nullptr;
    bool mIsLodDisabled = true;
    f32* mGlobalAlpha = nullptr;
    f32* mGlobalYOffset = nullptr;
};

static_assert(sizeof(SimpleModelG3D) == 0x90);

}  // namespace al
