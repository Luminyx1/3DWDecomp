#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace nn::g3d {
class MaterialObj;
class ModelObj;
class ResModel;
class ResRenderState;
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
class UniformBlockAssignArray;
class GpuMemAllocator;
class ModelAdditionalInfo;
class RenderState;
class Resource;
class ShaderHolder;

class SimpleModelG3D {
public:
    struct Shape {
        Shape();

        void* _0;
        RenderState* mRenderState;
        nn::g3d::ShadingModelObj* mShadingModelObj;
        void* _18;
        EnvTexInfo* mEnvTexInfo;
        bool _28;
        bool _29;
        bool _2a;
    };

    static SimpleModelG3D* createFromOtherModel(SimpleModelG3D* pOther);

    SimpleModelG3D();
    ~SimpleModelG3D();

    bool tryInitFixedMatUbo();
    void initResource(Resource* pResource, s32 bufferNum, sead::Heap* pHeap, GpuMemAllocator* pAllocator);
    void initialize(nn::g3d::ResModel* pResModel, s32 bufferNum, sead::Heap* pHeap,
                    GpuMemAllocator* pAllocator);
    void tryBindShader(const ShaderHolder* pShaderHolder);
    void setModelGlobalAlpha() const;
    void calcBounding();
    void calcBoundingForDepth();
    void updateWorldMatrix(const sead::Matrix34f& rMtx, const sead::Vector3f& rScale);
    void swapGPUBuffer();
    void updateGPUBuffer(const sead::Matrix34f* pViewMtx);
    bool isShapeVisible(s32 index) const;
    void setModelAdditionalInfo(const ModelAdditionalInfo& rInfo) const;
    void setCubeMapIndexAllShape(s32 index);
    const nn::g3d::ResRenderState* getResRenderState(s32 index) const;
    nn::g3d::ResRenderState* getResRenderStatePtr(s32 index);
    void useCustomRenderState(agl::DrawContext* pContext, nn::g3d::MaterialObj* pMaterial,
                              s32 index) const;
    void resetResRenderState(s32 index);
    void createResRenderState(s32 index);
    bool isCreateResRenderState(s32 index) const;
    nn::g3d::MaterialObj* getMaterialObj(s32 index) const;
    void setPostUpdateWorldMatrixCallback(const FunctorBase& rFunctor);
    void updateLod(const sead::Vector3f& rPos, s32 lodIndex);
    void setLodParams(const f32* pLodDistances, s32 lodNum);

    nn::g3d::ModelObj* getModelObj() const { return mModelObj; }

    const Shape& getShape(s32 index) const { return (*mShapes)[index]; }

    bool isDisableDraw() const { return mIsDisableDraw; }

    void setDisableDraw(bool isDisable) { mIsDisableDraw = isDisable; }

    bool isDisableDepthShadow() const { return mIsDisableDepthShadow || mIsDisableDraw; }

    void setDisableDepthShadow(bool isDisable) { mIsDisableDepthShadow = isDisable; }

    s32 getLodNum() const { return mLodNum; }

    void setGlobalAlphaPtr(f32* pAlpha) { mGlobalAlpha = pAlpha; }

    void setGlobalYOffsetPtr(f32* pYOffset) { mGlobalYOffset = pYOffset; }

    bool mIsCreatedFromOther;
    nn::g3d::ModelObj* mModelObj;
    sead::Buffer<Shape>* mShapes;
    GpuMemAllocator* mGpuMemAllocator;
    UniformBlockAssignArray* mUniformBlockAssignArray;
    void* _28;
    bool mIsVisible;
    bool mIsDisableDepthShadow;
    bool mIsDisableDraw;
    const s32* mCurrentBufferIndex;
    s32 mBufferNum;
    bool _44;
    bool _45;
    bool _46;
    bool mIsForceActivateTexture;
    void* _48;
    s32 mLodNum;
    f32 _54;
    f32 _58;
    f32 _5c;
    f32 _60;
    s32 mLodIndex;
    s32 mLodUpdateCount;
    const sead::Vector3f* mLodPos;
    bool mIsLodDisabled;
    f32* mGlobalAlpha;
    f32* mGlobalYOffset;
};

static_assert(sizeof(SimpleModelG3D) == 0x90);

}  // namespace al
