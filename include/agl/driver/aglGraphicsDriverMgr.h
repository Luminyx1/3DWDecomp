#pragma once

#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <nvn/nvn.h>

namespace sead::hostio {
class Context;
class PropertyEvent;
}  // namespace sead::hostio

namespace agl {
class DrawContext;
class DisplayList;

enum PrimitiveRestartIndex {
    cPrimitiveRestartIndex_None = 0,
    cPrimitiveRestartIndex_U16 = 0xFFFF,
    cPrimitiveRestartIndex_U32 = 0xFFFFFFFF,
};
}  // namespace agl

namespace agl::driver {

inline NVNcommandBuffer* getNvnCommandBuffer(const DrawContext* pDrawContext) {
    return *reinterpret_cast<NVNcommandBuffer* const*>(reinterpret_cast<uintptr_t>(pDrawContext) +
                                                       0xb8);
}

class GraphicsDriverMgr : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(GraphicsDriverMgr)
public:
    GraphicsDriverMgr();
    virtual ~GraphicsDriverMgr();

    virtual void dumpInfo() const {}
    virtual void waitDrawDone(DrawContext* pDrawContext) const {}

    void waitDrawDone() const;
    void setPointLimits(DrawContext* pDrawContext, f32 min, f32 max) const;
    void setPointSize(DrawContext* pDrawContext, f32 pointSize) const;
    void setLineWidth(DrawContext* pDrawContext, f32 lineWidth) const;
    void setPrimitiveRestartIndex(DrawContext* pDrawContext, PrimitiveRestartIndex index) const;
    void setDepthClamp(DrawContext* pDrawContext, bool enable) const;
    void setPolygonOffset(DrawContext* pDrawContext, f32 factor, f32 units) const;

    DisplayList* getDefaultCommandBuffer();

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

protected:
    void initialize_(sead::Heap* pHeap);

    DisplayList* mDefaultCommandBuffer;
    void* _30;
};

static_assert(sizeof(GraphicsDriverMgr) == 0x38);

}  // namespace agl::driver
