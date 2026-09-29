#pragma once

#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <nvn/nvn.h>

#include "common/aglGPUMemBlock.h"

namespace sead::hostio {
class Context;
class PropertyEvent;
}  // namespace sead::hostio

namespace agl::detail {

class DynamicUniformBlock : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(DynamicUniformBlock)
public:
    DynamicUniformBlock();
    virtual ~DynamicUniformBlock();

    void initialize(u64 size, sead::Heap* pHeap);

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    NVNbufferAddress getAddress() const { return mAddress; }

private:
    GPUMemBlock<u8> mMemBlock;
    NVNbuffer mBuffer;
    NVNbufferAddress mAddress;
};

static_assert(sizeof(DynamicUniformBlock) == 0x98);

}  // namespace agl::detail
