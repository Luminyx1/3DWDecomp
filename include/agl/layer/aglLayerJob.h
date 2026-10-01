#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <mc/seadJob.h>

namespace sead {
class Heap;
}

namespace agl::lyr {

class Layer;
class RenderDisplay;

class LayerJob : public sead::Job {
    friend class Renderer;

public:
    enum Type {
        cType_Draw = 0,
        cType_SubDraw = 1,
        cType_Begin = 2,
        cType_End = 3,
        cType_GPUCalc = 4,
    };

    LayerJob();
    ~LayerJob() override;

    void initialize(Type type, const Layer* pLayer, sead::Heap* pHeap);
    void finalize();
    void pushBackTo(const RenderDisplay* pDisplay, s32 priority, sead::PtrArray<LayerJob>* pArray);
    void invoke() override;
    static s32 compare(const LayerJob* pA, const LayerJob* pB);

private:
    Type mType;
    const Layer* mLayer;
    const RenderDisplay* mRenderDisplay;
    s32 mPriority;
};
static_assert(sizeof(LayerJob) == 0x38);

using LayerJobArray = sead::PtrArray<LayerJob>;

}  // namespace agl::lyr
