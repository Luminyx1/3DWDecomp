#pragma once

#include <basis/seadTypes.h>

#include "layer/aglLayerEnum.h"

namespace sead {
class Camera;
class Projection;
class Viewport;
}  // namespace sead

namespace agl {
class DrawContext;
class RenderBuffer;
}  // namespace agl

namespace agl::lyr {

class Layer;

class RenderInfo {
    friend class Layer;

public:
    RenderInfo(DrawContext* pDrawContext, s32 displayIndex, FrameworkType frameworkType,
               const RenderBuffer* pFrameBuffer, bool isDrawDebug, const Layer* pLayer);
    RenderInfo(DrawContext* pDrawContext, s32 displayIndex, const RenderBuffer* pFrameBuffer);

    const RenderBuffer* getFrameBuffer() const;
    void bindFrameBufferAndApplyViewport(DrawContext* pDrawContext) const;
    void setUpPrimitiveRenderer() const;

    s32 getRenderStep() const { return mRenderStep; }
    void setRenderStep(s32 step) { mRenderStep = step; }
    DrawContext* getDrawContext() const { return mDrawContext; }
    const Layer* getLayer() const { return mLayer; }
    const sead::Camera* getCamera() const { return mCamera; }
    const sead::Projection* getProjection() const { return mProjection; }
    const sead::Viewport* getViewport() const { return mViewport; }

private:
    s32 mRenderStep;
    FrameworkType mFrameworkType;
    u8 mDisplayIndex;
    const RenderBuffer* mFrameBuffer;
    const Layer* mLayer;
    s32 mLayerIndex;
    const sead::Camera* mCamera;
    const sead::Projection* mProjection;
    const sead::Viewport* mViewport;
    bool mIsDrawDebug;
    DrawContext* mDrawContext;
};
static_assert(sizeof(RenderInfo) == 0x50);

}  // namespace agl::lyr
