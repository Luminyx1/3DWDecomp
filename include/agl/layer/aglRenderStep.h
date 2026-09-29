#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadBitFlag.h>

namespace sead::hostio {
class Context;
class NodeEvent;
class PropertyEvent;
}  // namespace sead::hostio

namespace agl::lyr {

class DrawMethod;

class RenderStep : public sead::hostio::Node {
public:
    static constexpr s32 cDrawMethodMax = 256;

    RenderStep();
    virtual ~RenderStep() {}

    void calc();
    bool pushBack(DrawMethod* pMethod);
    s32 removeByObject(const void* pObject);
    s32 remove(const DrawMethod* pMethod);
    void clear();
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
    void listenNodeEvent(const sead::hostio::NodeEvent* pEvent);

    bool isEnable() const { return mFlag.isOn(1); }
    const sead::PtrArray<DrawMethod>& getDrawMethods() const { return mDrawMethod; }

private:
    sead::FixedPtrArray<DrawMethod, cDrawMethodMax> mDrawMethod;
    sead::BitFlag32 mFlag;
};
static_assert(sizeof(RenderStep) == 0x820);

}  // namespace agl::lyr
