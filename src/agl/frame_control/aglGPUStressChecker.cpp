#include "frame_control/aglGPUStressChecker.h"

#include <devenv/seadFontMgr.h>
#include <gfx/seadFrameBuffer.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadTextWriter.h>
#include <gfx/seadViewport.h>
#include <limits>
#include <math/seadMatrix.h>
#include <prim/seadSafeString.h>

#include "common/aglDrawContext.h"
#include "utility/aglDevTools.h"

namespace agl::fctr {

SEAD_SINGLETON_DISPOSER_IMPL(GPUStressChecker)

/**
 * Allocates one timestamp per type and clears the measurement flags.
 * @param pHeap heap to allocate from
 */
void GPUStressChecker::StampObj::initialize(sead::Heap* pHeap)
{
    mArray.allocBuffer(1, pHeap);
    mFlags = 0;
}

/**
 * Constructs a disabled stress checker with the default debug draw settings.
 */
GPUStressChecker::GPUStressChecker()
    : mFlags(0), mInfoNum(1), mInfoIndex(0), mDrawThreshold(95.0f), mTextScale(2.0f),
      mDrawPosX(0.03f), mDrawPosY(0.05f)
{
}

/**
 * Enables the checker and allocates its timestamp objects and load histories.
 * @param pHeap heap to allocate from
 */
void GPUStressChecker::initialize(sead::Heap* pHeap)
{
    mFlags = cFlag_Enable;

    for (auto& pStampObj : mStampObj)
    {
        pStampObj = new (pHeap) StampObj();
        pStampObj->initialize(pHeap);
    }

    for (s32 i = 0; i < 2; i++)
    {
        Info& rInfo = mInfo[i];
        rInfo.mTime = 0.0f;
        rInfo.mLoad = 0.0f;
        rInfo.mHistory.tryAllocBuffer(20, pHeap);
        rInfo.mHistory.clear();
    }
}

/**
 * Reads the oldest timestamps, updates the GPU load and its history, and rotates the stamps.
 */
void GPUStressChecker::calc()
{
    if (!mFlags.isOn(cFlag_Enable))
    {
        for (s32 i = 0; i < 2; i++)
        {
            mInfo[i].mTime = 0.0f;
            mInfo[i].mLoad = 0.0f;
        }
        return;
    }

    mInfoIndex = mInfoIndex + 1 < mInfoNum ? mInfoIndex + 1 : 0;

    StampObj* pStampObj = mStampObj[0];
    Info& rInfo = mInfo[mInfoIndex];
    if (pStampObj->mFlags.isOnAll(StampObj::cFlag_Started | StampObj::cFlag_Ended))
    {
        u64 end = pStampObj->getStamp(perf::GPUTimeStampArray::cType_End).get();
        u64 begin = pStampObj->getStamp(perf::GPUTimeStampArray::cType_Begin).get();
        rInfo.mTime = (end - begin) * 0.001f;
        rInfo.mLoad = rInfo.mTime * 100.0f / 16666.666f;
        pStampObj->mFlags = 0;
        mFlags.set(cFlag_Measured);
    }
    else
    {
        pStampObj->mFlags = 0;
        mFlags.reset(cFlag_Measured);
    }

    mStampObj[0] = mStampObj[1];
    mStampObj[1] = mStampObj[2];
    mStampObj[2] = pStampObj;

    sead::RingBuffer<History>& rHistory = rInfo.mHistory;
    if (rHistory.size() < 3)
    {
        f32 load = rInfo.mLoad;
        History history = {load, load, load, 0.0f, 0.0f, load};
        pushHistory_(&rHistory, history);
    }
    else
    {
        f32 load = rInfo.mLoad;
        f32 average3 = load;
        f32 average5 = load;
        if (rHistory.size() >= 5)
        {
            f32 sum3 = load + rHistory(0).mLoad + rHistory(1).mLoad;
            f32 sum5 = sum3 + rHistory(2).mLoad + rHistory(3).mLoad;
            average3 = sum3 / 3.0f;
            average5 = sum5 / 5.0f;
        }
        const History& rPrev = rHistory(0);
        f32 speed = average3 - rPrev.mAverage3;
        f32 accel = speed - rPrev.mSpeed;
        History history = {load, average3, average5, speed, accel,
                           (speed + load) + (speed + accel)};
        pushHistory_(&rHistory, history);
    }
}

/**
 * Records the begin timestamps of the current frame.
 * @param pDrawContext draw context to record into
 */
void GPUStressChecker::start(DrawContext* pDrawContext) const
{
    if (!mFlags.isOn(cFlag_Enable))
    {
        return;
    }

    StampObj* pStampObj = mStampObj[1];
    pStampObj->getStamp(perf::GPUTimeStampArray::cType_Top).startTop(pDrawContext);
    pStampObj->getStamp(perf::GPUTimeStampArray::cType_Begin).startBottom(pDrawContext);
    pStampObj->mFlags.set(StampObj::cFlag_Started);
}

/**
 * Records the end timestamp of the current frame.
 * @param pDrawContext draw context to record into
 */
void GPUStressChecker::end(DrawContext* pDrawContext) const
{
    if (!mFlags.isOn(cFlag_Enable))
    {
        return;
    }

    StampObj* pStampObj = mStampObj[1];
    pStampObj->getStamp(perf::GPUTimeStampArray::cType_End).startBottom(pDrawContext);
    pStampObj->mFlags.set(StampObj::cFlag_Ended);
}

/**
 * Draws the GPU load as text on a black box when it exceeds the draw threshold.
 * @param pDrawContext draw context to draw with
 * @param rFrameBuffer frame buffer to draw into
 * @param rViewport unused viewport
 */
void GPUStressChecker::drawDebug(DrawContext* pDrawContext, const sead::FrameBuffer& rFrameBuffer,
                                 const sead::Viewport& rViewport) const
{
    if (!mFlags.isOnAll(cFlag_Enable | cFlag_DrawDebug))
    {
        return;
    }

    const Info& rInfo = mInfo[mInfoIndex];
    if (rInfo.mLoad < mDrawThreshold)
    {
        return;
    }

    sead::FormatFixedSafeString<64> text("GPU:%3.1f%%", rInfo.mLoad);
    sead::TextWriter::setupGraphics(pDrawContext);
    sead::TextWriter writer(pDrawContext);
    writer.setScale(sead::Vector2f(mTextScale, mTextScale));
    writer.setWrapWidth(std::numeric_limits<f32>::infinity());

    const sead::BoundBox2f& rArea = rFrameBuffer.getPhysicalArea();
    f32 width = rArea.getSizeX();
    f32 height = rArea.getSizeY();
    f32 length = text.calcLength();
    rFrameBuffer.bind(pDrawContext);

    sead::Viewport viewport(width * mDrawPosX, height * mDrawPosY,
                            length * (writer.getFont()->getWidth() * writer.getScale().x),
                            writer.getFont()->getHeight() * writer.getScale().y);
    viewport.apply(pDrawContext, rFrameBuffer);

    {
        sead::GraphicsContext graphicsContext;
        graphicsContext.setDepthEnable(false, false);
        graphicsContext.setBlendEnable(true);
        graphicsContext.setColorMask(true, true, true, true);
        graphicsContext.apply(pDrawContext);
    }
    {
        sead::Matrix34f mtx;
        mtx.makeS(2.0f, 2.0f, 2.0f);
        utl::DevTools::drawColorQuad(pDrawContext, sead::Color4f::cBlack, mtx,
                                     sead::Matrix44f::ident);
    }
    pDrawContext->changeShaderMode(ShaderMode(0), ShaderOptimizeType(0));

    writer.setViewport(&viewport);
    writer.beginDraw();
    writer.setColor(sead::Color4f::cWhite);
    writer.setCursorFromTopLeft(sead::Vector2f::zero);
    writer.printf(text.cstr());
    writer.endDraw();
}

/**
 * Prints the GPU load with a text writer.
 * @param pTextWriter text writer to print with
 * @param scale text scale
 */
void GPUStressChecker::drawDebug(sead::TextWriter* pTextWriter, f32 scale) const
{
    if (!mFlags.isOn(cFlag_Enable))
    {
        return;
    }

    const Info& rInfo = mInfo[mInfoIndex];
    pTextWriter->setScale(sead::Vector2f(scale, scale));
    pTextWriter->printf("[GPUStressChecker] Stress(%%): %3.1f \n", rInfo.mLoad);
}

/**
 * Does nothing.
 * @param pContext unused
 */
void GPUStressChecker::genMessage(sead::hostio::Context* pContext) {}

/**
 * Does nothing.
 * @param pEvent unused
 */
void GPUStressChecker::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

}  // namespace agl::fctr
