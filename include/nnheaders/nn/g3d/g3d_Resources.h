#pragma once

// Shared G3D resource and object declarations used by NintendoWare and AGL.

#include <nn/gfx/gfx_Buffer.h>
#include <nn/gfx/gfx_BufferInfo.h>
#include <nn/util/util_BitFlagSet.h>
#include <nn/gfx/gfx_DescriptorSlot.h>
#include <nn/gfx/gfx_MemoryPoolInfo.h>
#include <nn/gfx/gfx_ResTexture.h>
#include <nn/gfx/gfx_Texture.h>
#include <nn/gfx/gfx_Types.h>
#include <nn/types.h>
#include <nn/g3d/g3d_Flag.h>
#include <nn/util/util_BinTypes.h>
#include <nn/util/util_BinaryFormat.h>
#include <nn/util/util_ResDic.h>

namespace nn::g3d {

class MaterialObj;
class ResSkeleton;
class ResSkeletalAnim;
class ResMaterialAnim;
class ResShapeAnim;
class ResSceneAnim;

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

    BindResult() : m_Flag(0), m_Pad(0) {}
    // flags records the success and failure bits for a single binding attempt.
    explicit BindResult(u32 flags) : m_Flag(flags), m_Pad(0) {}
    // result contributes success/failure bits from another resource binding operation.
    void Merge(const BindResult& result) { m_Flag |= result.m_Flag; }
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
    nn::util::BinTPtr<nn::util::BinPtrToString> pOptionArray;
    nn::util::BinTPtr<nn::util::ResDic> pOptionDic;
    u8 _40[6];
    u16 optionCount;
};
class ResShaderAssign : public nn::util::AccessorBase<ResShaderAssignData> {
public:
    // index selects a shader option name in the assignment dictionary.
    const char* GetOptionName(int index) const {
        const nn::util::ResDic* dictionary = pOptionDic.Get();
        return dictionary ? dictionary->GetKey(index).data() : nullptr;
    }
};
class ResShaderParam;
struct ResShaderParamData {
    size_t (*callback)(void*, const void*, const ResShaderParam*, void*);
    nn::util::BinPtrToString name;
    u8 type;
    u8 _11;
    u16 sourceOffset;
    s32 offset;
    u16 _18;
    u16 dependencyIndex;
    u8 _1c[4];
};

class ResShaderParam : public nn::util::AccessorBase<ResShaderParamData> {
public:
    enum Type { Type_Bool };
    // type selects the source parameter representation whose byte size is returned.
    static size_t GetSrcSize(Type type);
    // destination receives the GPU representation of source; swap enables byte swapping.
    template <bool swap> void Convert(void* destination, const void* source) const;
};

struct ResMaterialData {
    u8 _0[0x20];
    nn::util::BinTPtr<ResShaderAssignData> pShaderAssign;
    nn::util::BinTPtr<const nn::gfx::TextureView*> pTextureArray;
    nn::util::BinTPtr<nn::util::BinPtrToString> pTextureNameArray;
    nn::util::BinTPtr<ResSamplerData> pSamplerArray;
    u8 _40[0x48 - 0x40];
    nn::util::BinTPtr<nn::util::ResDic> pSamplerDic;
    nn::util::BinTPtr<ResShaderParamData> pShaderParamArray;
    u8 _58[8];
    nn::util::BinPtr pSourceParamData;
    u8 _68[0x10];
    nn::util::BinTPtr<u32> pVolatileParamFlags;
    u8 _80[8];
    nn::util::BinTPtr<u64> pSamplerSlotArray;
    nn::util::BinTPtr<u64> pTextureSlotArray;
    u8 _98[0x9c - 0x98];
    u8 samplerCount;
    u8 textureCount;
    u16 shaderParamCount;
    u16 volatileParamCount;
    u16 sourceParamSize;
    u16 materialBlockSize;
    u8 _a6[2];
};

class ResMaterial : public nn::util::AccessorBase<ResMaterialData> {
public:
    BindResult BindTexture(TextureBindCallback callback, void* user);
    bool ForceBindTexture(const TextureRef& texture, const char* name);
    void ReleaseTexture();
    void Setup(nn::gfx::Device* device);
    void Cleanup(nn::gfx::Device* device);
    void Reset();
    void Reset(u32 guard);
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
    u8 flags;
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
    nn::util::BinTPtr<nn::gfx::MemoryPool> pMemoryPool;
    nn::util::BinTPtr<nn::gfx::Buffer> pBufferObjects;
    nn::util::BinTPtr<nn::gfx::Buffer*> pVertexBufferArray;
    nn::util::BinTPtr<ResVertexBufferInfoData> pVertexBufferInfoArray;
    nn::util::BinTPtr<ResVertexBufferStrideData> pVertexBufferStrideArray;
    nn::util::BinPtr pUserPtr;
    u32 memoryPoolOffset;
    u8 attribCount;
    u8 bufferCount;
    u8 _4e[0xa];
};

class ResVertex : public nn::util::AccessorBase<ResVertexData> {
public:
    void Setup(nn::gfx::Device* device);
    void Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset);
    void Cleanup(nn::gfx::Device* device);
    void Reset();
    void Reset(u32 guard);
    void CalculateDynamicVertexBufferIndex(nn::util::BitFlagSet<255, void>* indices) const;
};

struct ResSubMesh { u32 offset; u32 count; };
class ResMesh {
public:
    void Setup(nn::gfx::Device* device);
    void Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset);
    void Cleanup(nn::gfx::Device* device);
    void DrawSubMesh(nn::gfx::CommandBuffer* command, int first, int count, int instances) const;
    void DrawSubMesh(nn::gfx::CommandBuffer* command, int first, int count, int instances, int baseInstance) const;

    ResSubMesh* subMeshes;
    nn::gfx::MemoryPool* memoryPool;
    nn::gfx::Buffer* buffer;
    nn::gfx::BufferInfo* bufferInfo;
    u32 memoryPoolOffset;
    nn::gfx::PrimitiveTopology topology;
    nn::gfx::IndexFormat indexFormat;
    u32 indexCount;
    u32 baseVertex;
    u16 subMeshCount;
    u16 _36;
};

struct ResKeyShape { u8 attribIndices[18]; u8 _12[2]; };
struct ResShapeData {
    u32 signature;
    u32 flags;
    nn::util::BinPtr pName;
    nn::util::BinTPtr<ResVertex> pVertex;
    nn::util::BinTPtr<ResMesh> pMeshArray;
    nn::util::BinPtr pSkinBoneIndexArray;
    nn::util::BinTPtr<ResKeyShape> pKeyShapeArray;
    nn::util::BinTPtr<nn::util::ResDic> pKeyShapeDic;
    u8 _38[0x48 - 0x38];
    nn::util::BinPtr pUserPtr;
    u16 index;
    u8 _52[9];
    u8 meshCount;
    u8 keyShapeCount;
    u8 _5d[3];
};

class ResShape : public nn::util::AccessorBase<ResShapeData> {
public:
    // name identifies a key shape in the resource dictionary.
    int FindKeyShapeIndex(const char* name) const {
        const nn::util::ResDic* dictionary = pKeyShapeDic.Get();
        if (!dictionary) return nn::util::ResDic::Npos;
        return dictionary->FindIndex(name);
    }
    int GetIndex() const { return index; }

    void ActivateDynamicVertexAttrForShapeAnim();
    void Setup(nn::gfx::Device* device);
    void Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset);
    void Cleanup(nn::gfx::Device* device);
    void Reset();
    void Reset(u32 guard);
};

struct ResModelData {
    u8 _0[0x18];
    nn::util::BinTPtr<ResSkeleton> pSkeleton;
    nn::util::BinTPtr<ResVertex> pVertexArray;
    nn::util::BinTPtr<ResShape> pShapeArray;
    nn::util::BinTPtr<nn::util::ResDic> pShapeDic;
    nn::util::BinTPtr<ResMaterial> pMaterialArray;
    u8 _40[0x60 - 0x40];
    nn::util::BinPtr pUserPtr;
    u16 vertexCount;
    u16 shapeCount;
    u16 materialCount;
    u8 _6e[0x78 - 0x6e];
};

class ResModel : public nn::util::AccessorBase<ResModelData> {
public:
    // name identifies a shape in this model; return null if its dictionary has no entry.
    __attribute__((noinline)) const ResShape* FindShape(const char* name) const {
        const nn::util::ResDic* dictionary = pShapeDic.Get();
        int index = dictionary ? dictionary->FindIndex(name) : nn::util::ResDic::Npos;
        if (index == nn::util::ResDic::Npos) return nullptr;
        return &pShapeArray.Get()[index];
    }

    BindResult BindTexture(TextureBindCallback callback, void* user);
    bool ForceBindTexture(const TextureRef& texture, const char* name);
    void ReleaseTexture();
    void ActivateDynamicVertexAttrForShapeAnim();
    void Setup(nn::gfx::Device* device);
    void Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset);
    void Cleanup(nn::gfx::Device* device);
    void Reset();
    void Reset(u32 guard);
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
    u8 _30[0x58 - 0x30];
    nn::util::BinTPtr<ResSkeletalAnim> pSkeletalAnimArray;
    nn::util::BinPtr pSkeletalAnimDic;
    nn::util::BinTPtr<ResMaterialAnim> pMaterialAnimArray;
    nn::util::BinPtr pMaterialAnimDic;
    u8 _78[0x88 - 0x78];
    nn::util::BinTPtr<ResShapeAnim> pShapeAnimArray;
    nn::util::BinPtr pShapeAnimDic;
    nn::util::BinTPtr<ResSceneAnim> pSceneAnimArray;
    nn::util::BinPtr pSceneAnimDic;
    nn::util::BinTPtr<nn::gfx::MemoryPool> pMemoryPool;
    nn::util::BinTPtr<nn::gfx::MemoryPoolInfo> pMemoryPoolInfo;
    nn::util::BinTPtr<ResExternalFileData> pExternalFileArray;
    nn::util::BinTPtr<nn::util::ResDic> pExternalFileDic;
    nn::util::BinPtr pUserPtr;
    u8 _d0[0xdc - 0xd0];
    u16 modelCount;
    u8 _de[4];
    u16 skeletalAnimCount;
    u16 materialAnimCount;
    u16 boneVisibilityAnimCount;
    u16 shapeAnimCount;
    u16 sceneAnimCount;
    u16 externalFileCount;
};

class ResFile : public nn::util::AccessorBase<ResFileData> {
public:
    static bool IsValid(const void* file);
    void Relocate();
    void Unrelocate();
    static ResFile* ResCast(void* pFile);
    void ReleaseTexture();
    void Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset, size_t size);
    void Reset();

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
    BindResult BindTexture(TextureBindCallback callback, void* user);
    void ReleaseTexture();
    void Reset();
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

namespace detail {
struct WorkMemoryBlock {
    size_t size;
    size_t alignment;
    void* pointer;
    ptrdiff_t offset;
    // bytes is the requested size; each block starts at an eight-byte boundary.
    void Initialize(size_t bytes) { size = bytes; alignment = 8; pointer = nullptr; offset = -1; }
    // buffer is the base of the allocated work area; empty blocks return null.
    void* GetPointer(void* buffer) const { return buffer && size ? static_cast<u8*>(buffer) + offset : nullptr; }
};
}
class MaterialObj {
public:
    struct InitializeArgument {
        const ResMaterial* resource;
        int bufferCount;
        size_t memorySize;
        size_t memoryAlignment;
        detail::WorkMemoryBlock blocks[5];
        void CalculateMemorySize();
    };
    bool Initialize(const InitializeArgument& argument, void* memory, size_t size);
    void InitializeDependPointer();
    size_t GetBlockBufferAlignment(nn::gfx::Device* device) const;
    size_t CalculateBlockBufferSize(nn::gfx::Device* device) const;
    bool SetupBlockBuffer(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset, size_t size);
    void SetupBlockBufferImpl(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset, size_t size);
    void CleanupBlockBuffer(nn::gfx::Device* device);
    void ResetDirtyFlags();
    void CalculateMaterial(int bufferIndex);
    template <bool swap> __attribute__((noinline)) void ConvertDirtyParams(void* destination, u32* dirtyFlags);
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
    int GetBufferingCount() const { return m_BufferingCount; }

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
    u16 m_Flag;
    u8 m_BufferingCount;
    u8 _b[5];
    detail::FlagSet m_DirtyFlags;
    nn::gfx::MemoryPool* m_pMemoryPool;
    ptrdiff_t m_MemoryPoolOffset;
    BufferImpl* m_pMaterialBlockArray;
    void* m_pParamSource;
    const nn::gfx::TextureView** m_ppTextureArray;
    u64* m_pTextureSlotArray;
    size_t m_MaterialBlockSize;
    void* m_pCallbackUserData;
    void* m_pWorkMemory;
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
