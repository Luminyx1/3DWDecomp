#pragma once

#include <container/seadRingBuffer.h>
#include <container/seadSafeArray.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadBitFlag.h>

#include "performance/aglGPUTimeStamp.h"

namespace sead {
class FrameBuffer;
class TextWriter;
class Viewport;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
}

namespace agl::fctr {

class GPUStressChecker : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(GPUStressChecker)

public:
    struct StampObj {
        enum Flag {
            cFlag_Started = 1 << 0,
            cFlag_Ended = 1 << 1,
        };

        void initialize(sead::Heap* pHeap);

        perf::GPUTimeStamp& getStamp(perf::GPUTimeStampArray::Type type)
        {
            return mArray.getStamp(type);
        }

        perf::GPUTimeStampArray mArray;
        sead::BitFlag32 mFlags = 0;
    };
    static_assert(sizeof(StampObj) == 0xc0);

    struct History {
        f32 mLoad;
        f32 mAverage3;
        f32 mAverage5;
        f32 mSpeed;
        f32 mAccel;
        f32 mPredict;
    };
    static_assert(sizeof(History) == 0x18);

    struct Info {
        f32 mTime;
        f32 mLoad;
        sead::RingBuffer<History> mHistory;
    };
    static_assert(sizeof(Info) == 0x20);

    enum Flag {
        cFlag_Enable = 1 << 0,
        cFlag_Measured = 1 << 1,
        cFlag_DrawDebug = 1 << 3,
    };

    GPUStressChecker();
    virtual ~GPUStressChecker() {}

    void initialize(sead::Heap* pHeap);
    void calc();
    void start(DrawContext* pDrawContext) const;
    void end(DrawContext* pDrawContext) const;
    void drawDebug(DrawContext* pDrawContext, const sead::FrameBuffer& rFrameBuffer,
                   const sead::Viewport& rViewport) const;
    void drawDebug(sead::TextWriter* pTextWriter, f32 scale) const;
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    static void pushHistory_(sead::RingBuffer<History>* pHistory, const History& rEntry)
    {
        if (pHistory->size() >= pHistory->capacity())
        {
            History oldest;
            pHistory->popBack(&oldest);
        }
        pHistory->pushBackwards(rEntry);
    }

    sead::SafeArray<Info, 2> mInfo;
    sead::BitFlag32 mFlags;
    u32 mInfoNum;
    u32 mInfoIndex;
    StampObj* mStampObj[3];
    f32 mDrawThreshold;
    f32 mTextScale;
    f32 mDrawPosX;
    f32 mDrawPosY;
};
static_assert(sizeof(GPUStressChecker) == 0xa0);

}  // namespace agl::fctr
