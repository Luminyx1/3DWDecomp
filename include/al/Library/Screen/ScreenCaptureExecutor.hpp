#pragma once

#include <utility/aglImageFilter2D.h>
#include <container/seadPtrArray.h>

namespace agl {
class DrawContext;
class RenderBuffer;
}  // namespace agl

namespace sead {
class LogicalFrameBuffer;
class Viewport;
}  // namespace sead

namespace al {
class ScreenCapture;

class ScreenCaptureExecutor {
public:
    struct CaptureInfo {
        ScreenCapture* screenCapture = nullptr;
        bool isActiveRequest = false;
        bool isDraw = false;
    };

    ScreenCaptureExecutor(s32 captureNum);
    virtual ~ScreenCaptureExecutor();

    void createScreenCapture(s32 width, s32 height, s32 index);
    void tryCaptureAndDraw(agl::DrawContext* pDrawContext, const agl::RenderBuffer* pRenderBuffer,
                           s32 index);
    bool isDraw(s32 index) const;
    void draw(agl::DrawContext* pDrawContext, const agl::RenderBuffer* pRenderBuffer,
              s32 index) const;
    bool tryCapture(agl::DrawContext* pDrawContext, const agl::RenderBuffer* pRenderBuffer,
                    s32 index);
    void requestCapture(bool isDraw, s32 index, bool isBlur);
    void initBlur(s32 index, const sead::LogicalFrameBuffer& rFrameBuffer,
                  const sead::Viewport& rViewport, agl::utl::ImageFilter2D::ReduceScale scale);
    void enableBlur(s32 index, bool isEnable);
    void onDraw(s32 index, bool isBlur);
    void offDraw(s32 index);
    void resetRequest(s32 index);
    bool isActiveRequest(s32 index) const;
    void offDrawGlobal();
    bool isAnyActiveRequest() const;

private:
    sead::PtrArray<CaptureInfo> mCaptureInfos;
    bool mIsCaptured = false;
};
}  // namespace al
