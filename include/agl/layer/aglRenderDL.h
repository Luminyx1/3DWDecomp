#pragma once

#include <basis/seadTypes.h>
#include <time/seadTickTime.h>

#include "common/aglDisplayList.h"

namespace agl::lyr {

class Layer;
class RenderDLBuffer;
class RenderDisplay;

class RenderDL : public DisplayList {
    friend class RenderDLBuffer;
    friend class RenderDisplay;

public:
    RenderDL();
    ~RenderDL() override;

    void copyToRenderDL(RenderDL* pDst) const;
    static s32 compare(const RenderDL* pA, const RenderDL* pB);
    void dumpInfo() const;

    s32 getPriority() const { return mPriority; }
    const Layer* getLayer() const { return mLayer; }
    void setLayer(const Layer* pLayer) { mLayer = pLayer; }

private:
    s32 mPriority;
    const Layer* mLayer;
    sead::TickTime mBeginTime;
    sead::TickTime mEndTime;
    u8 mCoreId;
};
static_assert(sizeof(RenderDL) == 0x288);

}  // namespace agl::lyr
