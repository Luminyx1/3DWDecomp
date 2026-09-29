#pragma once

#include <basis/seadTypes.h>
#include <nvn/nvn.h>

#include "common/aglGPUMemAddr.h"

namespace agl {

class DrawContext;

enum IndexStreamFormat {
    cIndexStreamFormat_u16 = 1,
    cIndexStreamFormat_u32 = 2,
};

class IndexStream {
public:
    IndexStream();
    virtual ~IndexStream();

    void flushCPUCache(u32 start, u32 count) const;
    void invalidateGPUCache(DrawContext* pDrawContext) const;
    void setBufferPtr(ConstGPUMemVoidAddr buffer, u32 count);

    IndexStreamFormat getFormat() const { return mFormat; }
    u32 getCount() const { return mCount; }
    u32 getStride() const { return mStride; }
    ConstGPUMemVoidAddr getBuffer() const { return mBuffer; }
    const NVNbuffer* getNvnBuffer() const { return &mNvnBuffer; }

protected:
    void setUpStream_(ConstGPUMemVoidAddr buffer, IndexStreamFormat format, u32 count);
    void cleanUp_();

private:
    IndexStreamFormat mFormat;
    NVNdrawPrimitive mPrimitiveType;
    ConstGPUMemVoidAddr mBuffer;
    u32 mCount;
    u32 mStride;
    NVNbuffer mNvnBuffer;
};
static_assert(sizeof(IndexStream) == 0x60);

}  // namespace agl
