#include "g3d/aglModelShaderAssign.h"

#include <cstring>
#include <nn/g3d/g3d_ResShader.h>
#include <nn/gfx/gfx_GpuAddress.h>
#include <nn/gfx/gfx_StateInfo.h>

#include "common/aglDrawContext.h"
#include "driver/aglNVNMgr.h"
#include "g3d/aglG3DDecl.h"
#include "g3d/aglNW4FToNN.h"
#include "g3d/aglShaderUtilG3D.h"

namespace nn::gfx::detail {
class Nvn {
public:
    static NVNformat GetAttributeFormat(AttributeFormat format);
};
}  // namespace nn::gfx::detail

namespace agl::g3d {

namespace {

using DeviceImpl = nn::gfx::detail::DeviceImpl<nn::gfx::ApiVariationNvn8>;
using CommandBufferImpl = nn::gfx::detail::CommandBufferImpl<nn::gfx::ApiVariationNvn8>;
using BufferImpl = nn::gfx::detail::BufferImpl<nn::gfx::ApiVariationNvn8>;
using VertexStateImpl = nn::gfx::detail::VertexStateImpl<nn::gfx::ApiVariationNvn8>;

constexpr u32 cDefaultTextureId = 0x23;
constexpr u32 cDefaultSamplerId = 0x24;

/**
 * Gets the gfx device of the graphics driver.
 * @return the gfx device
 */
DeviceImpl* getDevice()
{
    return static_cast<DeviceImpl*>(driver::NVNMgr::instance()->getGfxDevice());
}

}  // namespace

/**
 * Constructs an empty attribute set sized for the maximum number of attributes.
 */
ModelShaderAttribute::ModelShaderAttribute()
{
    calcFetchShaderBufferSize(cAttributeMax);
    clear();
}

/**
 * Computes the fetch shader memory size needed for a number of attributes.
 * @param attributeNum number of attributes and vertex buffers
 */
void ModelShaderAttribute::calcFetchShaderBufferSize(s32 attributeNum)
{
    nn::gfx::VertexAttributeStateInfo attributes[cAttributeMax];
    nn::gfx::VertexBufferStateInfo buffers[cVertexBufferMax];
    for (s32 i = 0; i < attributeNum; i++)
    {
        attributes[i].SetDefault();
        buffers[i].SetDefault();
    }

    nn::gfx::VertexStateInfo info;
    info.SetVertexAttributeStateInfoArray(attributes, attributeNum);
    info.SetVertexBufferStateInfoArray(buffers, attributeNum);
    mFetchShaderBufferSize = VertexStateImpl::GetRequiredMemorySize(info);
}

/**
 * Forgets every attribute and vertex buffer.
 */
void ModelShaderAttribute::clear()
{
    mVertexBufferNum = 0;
    mLocationNum = 0;
    mAttributeNum = 0;
    std::memset(mVertexBuffers, 0, sizeof(mVertexBuffers));
}

/**
 * Constructs an empty attribute set sized for a number of attributes.
 * @param attributeNum number of attributes
 */
ModelShaderAttribute::ModelShaderAttribute(s32 attributeNum)
{
    calcFetchShaderBufferSize(attributeNum);
    clear();
}

/**
 * Finalizes the vertex state.
 */
ModelShaderAttribute::~ModelShaderAttribute()
{
    mLocationNum = 0;
    mAttributeNum = 0;
    if (mVertexState.ToData()->state)
    {
        mVertexState.Finalize(getDevice());
    }
}

/**
 * Sets the memory of the vertex state.
 * @param pBuffer fetch shader memory
 * @param size size of the memory
 */
void ModelShaderAttribute::setFetchShaderBuffer(void* pBuffer, u64 size)
{
    mVertexState.SetMemory(pBuffer, size);
    mFetchShaderBufferSetSize = size;
}

/**
 * Binds the vertex attributes of a shape to the attributes of a shading model.
 * @param pMaterial material with the shader assignment
 * @param pShape shape providing the vertex attributes
 * @param pShadingModel shading model
 * @param pProgram shader program used to skip inactive attributes, may be nullptr
 */
void ModelShaderAttribute::bind(const nn::g3d::ResMaterial* pMaterial,
                                const nn::g3d::ResShape* pShape,
                                const nn::g3d::ResShadingModel* pShadingModel,
                                const nn::g3d::ResShaderProgram* pProgram)
{
    const nn::g3d::ResShaderAssignData* pShaderAssign = pMaterial->ToData().pShaderAssign.Get();
    if (!pShaderAssign)
    {
        return;
    }

    sead::UnsafeArray<Attribute, cAttributeMax> attributes;
    s32 attributeNum = 0;
    for (s32 i = 0; i < pShadingModel->GetAttribCount(); i++)
    {
        const char* pName = pShadingModel->GetAttribName(i);
        const nn::util::ResDic* pAssignDic = pShaderAssign->pAttribAssignDic.Get();
        if (!pAssignDic)
        {
            continue;
        }
        s32 assignIndex = pAssignDic->FindIndex(pName);
        if (assignIndex == -1)
        {
            continue;
        }
        const nn::util::BinPtrToString* pAssignArray = pShaderAssign->pAttribAssignArray.Get();
        if (!pAssignArray)
        {
            continue;
        }
        const nn::util::BinString* pVertexAttribName = pAssignArray[assignIndex].Get();
        const nn::g3d::ResAttribVarData* pAttrib = pShadingModel->GetAttrib(i);
        if (pAttrib->location == -1)
        {
            continue;
        }
        if (pProgram && !pProgram->IsAttribActive(i))
        {
            continue;
        }

        attributes[attributeNum].mLocation =
            AttributeLocation(pVertexAttribName->GetData(), pAttrib->location);
        const nn::util::ResDic* pVertexDic =
            pShape->ToData().pVertex.Get()->ToData().pAttribDic.Get();
        attributes[attributeNum].mIndex =
            pVertexDic ? pVertexDic->FindIndex(pVertexAttribName->GetData()) : -1;
        attributeNum++;
    }

    bind_(pShape->ToData().pVertex.Get(), attributes, attributeNum);
}

/**
 * Builds the vertex buffers, attributes and vertex state from bound attributes.
 * @param pVertex vertex resource
 * @param rAttributes bound attributes
 * @param attributeNum number of bound attributes
 */
void ModelShaderAttribute::bind_(const nn::g3d::ResVertex* pVertex,
                                 const sead::UnsafeArray<Attribute, cAttributeMax>& rAttributes,
                                 s32 attributeNum)
{
    mVertexBufferNum = 0;

    sead::SafeArray<s32, cVertexBufferMax> bufferSlots;
    sead::UnsafeArray<Attribute, cAttributeMax> validAttributes;
    bufferSlots.fill(-1);

    s32 validNum = 0;
    for (s32 i = 0; i < attributeNum; i++)
    {
        const Attribute& rAttribute = rAttributes[i];
        if (!rAttribute.mLocation.isValid() || rAttribute.mIndex == -1)
        {
            continue;
        }
        validAttributes[validNum++] = rAttribute;
    }

    mLocationNum = attributeNum;
    mAttributeNum = validNum;

    nn::gfx::VertexAttributeStateInfo attributeInfos[cAttributeMax];
    nn::gfx::VertexBufferStateInfo bufferInfos[cVertexBufferMax];
    const nn::g3d::ResVertexData& rVertex = pVertex->ToData();
    for (s32 i = 0; i < validNum; i++)
    {
        const Attribute& rAttribute = validAttributes[i];
        s32 attribIndex = rAttribute.mIndex;
        const nn::g3d::ResVertexAttribData& rResAttrib = rVertex.pAttribArray.Get()[attribIndex];
        s32 bufferIndex = rResAttrib.bufferIndex;
        if (bufferSlots[bufferIndex] == -1)
        {
            const nn::gfx::Buffer* pBuffer = rVertex.pVertexBufferArray.Get()[bufferIndex];
            bufferSlots[bufferIndex] = mVertexBufferNum;
            VertexBuffer& rBuffer = mVertexBuffers[mVertexBufferNum];
            rBuffer.mpBuffer = pBuffer;
            rBuffer.mStride = rVertex.pVertexBufferStrideArray.Get()[bufferIndex].stride;
            rBuffer.mSize = rVertex.pVertexBufferInfoArray.Get()[bufferIndex].size;
            bufferInfos[mVertexBufferNum].SetDefault();
            bufferInfos[mVertexBufferNum].SetStride(mVertexBuffers[mVertexBufferNum].mStride);
            mVertexBufferNum++;
        }

        s32 location = rAttribute.mLocation.getLocation(cShaderType_Vertex);
        u32 format = rResAttrib.format;
        u16 offset = rResAttrib.offset;
        nn::gfx::VertexAttributeStateInfo& rInfo = attributeInfos[i];
        rInfo.SetDefault();
        rInfo.SetOffset(offset);
        rInfo.SetFormat(static_cast<nn::gfx::AttributeFormat>(format));
        rInfo.SetShaderSlot(location);
        rInfo.SetNamePtr(nullptr);
        rInfo.SetBufferIndex(bufferSlots[bufferIndex]);

        AttributeInfo& rAttributeInfo = mAttributes[i];
        rAttributeInfo.mFormat = format;
        rAttributeInfo.mNvnFormat =
            nn::gfx::detail::Nvn::GetAttributeFormat(static_cast<nn::gfx::AttributeFormat>(format));
        rAttributeInfo.mOffset = offset;
        rAttributeInfo.mLocation = location;
        rAttributeInfo.mBufferIndex = bufferSlots[bufferIndex];
    }

    nn::gfx::VertexStateInfo info;
    info.SetVertexAttributeStateInfoArray(attributeInfos, mAttributeNum);
    info.SetVertexBufferStateInfoArray(bufferInfos, mVertexBufferNum);
    mFetchShaderBufferSize = VertexStateImpl::GetRequiredMemorySize(info);

    if (mVertexState.ToData()->state)
    {
        mVertexState.Finalize(getDevice());
    }
    if (mAttributeNum != 0)
    {
        mVertexState.Initialize(getDevice(), info, nullptr);
    }
}

/**
 * Checks whether two attribute sets have the same vertex layout.
 * @param rOther other attribute set
 * @return whether every attribute matches
 */
bool ModelShaderAttribute::isSameVertexAttribute(const ModelShaderAttribute& rOther) const
{
    if (mAttributeNum != rOther.mAttributeNum)
    {
        return false;
    }
    for (s32 i = 0; i < mAttributeNum; i++)
    {
        if (rOther.mAttributes[i].mLocation != mAttributes[i].mLocation ||
            rOther.mAttributes[i].mBufferIndex != mAttributes[i].mBufferIndex ||
            rOther.mAttributes[i].mNvnFormat != mAttributes[i].mNvnFormat ||
            rOther.mAttributes[i].mOffset != mAttributes[i].mOffset)
        {
            return false;
        }
    }
    return true;
}

/**
 * Gets the number of attributes.
 * @return the number of attributes
 */
s32 ModelShaderAttribute::getAttributeNum() const
{
    return mAttributeNum;
}

/**
 * Gets the shader location of an attribute.
 * @param index index of the attribute
 * @return the location
 */
s32 ModelShaderAttribute::getAttributeLocation(s32 index) const
{
    return mAttributes[index].mLocation;
}

/**
 * Gets the vertex buffer index of an attribute.
 * @param index index of the attribute
 * @return the vertex buffer index
 */
s32 ModelShaderAttribute::getAttributeBufferIndex(s32 index) const
{
    return mAttributes[index].mBufferIndex;
}

/**
 * Gets the NVN format of an attribute.
 * @param index index of the attribute
 * @return the NVN format
 */
s32 ModelShaderAttribute::getAttributeFormat(s32 index) const
{
    return mAttributes[index].mNvnFormat;
}

/**
 * Gets the byte offset of an attribute.
 * @param index index of the attribute
 * @return the offset
 */
s32 ModelShaderAttribute::getAttributeOffset(s32 index) const
{
    return mAttributes[index].mOffset;
}

/**
 * Checks whether two attribute sets use the same vertex buffers.
 * @param rOther other attribute set
 * @return whether every vertex buffer matches
 */
bool ModelShaderAttribute::isSameVertexBuffer(const ModelShaderAttribute& rOther) const
{
    if (mVertexBufferNum != rOther.mVertexBufferNum)
    {
        return false;
    }
    for (s32 i = 0; i < mVertexBufferNum; i++)
    {
        if (rOther.mVertexBuffers[i].mpBuffer != mVertexBuffers[i].mpBuffer)
        {
            return false;
        }
    }
    return true;
}

/**
 * Sets a vertex buffer.
 * @param pBuffer buffer
 * @param index index of the vertex buffer
 * @param stride stride of a vertex
 * @param size size of the buffer
 */
void ModelShaderAttribute::setVertexBuffer(const nn::gfx::Buffer* pBuffer, s32 index, s32 stride,
                                           s32 size)
{
    VertexBuffer& rBuffer = mVertexBuffers[index];
    rBuffer.mStride = stride;
    rBuffer.mSize = size;
    rBuffer.mpBuffer = pBuffer;
}

/**
 * Sets the vertex state on a draw context.
 * @param pDrawContext draw context
 */
void ModelShaderAttribute::activateVertexAttribute(DrawContext* pDrawContext) const
{
    if (mAttributeNum != 0)
    {
        static_cast<CommandBufferImpl*>(pDrawContext->getCommandBuffer())
            ->SetVertexState(&mVertexState);
    }
}

/**
 * Binds every vertex buffer on a draw context.
 * @param pDrawContext draw context
 */
void ModelShaderAttribute::activateVertexBuffer(DrawContext* pDrawContext) const
{
    if (mAttributeNum == 0)
    {
        return;
    }
    for (s32 i = 0; i < mVertexBufferNum; i++)
    {
        nn::gfx::GpuAddress address;
        address.ToData()->value = 0;
        address.ToData()->impl = 0;
        static_cast<const BufferImpl*>(mVertexBuffers[i].mpBuffer)->GetGpuAddress(&address);
        static_cast<CommandBufferImpl*>(pDrawContext->getCommandBuffer())
            ->SetVertexBuffer(i, address, mVertexBuffers[i].mStride, mVertexBuffers[i].mSize);
    }
}

/**
 * Searches the attribute bound to a shader location.
 * @param location shader location
 * @return the index of the attribute, or -1 if not found
 */
s32 ModelShaderAttribute::searchAttributeIndex(s32 location) const
{
    for (s32 i = 0; i < mAttributeNum; i++)
    {
        if (mAttributes[i].mLocation == location)
        {
            return i;
        }
    }
    return -1;
}

/**
 * Gets the stride of a vertex buffer.
 * @param index index of the vertex buffer
 * @return the stride
 */
s32 ModelShaderAttribute::getVertexBufferStride(s32 index) const
{
    return mVertexBuffers[index].mStride;
}

/**
 * Gets the size of a vertex buffer.
 * @param index index of the vertex buffer
 * @return the size
 */
s32 ModelShaderAttribute::getVertexBufferSize(s32 index) const
{
    return mVertexBuffers[index].mSize;
}

/**
 * Gets the fetch shader memory size required by the vertex state.
 * @return the size
 */
u32 ModelShaderAttribute::getFetchShaderBufferSize() const
{
    return mFetchShaderBufferSize;
}

/**
 * Gets the memory of the vertex state.
 * @return the memory
 */
void* ModelShaderAttribute::getFetchShaderBuffer()
{
    return mVertexState.GetMemory();
}

/**
 * Gets the memory of the vertex state.
 * @return the memory
 */
const void* ModelShaderAttribute::getFetchShaderBuffer() const
{
    return mVertexState.GetMemory();
}

/**
 * Rebuilds the vertex state from the current vertex buffers and attributes.
 */
void ModelShaderAttribute::flushVertexBuffer()
{
    nn::gfx::VertexAttributeStateInfo attributeInfos[cAttributeMax];
    nn::gfx::VertexBufferStateInfo bufferInfos[cVertexBufferMax];
    for (s32 i = 0; i < mVertexBufferNum; i++)
    {
        bufferInfos[i].SetDefault();
        bufferInfos[i].SetStride(mVertexBuffers[i].mStride);
    }
    for (s32 i = 0; i < mAttributeNum; i++)
    {
        const AttributeInfo& rAttributeInfo = mAttributes[i];
        nn::gfx::VertexAttributeStateInfo& rInfo = attributeInfos[i];
        rInfo.SetDefault();
        rInfo.SetBufferIndex(rAttributeInfo.mBufferIndex);
        rInfo.SetFormat(static_cast<nn::gfx::AttributeFormat>(rAttributeInfo.mFormat));
        rInfo.SetOffset(rAttributeInfo.mOffset);
        rInfo.SetShaderSlot(rAttributeInfo.mLocation);
        rInfo.SetNamePtr(nullptr);
    }

    nn::gfx::VertexStateInfo info;
    info.SetVertexBufferStateInfoArray(bufferInfos, mVertexBufferNum);
    info.SetVertexAttributeStateInfoArray(attributeInfos, mAttributeNum);
    if (mAttributeNum != 0)
    {
        if (mVertexState.ToData()->state)
        {
            mVertexState.Finalize(getDevice());
        }
        mVertexState.Initialize(getDevice(), info, nullptr);
    }
}

/**
 * Constructs an empty sampler set.
 */
ModelShaderSampler::ModelShaderSampler() = default;

/**
 * Forgets every sampler.
 */
void ModelShaderSampler::clear()
{
    mSamplerNum = 0;
    for (auto& rSampler : mSamplers)
    {
        rSampler.mSamplerIndex = 0;
        static_cast<ShaderLocation&>(rSampler.mLocation) = ShaderLocation();
    }
}

/**
 * Binds the samplers of a material to the samplers of a shading model.
 * @param pMaterial material
 * @param pShadingModel shading model
 * @param pProgram shader program
 */
void ModelShaderSampler::bind(const nn::g3d::ResMaterial* pMaterial,
                              const nn::g3d::ResShadingModel* pShadingModel,
                              const nn::g3d::ResShaderProgram* pProgram)
{
    const nn::g3d::ResShaderAssignData* pShaderAssign = pMaterial->ToData().pShaderAssign.Get();
    if (!pShaderAssign)
    {
        return;
    }
    mpResMaterial = pMaterial;

    for (s32 i = 0; i < pShadingModel->GetSamplerCount(); i++)
    {
        const char* pName = pShadingModel->GetSamplerName(i);
        const nn::util::ResDic* pAssignDic = pShaderAssign->pSamplerAssignDic.Get();
        if (!pAssignDic)
        {
            continue;
        }
        s32 assignIndex = pAssignDic->FindIndex(pName);
        if (assignIndex == -1)
        {
            continue;
        }

        s32 samplerIndex;
        const nn::util::ResDic* pSamplerDic = mpResMaterial->ToData().pSamplerDic.Get();
        if (pSamplerDic)
        {
            samplerIndex = pSamplerDic->FindIndex(
                pShaderAssign->pSamplerAssignArray.Get()[assignIndex].Get()->GetData());
        }
        else
        {
            samplerIndex = -1;
        }

        SamplerLocation location;
        ShaderUtilG3D::search(&location, pShadingModel, pProgram, pName);
        pushBackSampler(samplerIndex, location);
    }
}

/**
 * Adds a sampler unless it is invalid or already present.
 * @param samplerIndex index of the material sampler
 * @param rLocation shader location
 */
void ModelShaderSampler::pushBackSampler(s32 samplerIndex, const SamplerLocation& rLocation)
{
    if (!rLocation.isValid())
    {
        return;
    }
    for (s32 i = 0; i < mSamplerNum; i++)
    {
        if (mSamplers[i].mSamplerIndex == samplerIndex && mSamplers[i].mLocation == rLocation)
        {
            return;
        }
    }
    mSamplers[mSamplerNum].mLocation = rLocation;
    mSamplers[mSamplerNum++].mSamplerIndex = samplerIndex;
}

/**
 * Gets the name of the material sampler of an entry.
 * @param index index of the entry
 * @return the name, or nullptr if the material has no sampler dictionary
 */
const char* ModelShaderSampler::getResSamplerName(s32 index) const
{
    const nn::util::ResDic* pSamplerDic = mpResMaterial->ToData().pSamplerDic.Get();
    if (!pSamplerDic)
    {
        return nullptr;
    }
    return pSamplerDic->GetKey(mSamplers[index].mSamplerIndex).data();
}

/**
 * Binds the textures of a material instance to every sampler.
 * @param pDrawContext draw context
 * @param pMaterial material instance
 */
void ModelShaderSampler::activate(DrawContext* pDrawContext,
                                  const nn::g3d::MaterialObj* pMaterial) const
{
    for (s32 i = 0; i < mSamplerNum; i++)
    {
        const nn::gfx::ResTexture* pTexture =
            MaterialObj::GetResTexture(pMaterial, mSamplers[i].mSamplerIndex);
        if (pTexture)
        {
            u32 samplerId =
                mpResMaterial->ToData().pSamplerSlotArray.Get()[mSamplers[i].mSamplerIndex];
            u32 textureId = pTexture->ToData().userDescriptorSlot.value;
            ShaderUtilG3D::load(pDrawContext, mSamplers[i].mLocation, samplerId, textureId);
        }
        else
        {
            u32 samplerId = cDefaultSamplerId;
            u32 textureId = cDefaultTextureId;
            ShaderUtilG3D::load(pDrawContext, mSamplers[i].mLocation, samplerId, textureId);
        }
    }
}

/**
 * Binds the textures of a material instance, or overriding textures, to every sampler.
 * @param pDrawContext draw context
 * @param pMaterial material instance
 * @param ppTextures overriding textures indexed by material sampler, entries may be nullptr
 */
void ModelShaderSampler::activate(DrawContext* pDrawContext, const nn::g3d::MaterialObj* pMaterial,
                                  const nn::gfx::ResTexture** ppTextures) const
{
    for (s32 i = 0; i < mSamplerNum; i++)
    {
        const nn::gfx::ResTexture* pTexture =
            MaterialObj::GetResTexture(pMaterial, mSamplers[i].mSamplerIndex);
        const nn::gfx::ResTexture* pOverride = ppTextures[mSamplers[i].mSamplerIndex];
        if (pOverride)
        {
            pTexture = pOverride;
        }
        if (pTexture)
        {
            u32 samplerId =
                mpResMaterial->ToData().pSamplerSlotArray.Get()[mSamplers[i].mSamplerIndex];
            u32 textureId = pTexture->ToData().userDescriptorSlot.value;
            ShaderUtilG3D::load(pDrawContext, mSamplers[i].mLocation, samplerId, textureId);
        }
        else
        {
            u32 samplerId = cDefaultSamplerId;
            u32 textureId = cDefaultTextureId;
            ShaderUtilG3D::load(pDrawContext, mSamplers[i].mLocation, samplerId, textureId);
        }
    }
}

/**
 * Binds the textures of a material resource to every sampler.
 * @param pDrawContext draw context
 * @param pMaterial material resource
 */
void ModelShaderSampler::activate(DrawContext* pDrawContext,
                                  const nn::g3d::ResMaterial* pMaterial) const
{
    for (s32 i = 0; i < mSamplerNum; i++)
    {
        const nn::gfx::ResTexture* pTexture =
            ResMaterial::GetTexture(pMaterial, mSamplers[i].mSamplerIndex);
        if (pTexture)
        {
            u32 samplerId =
                mpResMaterial->ToData().pSamplerSlotArray.Get()[mSamplers[i].mSamplerIndex];
            u32 textureId = pTexture->ToData().userDescriptorSlot.value;
            ShaderUtilG3D::load(pDrawContext, mSamplers[i].mLocation, samplerId, textureId);
        }
        else
        {
            u32 samplerId = cDefaultSamplerId;
            u32 textureId = cDefaultTextureId;
            ShaderUtilG3D::load(pDrawContext, mSamplers[i].mLocation, samplerId, textureId);
        }
    }
}

/**
 * Checks whether two sampler sets bind the same samplers to the same locations.
 * @param rOther other sampler set
 * @return whether both sets are equal
 */
bool ModelShaderSampler::isEqual(const ModelShaderSampler& rOther) const
{
    if (mSamplerNum != rOther.mSamplerNum)
    {
        return false;
    }
    for (s32 i = 0; i < mSamplerNum; i++)
    {
        if (&mpResMaterial->ToData().pSamplerArray.Get()[mSamplers[i].mSamplerIndex] !=
            &rOther.mpResMaterial->ToData().pSamplerArray.Get()[rOther.mSamplers[i].mSamplerIndex])
        {
            return false;
        }
        if (!(mSamplers[i].mLocation == rOther.mSamplers[i].mLocation))
        {
            return false;
        }
    }
    return true;
}

}  // namespace agl::g3d
