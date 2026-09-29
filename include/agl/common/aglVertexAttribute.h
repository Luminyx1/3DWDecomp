#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadSafeArray.h>
#include <nvn/nvn.h>
#include <prim/seadBitFlag.h>

namespace sead {
class Heap;
}

namespace agl {

class DrawContext;
class VertexBuffer;

class VertexAttribute {
public:
    static constexpr s32 cVertexAttributeMax = 16;

    enum Flag {
        cFlag_SetUp = 1 << 0,
        cFlag_Created = 1 << 1,
    };

    struct Attribute_ {
        Attribute_() : mVertexBuffer(nullptr), mStreamIndex(-1), mBufferIndex(-1) {}

        const VertexBuffer* mVertexBuffer;
        s8 mStreamIndex;
        s8 mBufferIndex;
    };
    static_assert(sizeof(Attribute_) == 0x10);

    VertexAttribute();
    virtual ~VertexAttribute();

    void create(u32 bufferNum, sead::Heap* pHeap);
    void cleanUp();
    void destroy();
    void setVertexStream(s32 location, const VertexBuffer* pVertexBuffer, u32 streamIndex);
    const VertexBuffer* getVertexStream(s32 location, u32* pStreamIndex) const;
    void setUp();
    void activate(DrawContext* pDrawContext) const;
    static void disableAttributeAll(DrawContext* pDrawContext);

private:
    s32 disableVertexBuffer_(Attribute_* pAttribute);
    s32 enableVertexBuffer_(Attribute_* pAttribute, const VertexBuffer* pVertexBuffer,
                            u32 streamIndex);

    sead::UnsafeArray<Attribute_, cVertexAttributeMax> mAttributes;
    sead::Buffer<const VertexBuffer*> mVertexBuffers;
    sead::BitFlag8 mFlags;
    alignas(4) NVNvertexAttribState mAttribStates[cVertexAttributeMax];
    alignas(8) NVNvertexStreamState mStreamStates[cVertexAttributeMax];
};
static_assert(sizeof(VertexAttribute) == 0x1e0);

}  // namespace agl
