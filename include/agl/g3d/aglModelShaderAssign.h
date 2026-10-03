#pragma once

#include <container/seadSafeArray.h>
#include <nn/gfx/gfx_Buffer.h>
#include <nn/gfx/detail/gfx_State-api.nvn.8.h>
#include <nn/gfx/gfx_State.h>
#include <nn/gfx/gfx_Types.h>

#include "common/aglShaderLocation.h"

namespace nn::g3d {
class MaterialObj;
class ResMaterial;
class ResShaderProgram;
class ResShadingModel;
class ResShape;
class ResVertex;
}  // namespace nn::g3d

namespace nn::gfx {
class ResTexture;
}  // namespace nn::gfx

namespace agl {
class DrawContext;
}  // namespace agl

namespace agl::g3d {

class ModelShaderAttribute {
public:
    static constexpr s32 cAttributeMax = 16;
    static constexpr s32 cVertexBufferMax = 16;

    struct Attribute {
        s32 mIndex = 0;
        AttributeLocation mLocation;
    };
    static_assert(sizeof(Attribute) == 0x20);

    ModelShaderAttribute();
    explicit ModelShaderAttribute(s32 attributeNum);
    ~ModelShaderAttribute();

    void calcFetchShaderBufferSize(s32 attributeNum);
    void clear();
    void setFetchShaderBuffer(void* pBuffer, u64 size);
    void bind(const nn::g3d::ResMaterial* pMaterial, const nn::g3d::ResShape* pShape,
              const nn::g3d::ResShadingModel* pShadingModel,
              const nn::g3d::ResShaderProgram* pProgram);
    bool isSameVertexAttribute(const ModelShaderAttribute& rOther) const;
    s32 getAttributeNum() const;
    s32 getAttributeLocation(s32 index) const;
    s32 getAttributeBufferIndex(s32 index) const;
    s32 getAttributeFormat(s32 index) const;
    s32 getAttributeOffset(s32 index) const;
    bool isSameVertexBuffer(const ModelShaderAttribute& rOther) const;
    void setVertexBuffer(const nn::gfx::Buffer* pBuffer, s32 index, s32 stride, s32 size);
    void activateVertexAttribute(DrawContext* pDrawContext) const;
    void activateVertexBuffer(DrawContext* pDrawContext) const;
    s32 searchAttributeIndex(s32 location) const;
    s32 getVertexBufferStride(s32 index) const;
    s32 getVertexBufferSize(s32 index) const;
    u64 getFetchShaderBufferSize() const;
    void* getFetchShaderBuffer();
    const void* getFetchShaderBuffer() const;
    void flushVertexBuffer();

private:
    struct VertexBuffer {
        const nn::gfx::Buffer* mpBuffer;
        u32 mStride;
        u32 mSize;
    };

    struct AttributeInfo {
        u16 mFormat;
        u16 mNvnFormat;
        u16 mOffset;
        u8 mLocation;
        u8 mBufferIndex;
    };

    void bind_(const nn::g3d::ResVertex* pVertex,
               const sead::UnsafeArray<Attribute, cAttributeMax>& rAttributes, s32 attributeNum);

    u8 mVertexBufferNum;
    u16 mFetchShaderBufferSetSize = 0;
    VertexBuffer mVertexBuffers[cVertexBufferMax];
    nn::gfx::detail::VertexStateImpl<nn::gfx::ApiVariationNvn8> mVertexState;
    AttributeInfo mAttributes[cAttributeMax];
    u8 mLocationNum;
    u8 mAttributeNum;
    u16 mFetchShaderBufferSize;
};
static_assert(sizeof(ModelShaderAttribute) == 0x1b0);

class ModelShaderSampler {
public:
    static constexpr s32 cSamplerMax = 16;

    ModelShaderSampler();

    void clear();
    void bind(const nn::g3d::ResMaterial* pMaterial, const nn::g3d::ResShadingModel* pShadingModel,
              const nn::g3d::ResShaderProgram* pProgram);
    void pushBackSampler(s32 samplerIndex, const SamplerLocation& rLocation);
    const char* getResSamplerName(s32 index) const;
    void activate(DrawContext* pDrawContext, const nn::g3d::MaterialObj* pMaterial) const;
    void activate(DrawContext* pDrawContext, const nn::g3d::MaterialObj* pMaterial,
                  const nn::gfx::ResTexture** ppTextures) const;
    void activate(DrawContext* pDrawContext, const nn::g3d::ResMaterial* pMaterial) const;
    bool isEqual(const ModelShaderSampler& rOther) const;

private:
    struct Sampler {
        SamplerLocation mLocation;
        u8 mSamplerIndex;
    };
    static_assert(sizeof(Sampler) == 0x20);

    const nn::g3d::ResMaterial* mpResMaterial = nullptr;
    Sampler mSamplers[cSamplerMax];
    s32 mSamplerNum = 0;
};
static_assert(sizeof(ModelShaderSampler) == 0x210);

}  // namespace agl::g3d
