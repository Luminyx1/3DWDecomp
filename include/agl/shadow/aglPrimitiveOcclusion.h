#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadBuffer2.h>
#include <container/seadListImpl.h>
#include <container/seadOffsetList.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglShaderEnum.h"
#include "common/aglShaderLocation.h"
#include "common/aglUniformBlock.h"
#include "common/aglVertexAttribute.h"
#include "cull/aglViewFrustumCulling.h"
#include "utility/aglDebugTexturePage.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterObj.h"

namespace sead
{
class Camera;
class PerspectiveProjection;
class Projection;

namespace hostio
{
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl
{
class RenderBuffer;
class ShaderProgram;
class TextureData;
class TextureSampler;
}  // namespace agl

namespace agl::sdw
{

/**
 * Uniform block contents of one ambient occlusion sphere.
 */
struct SphereAo
{
    sead::Vector3f mViewPos;
    f32 mIntensity;
    f32 mRadiusSq;
    f32 mRangeInvSq;
};

/**
 * Uniform block contents of one directional occlusion sphere.
 */
struct SphereDo
{
    sead::Vector3f mViewPos;
    f32 mIntensity;
    sead::Vector3f mViewDir;
    f32 mRadius;
    f32 mLengthInv;
    f32 _24[3];
};

/**
 * A sphere that casts ambient occlusion and/or directional occlusion.
 */
class SphereOcclusion
{
public:
    SphereOcclusion()
        : mPos(sead::Vector3f::zero), mRadius(0.0f), mAoIntensity(1.0f), mDoIntensity(1.0f),
          mDir(sead::Vector3f::ey)
    {
    }

    void setParam(const sead::Vector3f& rPos, f32 radius, f32 intensity);
    void setDoParam(const sead::Vector3f& rDir, f32 length, f32 intensity);
    void storeUboStructAo(SphereAo* pDst, const sead::Matrix34f& rViewMtx) const;
    void storeUboStructDo(SphereDo* pDst, const sead::Matrix34f& rViewMtx) const;

    sead::Vector3f mPos;
    f32 mRadius;
    f32 mAoIntensity;
    f32 mDoIntensity;
    sead::Vector3f mDir;
    f32 mDoLength;
    sead::ListNode mAoNode;
    sead::ListNode mDoNode;
};
static_assert(sizeof(SphereOcclusion) == 0x48);

/**
 * Ambient/directional occlusion from analytic primitives (spheres).
 */
class PrimitiveOcclusion : public sead::hostio::Node
{
public:
    struct CreateArg
    {
        s32 mContextNum = 1;
        s32 mSphereNum = 256;
        s32 _8 = 256;
    };
    static_assert(sizeof(CreateArg) == 0xc);

    enum RequestType
    {
        cRequestType_Ao = 0,
        cRequestType_Do = 1,
        cRequestType_Both = 2,
    };

    /**
     * Per view data: culling, the view uniform block and derived camera values.
     */
    struct Context
    {
        cull::ViewFrustumCulling mCulling;
        UniformBlock mUbo;
        void* _2b0 = nullptr;
        f32 _2b8;
        sead::Matrix34f mViewMtx;
        sead::Matrix44f mViewProjMtx;
        sead::Vector2f mScreenOffset;
        f32 mDepthRatio;
        f32 mRange;
        f32 mRangeInv;
        f32 mTanHalfFovx;
        f32 mSignZ;
    };
    static_assert(sizeof(Context) == 0x348);

    PrimitiveOcclusion();
    ~PrimitiveOcclusion();

    virtual void init(const CreateArg& rArg);

    void initSphereAoUbo(sead::Buffer2<UniformBlock>* pBuffer);
    void initSphereDoUbo(sead::Buffer2<UniformBlock>* pBuffer);

    void setShaderSphereAo(const ShaderProgram* pProgram);
    void setShaderSphereDo(const ShaderProgram* pProgram);
    void setShaderMakeTableSphereDo(const ShaderProgram* pProgram);
    void clearRequest();
    void calcContext(s32 context, const sead::Camera& rCamera,
                     const sead::Projection& rProjection);
    void calcContext(s32 context, const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                     f32 near, f32 far, f32 fovy, f32 aspect, const sead::Vector2f& rOffset);
    void calcView(s32 context, const sead::Camera* pCamera,
                  const sead::PerspectiveProjection* pProjection);
    ShaderMode drawAo(s32 context, const TextureData& rNormal, const TextureData& rDepth,
                      bool isView, ShaderMode shaderMode) const;
    ShaderMode drawDo(s32 context, const TextureData& rNormal, const TextureData& rDepth,
                      bool isView, ShaderMode shaderMode) const;
    ShaderMode drawDebug(s32 context, ShaderMode shaderMode) const;
    ShaderMode drawPrecomputeSphereDo(s32 context, ShaderMode shaderMode) const;
    void requestSphereOpp(const sead::Vector3f& rPos, const sead::Vector3f& rDir, f32 radius,
                          f32 aoIntensity, f32 doIntensity, f32 doLength, RequestType type);

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    /**
     * Free running pool of sphere requests for the current frame.
     */
    struct RequestPool
    {
        /**
         * Restarts the pool at the beginning of a buffer.
         * @param pBuffer sphere buffer
         */
        void reset(SphereOcclusion* pBuffer)
        {
            mNum = 0;
            _4 = 0;
            mBuffer = pBuffer;
        }

        s32 mNum;
        s32 _4;
        SphereOcclusion* mBuffer;
    };

    /**
     * Double buffered request to rebake the directional occlusion table.
     */
    struct MakeTableRequest
    {
        bool isRequested() const { return mIsRequest[mIndex & 1]; }
        void requestNext() { mIsRequest[(mIndex + 1) & 1] = true; }

        void swap()
        {
            const s32 index = mIndex & 1;
            mIndex = index ^ 1;
            mIsRequest[index] = false;
        }

        bool mIsRequest[2] = {false, true};
        s32 mIndex = 0;
    };

    void calcContext_(s32 context);

    /**
     * Takes the next sphere of the pool and links it into the requested lists.
     */
    void requestSphere_(const sead::Vector3f& rPos, const sead::Vector3f& rDir, f32 radius,
                        f32 aoIntensity, f32 doIntensity, f32 doLength, RequestType type)
    {
        SphereOcclusion* pSphere = &mRequestPool.mBuffer[mRequestPool.mNum];
        pSphere->setParam(rPos, radius, aoIntensity);
        pSphere->setDoParam(rDir, doLength, doIntensity);
        mRequestPool.mNum++;

        if (type == cRequestType_Ao || type == cRequestType_Both)
        {
            mAoList.pushBack(pSphere);
        }

        if (type == cRequestType_Do || type == cRequestType_Both)
        {
            mDoList.pushBack(pSphere);
        }
    }

    CreateArg mArg;
    s32 mBufferIndex = 0;
    sead::Buffer<Context> mContexts;
    const ShaderProgram* mSphereAoProgram = nullptr;
    UniformBlockLocation mSphereAoContextLocation{"Context"};
    UniformBlockLocation mSphereAoViewLocation{"SphereAoView"};
    const ShaderProgram* mSphereDoProgram = nullptr;
    UniformBlockLocation mSphereDoContextLocation{"Context"};
    UniformBlockLocation mSphereDoViewLocation{"SphereDoView"};
    const ShaderProgram* mMakeTableSphereDoProgram = nullptr;
    TextureData* mSphereDoTable = nullptr;
    GPUMemVoidAddr mSphereDoTableBuffer;
    TextureSampler* mSphereDoTableSampler = nullptr;
    RenderBuffer* mSphereDoTableRenderBuffer = nullptr;
    MakeTableRequest mMakeTableRequest;
    f32 mDebugDoLength = 200.0f;
    f32 mDoOffset = 10.0f;
    f32 mDoEnvParam = 0.1f;
    utl::ParameterObj mParameterObj;
    utl::Parameter<f32> mConeDegreeMin;
    utl::Parameter<f32> mConeDegreeMax;
    utl::Parameter<f32> mConeDegree;
    sead::Buffer<SphereOcclusion> mSpheres;
    RequestPool mRequestPool = {0, {}, nullptr};
    sead::OffsetList<SphereOcclusion> mAoList;
    sead::OffsetList<SphereOcclusion> mDoList;
    sead::Buffer2<UniformBlock> mSphereAoUbo;
    sead::Buffer2<UniformBlock> mSphereDoUbo;
    VertexAttribute mSphereAttribute;
    VertexAttribute mConeAttribute;
    UniformBlock mEnvUbo;
    bool mIsDrawDebugSphere = false;
    bool mIsDrawDebugAo = false;
    bool mIsDrawDebugDo = false;
    bool mIsRequestDebugSphere = false;
    sead::Vector3f mDebugPos{1050.0f, -50.0f, 700.0f};
    sead::Vector3f mDebugDir{1.0f, 1.0f, 0.0f};
    f32 mDebugRadius = 100.0f;
    f32 mDebugIntensity = 1.0f;
    f32 mAoRangeScale = 1.0f;
    utl::DebugTexturePage mDebugTexturePage;
};

static_assert(sizeof(PrimitiveOcclusion) == 0x888);

}  // namespace agl::sdw
