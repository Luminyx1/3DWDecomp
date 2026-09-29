#pragma once

// Declarations of the parts of nn::g3d used by agl's g3d glue (NintendoSDK g3d for gfx/NVN).
// Include this instead of <nn/g3d/g3d_ResFile.h> & co.

#include <nn/gfx/gfx_Buffer.h>
#include <nn/gfx/gfx_DescriptorSlot.h>
#include <nn/gfx/gfx_ResTexture.h>
#include <nn/gfx/gfx_Texture.h>
#include <nn/gfx/gfx_Types.h>
#include <nn/types.h>
#include <nn/util/util_BinTypes.h>
#include <nn/util/util_BinaryFormat.h>
#include <nn/util/util_ResDic.h>

namespace nn::g3d {

class MaterialObj;

typedef nn::gfx::detail::BufferImpl<nn::gfx::ApiVariationNvn8> BufferImpl;

class TextureRef {
public:
    TextureRef() : m_pTextureView(nullptr), m_DescriptorSlot(InvalidDescriptorSlot) {}
    TextureRef(const nn::gfx::TextureView* pTextureView, u64 descriptorSlot)
        : m_pTextureView(pTextureView), m_DescriptorSlot(descriptorSlot)
    {
    }

    const nn::gfx::TextureView* GetTextureView() const { return m_pTextureView; }
    u64 GetDescriptorSlot() const { return m_DescriptorSlot; }

    static const u64 InvalidDescriptorSlot = 0xFFFFFFFFFFFFFFFF;

private:
    const nn::gfx::TextureView* m_pTextureView;
    u64 m_DescriptorSlot;
};

class BindResult {
public:
    enum Flag { Flag_Success = 1 << 0, Flag_Failure = 1 << 16 };

    bool IsComplete() const { return (m_Flag & (Flag_Success | Flag_Failure)) == Flag_Success; }

private:
    u32 m_Flag;
    u32 m_Pad;
};

typedef TextureRef (*TextureBindCallback)(const char* name, void* pUserData);

inline void* GetTextureViewUserPtr(const nn::gfx::TextureView* pTextureView)
{
    return pTextureView->ToData()->userPtr.ptr;
}

struct ResSamplerData {
    nn::util::BinPtr pName;
    nn::util::BinPtr pGfxSampler;
    u8 _10[0x78 - 0x10];
};

struct ResShaderAssignData {
    u8 _0[0x10];
    nn::util::BinTPtr<nn::util::BinPtrToString> pAttribAssignArray;
    nn::util::BinTPtr<nn::util::ResDic> pAttribAssignDic;
    nn::util::BinTPtr<nn::util::BinPtrToString> pSamplerAssignArray;
    nn::util::BinTPtr<nn::util::ResDic> pSamplerAssignDic;
};

struct ResMaterialData {
    u8 _0[0x20];
    nn::util::BinTPtr<ResShaderAssignData> pShaderAssign;
    nn::util::BinTPtr<const nn::gfx::TextureView*> pTextureArray;
    nn::util::BinTPtr<nn::util::BinPtrToString> pTextureNameArray;
    nn::util::BinTPtr<ResSamplerData> pSamplerArray;
    u8 _40[0x48 - 0x40];
    nn::util::BinTPtr<nn::util::ResDic> pSamplerDic;
    u8 _50[0x88 - 0x50];
    nn::util::BinTPtr<u64> pSamplerSlotArray;
    nn::util::BinTPtr<u64> pTextureSlotArray;
    u8 _98[0x9c - 0x98];
    u8 samplerCount;
    u8 textureCount;
    u8 _9e[0xa8 - 0x9e];
};

class ResMaterial : public nn::util::AccessorBase<ResMaterialData> {
public:
    int GetSamplerCount() const { return ToData().samplerCount; }
    int GetTextureCount() const { return ToData().textureCount; }

    const nn::gfx::TextureView* GetTextureView(int index) const
    {
        return ToData().pTextureArray.Get()[index];
    }
    const char* GetTextureName(int index) const
    {
        return ToData().pTextureNameArray.Get()[index].Get()->GetData();
    }
    u64 GetTextureDescriptorSlot(int index) const
    {
        return ToData().pTextureSlotArray.Get()[index];
    }

    void ForceBindTexture(int index, const TextureRef& rRef)
    {
        ToData().pTextureArray.Get()[index] = rRef.GetTextureView();
        ToData().pTextureSlotArray.Get()[index] = rRef.GetDescriptorSlot();
    }
    void ReleaseTexture(int index)
    {
        ToData().pTextureArray.Get()[index] = nullptr;
        ToData().pTextureSlotArray.Get()[index] = TextureRef::InvalidDescriptorSlot;
    }
};

struct ResVertexAttribData {
    u8 _0[0x8];
    u32 format;
    u16 offset;
    u8 bufferIndex;
    u8 _f;
};

struct ResVertexBufferInfoData {
    u32 size;
    u8 _4[0xc];
};

struct ResVertexBufferStrideData {
    u32 stride;
    u8 _4[0xc];
};

struct ResVertexData {
    u8 _0[0x8];
    nn::util::BinTPtr<ResVertexAttribData> pAttribArray;
    nn::util::BinTPtr<nn::util::ResDic> pAttribDic;
    u8 _18[0x28 - 0x18];
    nn::util::BinTPtr<nn::gfx::Buffer*> pVertexBufferArray;
    nn::util::BinTPtr<ResVertexBufferInfoData> pVertexBufferInfoArray;
    nn::util::BinTPtr<ResVertexBufferStrideData> pVertexBufferStrideArray;
};

class ResVertex : public nn::util::AccessorBase<ResVertexData> {};

struct ResShapeData {
    u8 _0[0x10];
    nn::util::BinTPtr<ResVertex> pVertex;
};

class ResShape : public nn::util::AccessorBase<ResShapeData> {};

struct ResModelData {
    u8 _0[0x38];
    nn::util::BinTPtr<ResMaterial> pMaterialArray;
    u8 _40[0x6c - 0x40];
    u16 materialCount;
    u8 _6e[0x78 - 0x6e];
};

class ResModel : public nn::util::AccessorBase<ResModelData> {
public:
    int GetMaterialCount() const { return ToData().materialCount; }
    ResMaterial* GetMaterial(int index) { return &ToData().pMaterialArray.Get()[index]; }
};

struct ResExternalFileData {
    nn::util::BinPtr pData;
    u32 size;
    u32 _c;
};

struct ResFileData {
    nn::util::BinaryFileHeader fileHeader;
    u8 _20[0x28 - 0x20];
    nn::util::BinTPtr<ResModel> pModelArray;
    u8 _30[0xb8 - 0x30];
    nn::util::BinTPtr<ResExternalFileData> pExternalFileArray;
    nn::util::BinTPtr<nn::util::ResDic> pExternalFileDic;
    u8 _c8[0xdc - 0xc8];
    u16 modelCount;
    u8 _de[0xec - 0xde];
    u16 externalFileCount;
};

class ResFile : public nn::util::AccessorBase<ResFileData> {
public:
    static ResFile* ResCast(void* pFile);

    void Setup(nn::gfx::Device* pDevice);
    void Cleanup(nn::gfx::Device* pDevice);
    BindResult BindTexture(TextureBindCallback pCallback, void* pUserData);

    int GetModelCount() const { return ToData().modelCount; }
    ResModel* GetModel(int index) { return &ToData().pModelArray.Get()[index]; }

    int FindExternalFileIndex(const char* pName) const
    {
        const nn::util::ResDic* pDic = ToData().pExternalFileDic.Get();
        return pDic ? pDic->FindIndex(pName) : nn::util::ResDic::Npos;
    }
    int GetExternalFileCount() const { return ToData().externalFileCount; }
    const char* GetExternalFileName(int index) const
    {
        const nn::util::ResDic* pDic = ToData().pExternalFileDic.Get();
        return pDic ? pDic->GetKey(index).data() : nullptr;
    }
    const ResExternalFileData* GetExternalFile(int index) const
    {
        return &ToData().pExternalFileArray.Get()[index];
    }
    const ResExternalFileData* FindExternalFile(const char* pName) const
    {
        int index = FindExternalFileIndex(pName);
        return index != nn::util::ResDic::Npos ? GetExternalFile(index) : nullptr;
    }
};

struct ResMaterialAnimData {
    u8 _0[0x30];
    nn::util::BinTPtr<const nn::gfx::TextureView*> pTextureArray;
    nn::util::BinTPtr<nn::util::BinPtrToString> pTextureNameArray;
    u8 _40[0x50 - 0x40];
    nn::util::BinTPtr<u64> pTextureSlotArray;
    u8 _58[0x6c - 0x58];
    u16 textureCount;
};

class ResMaterialAnim : public nn::util::AccessorBase<ResMaterialAnimData> {
public:
    int GetTextureCount() const { return ToData().textureCount; }
    const nn::gfx::TextureView* GetTextureView(int index) const
    {
        return ToData().pTextureArray.Get()[index];
    }
    const char* GetTextureName(int index) const
    {
        return ToData().pTextureNameArray.Get()[index].Get()->GetData();
    }
    void ForceBindTexture(int index, const TextureRef& rRef)
    {
        ToData().pTextureArray.Get()[index] = rRef.GetTextureView();
        ToData().pTextureSlotArray.Get()[index] = rRef.GetDescriptorSlot();
    }
    void ReleaseTexture(int index)
    {
        ToData().pTextureArray.Get()[index] = nullptr;
        ToData().pTextureSlotArray.Get()[index] = TextureRef::InvalidDescriptorSlot;
    }
};

class MaterialObj {
public:
    typedef void (*TextureChangeCallback)(MaterialObj* pMaterial, int index);

    const ResMaterial* GetResource() const { return m_pRes; }
    bool IsBlockBufferValid() const { return m_Flag & Flag_BlockBufferValid; }

    BufferImpl* GetMaterialBlock(int bufferIndex)
    {
        if (!IsBlockBufferValid())
        {
            return nullptr;
        }
        return m_pMaterialBlockArray ? &m_pMaterialBlockArray[bufferIndex] : nullptr;
    }

    size_t GetMaterialBlockSize() const { return m_MaterialBlockSize; }

    const nn::gfx::TextureView* GetTextureView(int index) const { return m_ppTextureArray[index]; }

    void SetTexture(int index, const TextureRef& rRef)
    {
        const nn::gfx::TextureView* pOldView = m_ppTextureArray[index];
        const u64& rOldSlot = m_pTextureSlotArray[index];
        m_ppTextureArray[index] = rRef.GetTextureView();
        m_pTextureSlotArray[index] = rRef.GetDescriptorSlot();
        if (m_pTextureChangeCallback &&
            (pOldView != rRef.GetTextureView() || rOldSlot != m_pTextureSlotArray[index]))
        {
            m_pTextureChangeCallback(this, index);
        }
    }

private:
    enum Flag { Flag_BlockBufferValid = 1 << 0 };

    const ResMaterial* m_pRes;
    u8 m_Flag;
    u8 _9[0x40 - 0x9];
    BufferImpl* m_pMaterialBlockArray;
    u8 _48[0x50 - 0x48];
    const nn::gfx::TextureView** m_ppTextureArray;
    u64* m_pTextureSlotArray;
    size_t m_MaterialBlockSize;
    u8 _68[0x78 - 0x68];
    TextureChangeCallback m_pTextureChangeCallback;
};

class MaterialAnimObj {
public:
    const nn::gfx::TextureView* GetTextureView(int index) const { return m_ppTextureArray[index]; }
    void SetTexture(int index, const TextureRef& rRef)
    {
        m_ppTextureArray[index] = rRef.GetTextureView();
        m_pTextureSlotArray[index] = rRef.GetDescriptorSlot();
    }

private:
    u8 _0[0x90];
    const nn::gfx::TextureView** m_ppTextureArray;
    u64* m_pTextureSlotArray;
};

class ShapeObj {
public:
    bool IsBlockBufferValid() const { return m_Flag & Flag_BlockBufferValid; }
    bool IsViewDependent() const { return m_ViewDependent; }

    BufferImpl* GetShapeBlock(int viewIndex, int bufferIndex)
    {
        if (!IsBlockBufferValid())
        {
            return nullptr;
        }
        if (IsViewDependent())
        {
            return m_pShapeBlockArray ?
                       &m_pShapeBlockArray[viewIndex * m_BufferingCount + bufferIndex] :
                       nullptr;
        }
        return m_pShapeBlockArray ? &m_pShapeBlockArray[bufferIndex] : nullptr;
    }

private:
    enum Flag { Flag_BlockBufferValid = 1 << 0 };

    u8 _0[0x8];
    u8 m_Flag;
    u8 _9[0xd - 0x9];
    u8 m_ViewDependent;
    u8 _e;
    u8 m_BufferingCount;
    u8 _10[0x20 - 0x10];
    BufferImpl* m_pShapeBlockArray;
};

}  // namespace nn::g3d
