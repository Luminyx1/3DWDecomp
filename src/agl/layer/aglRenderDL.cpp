#include "layer/aglRenderDL.h"

namespace agl::lyr {

/**
 * Constructs an empty render display list.
 */
RenderDL::RenderDL() : mPriority(0), mLayer(nullptr), mCoreId(0) {}

/**
 * Destroys the render display list.
 */
RenderDL::~RenderDL() = default;

/**
 * Copies the display list and its render information to another render display list.
 * @param pDst destination render display list
 */
void RenderDL::copyToRenderDL(RenderDL* pDst) const
{
    copyTo(pDst);
    pDst->mBeginTime = mBeginTime;
    pDst->mEndTime = mEndTime;
    pDst->mPriority = mPriority;
    pDst->mCoreId = mCoreId;
    pDst->mLayer = mLayer;
}

/**
 * Compares two render display lists by priority.
 * @param pA first render display list
 * @param pB second render display list
 * @return difference of the priorities
 */
s32 RenderDL::compare(const RenderDL* pA, const RenderDL* pB)
{
    return pA->mPriority - pB->mPriority;
}

/**
 * Dumps information about the display list (no-op in release builds).
 */
void RenderDL::dumpInfo() const {}

}  // namespace agl::lyr
