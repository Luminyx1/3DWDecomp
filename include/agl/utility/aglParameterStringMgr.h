#pragma once

#include <container/seadPtrArray.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadSafeString.h>
#include <thread/seadCriticalSection.h>

namespace agl::utl {

class ParameterStringMgr : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(ParameterStringMgr)
    ParameterStringMgr();
    virtual ~ParameterStringMgr();

public:
    void initialize(sead::Heap* pHeap);
    const char* appendString(const sead::SafeString& rString);

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    sead::Heap* mHeap = nullptr;
    sead::PtrArray<sead::HeapSafeString> mStrings;
    sead::CriticalSection mCS;
};

}  // namespace agl::utl
