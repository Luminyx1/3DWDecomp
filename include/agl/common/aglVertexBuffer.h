#pragma once

#include <basis/seadTypes.h>
#include <container/seadSafeArray.h>
#include <nvn/nvn.h>

#include "common/aglGPUMemAddr.h"

namespace agl {

class DrawContext;

enum VertexStreamFormat {};

class VertexBuffer {
public:
    static constexpr s32 cVertexStreamMax = 16;

    struct Stream {
        Stream() : mFormat(VertexStreamFormat(1)), mOffset(0), mEnable(false), mDivisor(false) {}

        VertexStreamFormat mFormat;
        u32 mOffset;
        bool mEnable;
        bool mDivisor;
    };
    static_assert(sizeof(Stream) == 0xc);

    VertexBuffer();
    virtual ~VertexBuffer();

    void setUpBuffer(ConstGPUMemVoidAddr buffer, u64 stride, u64 size);
    void flushCPUCache(u32 offset, u64 size) const;
    void invalidateGPUCache(DrawContext* pDrawContext) const;
    void setUpStream(s32 index, VertexStreamFormat format, u64 offset, bool divisor);
    void setBufferPtr(ConstGPUMemVoidAddr buffer, u32 size);

    ConstGPUMemVoidAddr getBuffer() const { return mBuffer; }
    const Stream& getStream(s32 index) const { return mStreams[index]; }
    u32 getStride() const { return mStride; }
    u32 getVertexNum() const { return mVertexNum; }
    u32 getBufferSize() const { return mBufferSize; }
    const NVNbuffer* getNvnBuffer() const { return &mNvnBuffer; }

protected:
    void cleanUp_();

private:
    NVNbuffer mNvnBuffer;
    sead::SafeArray<Stream, cVertexStreamMax> mStreams;
    ConstGPUMemVoidAddr mBuffer;
    u32 mStride;
    u32 mVertexNum;
    u32 mBufferSize;
};
static_assert(sizeof(VertexBuffer) == 0x120);

}  // namespace agl
