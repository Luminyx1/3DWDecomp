#include "Library/Screen/ScreenCaptureExecutor.hpp"

#include "Project/Screen/ScreenCapture.hpp"

namespace al {
/**
 * Creates a screen capture executor.
 * @param captureNum number of capture slots
 */
ScreenCaptureExecutor::ScreenCaptureExecutor(s32 captureNum) {
    mCaptureInfos.allocBuffer(captureNum, nullptr);
    for (s32 i = 0; i < mCaptureInfos.capacity(); i++) {
        mCaptureInfos.pushBack(new CaptureInfo());
    }
}

/**
 * Destroys the executor and its screen captures.
 */
ScreenCaptureExecutor::~ScreenCaptureExecutor() {
    for (s32 i = 0; i < mCaptureInfos.size(); i++) {
        CaptureInfo* info = mCaptureInfos.at(i);
        if (info->screenCapture) {
            delete info->screenCapture;
            info->screenCapture = nullptr;
        }
        delete info;
    }
}

/**
 * Creates the screen capture of a slot.
 * @param width capture width
 * @param height capture height
 * @param index capture slot
 */
void ScreenCaptureExecutor::createScreenCapture(s32 width, s32 height, s32 index) {
    mCaptureInfos.unsafeAt(index)->screenCapture = new ScreenCapture(width, height);
}

/**
 * Draws the capture of a slot and captures the frame buffer if requested.
 * @param pDrawContext draw context
 * @param pRenderBuffer render buffer
 * @param index capture slot
 */
void ScreenCaptureExecutor::tryCaptureAndDraw(agl::DrawContext* pDrawContext,
                                              const agl::RenderBuffer* pRenderBuffer, s32 index) {
    if (mCaptureInfos.unsafeAt(index)->isDraw) {
        mCaptureInfos.unsafeAt(index)->screenCapture->drawCaptureImage(pDrawContext,
                                                                       pRenderBuffer);
    }
    tryCapture(pDrawContext, pRenderBuffer, index);
}

/**
 * Checks whether the capture of a slot is drawn.
 * @param index capture slot
 * @return whether it is drawn
 */
bool ScreenCaptureExecutor::isDraw(s32 index) const {
    return mCaptureInfos.unsafeAt(index)->isDraw;
}

/**
 * Draws the capture of a slot.
 * @param pDrawContext draw context
 * @param pRenderBuffer render buffer
 * @param index capture slot
 */
void ScreenCaptureExecutor::draw(agl::DrawContext* pDrawContext,
                                 const agl::RenderBuffer* pRenderBuffer, s32 index) const {
    mCaptureInfos.unsafeAt(index)->screenCapture->drawCaptureImage(pDrawContext, pRenderBuffer);
}

/**
 * Captures the frame buffer into a slot if requested.
 * @param pDrawContext draw context
 * @param pRenderBuffer render buffer
 * @param index capture slot
 * @return whether the frame buffer was captured
 */
bool ScreenCaptureExecutor::tryCapture(agl::DrawContext* pDrawContext,
                                       const agl::RenderBuffer* pRenderBuffer, s32 index) {
    CaptureInfo* info = mCaptureInfos.at(index);
    if (!info->isActiveRequest) {
        return false;
    }
    info->screenCapture->copyImageFromFrameBuffer(pDrawContext, pRenderBuffer);
    info->isActiveRequest = false;
    info->isDraw = true;
    mIsCaptured = true;
    return true;
}

/**
 * Requests a capture of the frame buffer into a slot.
 * @param isDraw whether the old capture stays drawn until the new one is taken
 * @param index capture slot
 * @param isBlur whether the capture is blurred
 */
void ScreenCaptureExecutor::requestCapture(bool isDraw, s32 index, bool isBlur) {
    mCaptureInfos.unsafeAt(index)->isActiveRequest = true;
    mCaptureInfos.unsafeAt(index)->screenCapture->enableBlur(isBlur);
    if (isDraw) {
        mCaptureInfos.unsafeAt(index)->isDraw = false;
    }
}

/**
 * Initializes the blur of a slot.
 * @param index capture slot
 * @param rFrameBuffer frame buffer
 * @param rViewport viewport
 * @param scale reduce scale of the blur
 */
void ScreenCaptureExecutor::initBlur(s32 index, const sead::LogicalFrameBuffer& rFrameBuffer,
                                     const sead::Viewport& rViewport,
                                     agl::utl::ImageFilter2D::ReduceScale scale) {
    mCaptureInfos.unsafeAt(index)->screenCapture->initBlur(rFrameBuffer, rViewport, scale);
}

/**
 * Enables or disables the blur of a slot.
 * @param index capture slot
 * @param isEnable whether the blur is enabled
 */
void ScreenCaptureExecutor::enableBlur(s32 index, bool isEnable) {
    mCaptureInfos.unsafeAt(index)->screenCapture->enableBlur(isEnable);
}

/**
 * Starts drawing the capture of a slot.
 * @param index capture slot
 * @param isBlur whether the capture is blurred
 */
void ScreenCaptureExecutor::onDraw(s32 index, bool isBlur) {
    mCaptureInfos.unsafeAt(index)->isDraw = true;
    mCaptureInfos.unsafeAt(index)->screenCapture->enableBlur(isBlur);
}

/**
 * Stops drawing the capture of a slot.
 * @param index capture slot
 */
void ScreenCaptureExecutor::offDraw(s32 index) {
    mCaptureInfos.unsafeAt(index)->isDraw = false;
    mCaptureInfos.unsafeAt(index)->screenCapture->enableBlur(false);
}

/**
 * Cancels the capture request of a slot and stops drawing it.
 * @param index capture slot
 */
void ScreenCaptureExecutor::resetRequest(s32 index) {
    mCaptureInfos.unsafeAt(index)->isDraw = false;
    mCaptureInfos.unsafeAt(index)->isActiveRequest = false;
    mCaptureInfos.unsafeAt(index)->screenCapture->enableBlur(false);
}

/**
 * Checks whether a slot has a pending capture request.
 * @param index capture slot
 * @return whether a capture is requested
 */
bool ScreenCaptureExecutor::isActiveRequest(s32 index) const {
    return mCaptureInfos.unsafeAt(index)->isActiveRequest;
}

/**
 * Stops drawing every capture.
 */
void ScreenCaptureExecutor::offDrawGlobal() {
    for (s32 i = 0; i < mCaptureInfos.capacity(); i++) {
        mCaptureInfos.unsafeAt(i)->isDraw = false;
    }
    mIsCaptured = false;
}

/**
 * Checks whether any slot has a pending capture request.
 * @return whether a capture is requested
 */
bool ScreenCaptureExecutor::isAnyActiveRequest() const {
    for (s32 i = 0; i < mCaptureInfos.capacity(); i++) {
        if (mCaptureInfos.unsafeAt(i)->isActiveRequest) {
            return true;
        }
    }
    return false;
}
}  // namespace al
