#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <gfx/seadCamera.h>
#include <gfx/seadColor.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <prim/seadSafeString.h>

#include "layer/aglRenderStep.h"

namespace sead {
class Controller;
class LogicalFrameBuffer;
namespace hostio {
class Context;
class NodeEvent;
class PropertyEvent;
class Reflexible;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
}

namespace agl::lyr {

class DrawMethod;
class LayerJob;
class RenderDisplay;
class Renderer;
class RenderInfo;

class Layer : public sead::IDisposer, public sead::hostio::Node {
    SEAD_RTTI_BASE(Layer)
    friend class Renderer;
    friend class RenderDisplay;
    friend class RenderInfo;
    friend class LayerJob;

public:
    enum Flag {
        cFlag_Visible = 1 << 0,
        cFlag_Enable = 1 << 1,
        cFlag_ListDirty = 1 << 5,
        cFlag_Initialized = 1 << 6,
    };

    struct DebugInfo {
        DebugInfo();

        s32 mMessageTimer;
        sead::LookAtCamera mCamera;
        sead::LookAtCamera mDebugCamera;
        f32 mAtDist;
        f32 mTwist;
        sead::Vector3f mBoundAt;
        u8 _dc[0xc];
        f32 mNear;
        f32 mFar;
        f32 mFovyDeg;
        sead::Vector2f mOffset;
        f32 mAspect;
        sead::Projection* mProjection;
        sead::PerspectiveProjection mPerspectiveProjection;
        sead::DirectProjection mDirectProjection;
        sead::OrthoProjection mOrthoProjection;
        sead::Vector2f mViewportPos;
        sead::Vector2f mViewportSize;
        f32 _380;
        f32 _384;
        sead::Vector3f mRotateCenter;
        f32 mRotateDist;
        f32 mRotateYDeg;
        f32 mRotateXDeg;
        sead::Buffer<sead::FixedSafeString<512>> mInfoText;
        u32 mFrame;
        u8 mPointerBuffer[0xc];
        f32 mRandomRange;
        sead::BitFlag32 mFlag;
        sead::FixedSafeString<256> mText;
    };
    static_assert(sizeof(DebugInfo) == 0x4e0);

    Layer();
    ~Layer() override;

    virtual s32 getRenderStepNum() const = 0;
    virtual sead::SafeString getRenderStepName(s32 index) const = 0;
    virtual void initializeImpl(sead::Heap* pHeap) {}
    virtual void calcImpl() {}
    virtual void postCalcCommandImpl() {}
    virtual void preDrawImpl(const RenderInfo& rInfo) const {}
    virtual void preDrawRenderStepImpl(const RenderInfo& rInfo) const {}
    virtual void preDrawRenderStepMethodImpl(const RenderInfo& rInfo,
                                             const DrawMethod& rMethod) const
    {
    }
    virtual void postDrawRenderStepMethodImpl(const RenderInfo& rInfo,
                                              const DrawMethod& rMethod) const
    {
    }
    virtual void postDrawRenderStepImpl(const RenderInfo& rInfo) const {}
    virtual void postDrawImpl(const RenderInfo& rInfo) const {}
    virtual bool isRenderStepGPUCalc(s32 index) const { return false; }
    virtual bool isRenderStepNoDependency(s32 index) const { return false; }
    virtual bool isForceInvisible() const { return false; }
    virtual bool isForceDisableClear() const { return false; }
    virtual void calcJobWeight() {}

    f32 getDebugCameraAtDist() const;
    void resetBoundDebugCameraAt();
    void setBoundDebugCameraAt(const sead::Vector3f& rAt);
    DrawMethod* pushBackDrawMethod(u32 renderStep, DrawMethod* pMethod);
    DrawMethod* pushBackDrawMethod(DrawMethod* pMethod);
    s32 removeDrawMethodByObject(const void* pObject);
    s32 removeDrawMethod(const DrawMethod* pMethod);
    s32 removeDrawMethod(u32 renderStep, const DrawMethod* pMethod);
    void clearDrawMethod();
    const sead::Camera* getRenderCamera() const;
    const sead::Projection* getRenderProjection() const;
    sead::LogicalFrameBuffer* getLogicalFrameBuffer() const;
    void setLastDisplayListSize(u64 size) const;
    void setEnable(bool enable);
    void setVisible(bool visible);
    void setDisplayType(s32 displayType);
    bool isRenderingEnabled() const;
    void drawDebugCamera(DrawContext* pDrawContext) const;
    void setDebugCameraAtDist(f32 dist);
    void setDebugCameraTwist(f32 twist);
    f32 getDebugCameraTwist();
    sead::LookAtCamera* getDebugCameraPtr();
    void setDebugNear(f32 near);
    void setDebugFar(f32 far);
    void setDebugFovyDeg(f32 fovyDeg);

    void genMessage(sead::hostio::Context* pContext);
    void genMessageCamera(sead::hostio::Context* pContext);
    void genMessageProjection(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
    void listenPropertyEventCamera(sead::hostio::Reflexible* pReflexible,
                                   const sead::hostio::PropertyEvent* pEvent);
    void listenPropertyEventProjection(sead::hostio::Reflexible* pReflexible,
                                       const sead::hostio::PropertyEvent* pEvent);
    void listenNodeEvent(const sead::hostio::NodeEvent* pEvent);

    bool isVisible() const { return mFlag.isOn(cFlag_Visible); }
    bool isEnable() const { return mFlag.isOn(cFlag_Enable); }
    s32 getLayerIndex() const { return mLayerIndex; }
    const sead::Viewport& getViewport() const { return mViewport; }
    const sead::SafeString& getName() const { return mName; }

    static sead::DirectCamera sCameraIdentity;
    static sead::OrthoProjection sProjectionIdentity;

protected:
    void initialize_(sead::Heap* pHeap);
    void updateDebugInfo_(u32 flag);
    void copyCurrentCameraToDebugCamera_();
    void copyCurrentProjectionToDebugProjection_();
    void calc_(const sead::Controller* pController, s32 displayIndex, bool isDebugCamera);
    void postCalcCommand_();
    void clearColor_(const RenderInfo& rInfo) const;
    void drawRenderStep_(const RenderInfo& rInfo) const;
    void drawDebugInfo_(const RenderInfo& rInfo) const;

    Renderer* mRenderer = nullptr;
    sead::Viewport mDisplayViewport;
    sead::Viewport mViewport;
    sead::Camera* mCamera = nullptr;
    sead::Projection* mProjection = nullptr;
    sead::BitFlag16 mFlag{0x483};
    sead::BitFlag16 mDebugFlag{0xc2};
    u8 mDebugDrawFlag = 0;
    u8 _95 = 0;
    s8 _96 = 0;
    u8 _97;
    s8 mDisplayType = 0;
    s8 mDisplayTypeOverride = -1;
    u8 _9a = 0;
    u8 mClearFlag = 0;
    s32 mLayerIndex = 0;
    s32 _a0 = 0;
    sead::Color4f mClearColor{0.0f, 0.0f, 0.0f, 0.0f};
    f32 mClearDepth = 1.0f;
    sead::Buffer<RenderStep> mRenderStep;
    mutable u32 mLastDisplayListSize = 0;
    LayerJob* mJobDraw;
    LayerJob* mJobSubDraw;
    LayerJob* mJobGPUCalc;
    sead::FixedSafeString<256> mName;
    DebugInfo* mDebugInfo;
};
static_assert(sizeof(Layer) == 0x208);

}  // namespace agl::lyr
