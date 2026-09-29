#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <container/seadSafeArray.h>
#include <gfx/seadColor.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <math/seadVector.h>
#include <mc/seadJobQueue.h>
#include <prim/seadBitFlag.h>
#include <prim/seadSafeString.h>
#include <thread/seadCriticalSection.h>

#include "layer/aglLayerJob.h"
#include "layer/aglRenderDLBuffer.h"

namespace sead {
class Controller;
class Heap;
class LogicalFrameBuffer;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
}

namespace agl::lyr {

class DrawMethod;
class Layer;
class RenderDisplay;
class RenderInfo;

class Renderer : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(Renderer)

public:
    static constexpr s32 cDisplayMax = 2;
    static constexpr s32 cJobQueueNum = 4;

    enum CallbackType {
        cCallbackType_0 = 0,
        cCallbackType_1 = 1,
        cCallbackType_2 = 2,
        cCallbackType_3 = 3,
        cCallbackType_4 = 4,
    };

    enum DebugCameraState {
        cDebugCameraState_None = 0,
        cDebugCameraState_1 = 1,
        cDebugCameraState_2 = 2,
    };

    enum DisplayControl {
        cDisplayControl_0 = 0,
        cDisplayControl_1 = 1,
    };

    class CreateArg {
    public:
        CreateArg();
        virtual ~CreateArg() {}

        void setDisplayInfo(s32 index, const sead::Vector2i& rVirtualSize,
                            const sead::Vector2i& rPhysicalSize);

        sead::SafeArray<sead::Vector2i, cDisplayMax> mVirtualSize;
        sead::SafeArray<sead::Vector2i, cDisplayMax> mPhysicalSize;
        s32 mDisplayNum = 2;
        s32 mLayerNum = 32;
        s32 mRenderDLNum = 32;
        u64 mDLBufferSize = 0x400000;
        u64 mDLControlMemorySize = 0x100000;
        s32 mDLMultiBufferNum = 2;
        u32 mFlag = 0;
        s32 mMultiSampleType = 0;
        sead::SafeArray<sead::LogicalFrameBuffer*, cDisplayMax> mLogicalFrameBuffer;
    };
    static_assert(sizeof(CreateArg) == 0x68);

    Renderer();
    virtual ~Renderer();

    void initialize(const CreateArg& rArg, sead::Heap* pHeap, sead::Heap* pDebugHeap);
    static sead::SafeString getDisplayName(s32 index);
    void calc(bool b);
    void calcCommand(bool b, u32 displayMask);
    void postCalcCommand();
    void calcGPU(bool b) const;
    bool draw(DrawContext* pDrawContext, s32 displayIndex) const;
    void setCallback(CallbackType type, DrawMethod* pMethod);
    void removeDrawMethodByObject(const void* pObject);
    void removeDrawMethod(const DrawMethod* pMethod);
    bool removeLayer(Layer* pLayer);
    s32 searchLayerIndex(const Layer* pLayer) const;
    s32 searchEmptyLayerIndexFromFront() const;
    s32 searchEmptyLayerIndexFromBack() const;
    void setFrameBufferClear(u32 clearFlag, const sead::Color4f& rColor, f32 depth, u32 stencil,
                             s32 displayIndex);
    void changeDebugCameraStateNext();
    void changeDebugCameraState(DebugCameraState state);
    void resetDebugCamera();
    void setDisplayControl(DisplayControl control);
    void drawStatus(const RenderInfo& rInfo) const;
    void dumpDisplayListInfo() const;
    void dumpRawDisplayList() const;

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    template <typename T>
    T* createLayer(s32 layerIndex, const sead::SafeString& rName, s32 displayType,
                   sead::Heap* pHeap)
    {
        T* pLayer = new (pHeap) T();
        initLayer_(pLayer, layerIndex, rName, displayType, pHeap);
        return pLayer;
    }

    RenderDisplay* getDisplay(s32 index) const { return mDisplay[index]; }

private:
    friend class Layer;

    bool isDisplayList_() const;
    void initLayer_(Layer* pLayer, s32 layerIndex, const sead::SafeString& rName,
                    s32 displayType, sead::Heap* pHeap);
    void lockLayerList_();
    void unlockLayerList_();

    s32 mMultiSampleType = 0;
    sead::Buffer<RenderDisplay*> mDisplay;
    sead::Buffer<Layer*> mLayer;
    sead::Buffer<LayerJob> mJobDraw;
    sead::Buffer<LayerJob> mJobSubDraw;
    sead::Buffer<LayerJob> mJobGPUCalc;
    RenderDLBuffer mDLBuffer;
    s32 mColorFormat = 29;
    s32 mDepthFormat = 60;
    mutable sead::BitFlag32 mFlag{0xc2c};
    u8 mDisplayListMode = 0;
    u8 mDebugFlag = 0;
    u8 mRenderDLNum = 0;
    void* _1c0[5];
    sead::FixedSizeJQ mJobQueue[cJobQueueNum];
    s32 _508;
    s32 _50c;
    s32 _510;
    s32 _514;
    mutable sead::CriticalSection mLayerListCS;
    DebugCameraState mDebugCameraState = cDebugCameraState_None;
    s32 mDebugCameraControllerIndex = 0;
    const sead::Controller* mDebugCameraController = nullptr;
    f32 _568 = 1.0f;
    f32 _56c = 0.0f;
    f32 _570 = 0.0f;
    f32 _574 = 0.9f;
    u8 mDisplayControl = 0;
    u8 mDebugCameraMessageTimer = 0;
    u8 mFrameCounter = 0;
    u8 mCalcInterval = 1;
    u8 mJobGranularity = 1;
    u8 mColorTextureNum = 0;
    u8 mDepthTextureNum = 1;
    u8 _57f[0x11];
};
static_assert(sizeof(Renderer) == 0x590);

}  // namespace agl::lyr
