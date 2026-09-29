#pragma once

#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadSafeString.h>
#include <thread/seadCriticalSection.h>

namespace sead::hostio {
class Context;
class PropertyEvent;
}  // namespace sead::hostio

namespace agl::detail {

class RootNode : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(RootNode)
public:
    RootNode();
    virtual ~RootNode();

    void initialize(sead::Heap* pHeap, const sead::SafeString& rMetaSuffix);

    static void setNodeMeta(sead::hostio::Node* pNode, const sead::SafeString& rMeta);
    void appendChildAGL(const sead::SafeString& rName, sead::hostio::Node* pNode);

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    sead::FixedSafeString<256> mMetaSuffix;
    sead::CriticalSection mCriticalSection;
};

static_assert(sizeof(RootNode) == 0x180);

}  // namespace agl::detail
