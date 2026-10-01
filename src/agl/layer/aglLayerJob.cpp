#include "layer/aglLayerJob.h"

#include "common/aglDrawContext.h"
#include "layer/aglLayer.h"
#include "layer/aglRenderDLBuffer.h"
#include "layer/aglRenderDisplay.h"

namespace agl::lyr {

/**
 * Constructs an uninitialized layer job.
 */
LayerJob::LayerJob() : mType(cType_Draw), mLayer(nullptr), mRenderDisplay(nullptr), mPriority(0)
{
}

/**
 * Destroys the layer job.
 */
LayerJob::~LayerJob() = default;

/**
 * Initializes the job type and target layer.
 * @param type job type
 * @param pLayer layer to process, or nullptr for display jobs
 * @param pHeap unused
 */
void LayerJob::initialize(Type type, const Layer* pLayer, sead::Heap* pHeap)
{
    mType = type;
    mLayer = pLayer;

    switch (type)
    {
    case cType_Draw:
        setName("Layer(Draw)");
        break;
    case cType_SubDraw:
        setName("Layer(SubDraw)");
        break;
    case cType_Begin:
        setName("Layer(Begin)");
        break;
    case cType_End:
        setName("Layer(End)");
        break;
    case cType_GPUCalc:
        setName("Layer(GPUCalc)");
        break;
    }
}

/**
 * Detaches the job from its layer.
 */
void LayerJob::finalize()
{
    mLayer = nullptr;
}

/**
 * Sets the display and priority of the job and queues it, or runs it immediately.
 * @param pDisplay display the job renders for
 * @param priority sort priority of the job
 * @param pArray job array to queue into, or nullptr to invoke immediately
 */
void LayerJob::pushBackTo(const RenderDisplay* pDisplay, s32 priority,
                          LayerJobArray* pArray)
{
    mRenderDisplay = pDisplay;
    mPriority = priority;

    if (pArray != nullptr)
    {
        pArray->pushBack(this);
    }
    else
    {
        invoke();
    }
}

/**
 * Records the display lists this job is responsible for.
 */
void LayerJob::invoke()
{
    if (mLayer == nullptr && (mType == cType_Draw || mType == cType_GPUCalc))
    {
        return;
    }

    RenderDLBuffer* pBuffer = mRenderDisplay->mDLBuffer;
    DrawContext context;

    switch (mType)
    {
    case cType_Draw:
        mRenderDisplay->calcLayerDL_(&context, mLayer, mLayer->mLayerIndex * 0x102 + 0xffff);
        break;
    case cType_SubDraw:
        mRenderDisplay->calcSubLayerDL_(&context, mLayer, mLayer->mLayerIndex * 0x102 + 0xffff);
        break;
    case cType_Begin:
    {
        s32 index = pBuffer->begin(&context, "agl::lyr::Renderer::begin", 0);
        mRenderDisplay->beginDraw_(&context);
        RenderDL* pDL = pBuffer->end(&context, index);
        mRenderDisplay->pushBackDL_(&context, pDL, nullptr);
        break;
    }
    case cType_End:
    {
        s32 index = pBuffer->begin(&context, "agl::lyr::Renderer::end", 0x201ff);
        mRenderDisplay->endDraw_(&context);
        RenderDL* pDL = pBuffer->end(&context, index);
        mRenderDisplay->pushBackDL_(&context, pDL, nullptr);
        break;
    }
    case cType_GPUCalc:
        mRenderDisplay->calcGPU_(&context, mLayer, mLayer->mLayerIndex + 1);
        break;
    }
}

/**
 * Compares two layer jobs by priority, higher priorities first.
 * @param pA first job
 * @param pB second job
 * @return difference of the priorities
 */
s32 LayerJob::compare(const LayerJob* pA, const LayerJob* pB)
{
    return pB->mPriority - pA->mPriority;
}

}  // namespace agl::lyr
