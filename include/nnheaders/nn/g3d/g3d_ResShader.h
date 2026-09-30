#pragma once

#include <nn/types.h>
#include <nn/g3d/g3d_Resources.h>
#include <nn/util/AccessorBase.h>
#include <nn/util/util_BinTypes.h>
#include <nn/util/util_ResDic.h>

namespace nn::g3d {

enum ShaderStage {
    Stage_Vertex,
    Stage_Geometry,
    Stage_Pixel,
    Stage_Compute,
    Stage_Hull,
    Stage_Domain,
    Stage_Num
};

struct ResShaderProgramData {
    nn::util::BinTPtr<s32> pSamplerTable;
    nn::util::BinTPtr<s32> pImageTable;
    nn::util::BinTPtr<s32> pUniformBlockTable;
    u8 _18[0x30 - 0x18];
    u32 attribActiveFlag;
    u8 _34[12];
};

class ResShaderProgram : public nn::util::AccessorBase<ResShaderProgramData> {
public:
    // device prepares the selected GPU shader program.
    void Update(nn::gfx::Device* device);
    int GetSamplerLocation(int samplerIndex, ShaderStage stage) const {
        return ToData().pSamplerTable.Get()[samplerIndex * Stage_Num + stage];
    }

    int GetUniformBlockLocation(int blockIndex, ShaderStage stage) const {
        return ToData().pUniformBlockTable.Get()[blockIndex * Stage_Num + stage];
    }

    bool IsAttribActive(int attribIndex) const {
        return ToData().attribActiveFlag & (1 << attribIndex);
    }
};

struct ResAttribVarData {
    u8 index;
    s8 location;
};

class ResShaderOption {
public:
    // key is the packed shader key to edit; choice identifies a value in this option's dictionary.
    void WriteStaticKey(u32* key, int choice) const;
    // key contains packed option values; choice selects the value to encode.
    int ReadStaticKey(const u32* key) const;
    void WriteDynamicKey(u32* key, int choice) const;
    int ReadDynamicKey(const u32* key) const;
    // name selects a value in this shader option's choice dictionary.
    int FindChoiceIndex(const char* name) const {
        return choiceDictionary ? choiceDictionary->FindIndex(name) : nn::util::ResDic::Npos;
    }
    u8 _0[8];
    nn::util::ResDic* choiceDictionary;
    const u32* values;
    u8 _18;
    u8 defaultChoice;
    u16 blockOffset;
    u8 flags;
    u8 _1d[11];
};
struct ResUniformVar {
    u8 _0[12];
    u16 offset;
    u16 _e;
};
struct ResUniformBlock {
    ResUniformVar* uniforms;
    nn::util::ResDic* dictionary;
    const void* defaultValues;
    u16 _18;
    u16 size;
    u8 _1c[4];
    // name selects a uniform in the block; return null if it has no matching entry.
    const ResUniformVar* FindUniform(const char* name) const {
        int index = dictionary ? dictionary->FindIndex(name) : nn::util::ResDic::Npos;
        if (index == nn::util::ResDic::Npos) return nullptr;
        return &uniforms[index];
    }
};

struct ResShadingModelData {
    u8 _0[8];
    nn::util::BinTPtr<ResShaderOption> pStaticOptions;
    nn::util::BinTPtr<nn::util::ResDic> pStaticOptionDic;
    nn::util::BinTPtr<ResShaderOption> pDynamicOptions;
    u8 _20[8];
    nn::util::BinTPtr<ResAttribVarData> pAttribArray;
    nn::util::BinTPtr<nn::util::ResDic> pAttribDic;
    char _38[0x40 - 0x38];
    nn::util::BinTPtr<nn::util::ResDic> pSamplerDic;
    char _48[0x58 - 0x48];
    nn::util::BinTPtr<ResUniformBlock> pUniformBlocks;
    nn::util::BinTPtr<nn::util::ResDic> pUniformBlockDic;
    char _68[0x88 - 0x68];
    nn::util::BinTPtr<ResShaderProgram> pPrograms;
    nn::util::BinTPtr<u32> pKeyTable;
    char _98[0xe4 - 0x98];
    u16 staticOptionCount;
    u16 dynamicOptionCount;
    u16 programCount;
    u8 staticKeyLength;
    u8 dynamicKeyLength;
    u8 attribCount;
    u8 samplerCount;
    u8 _ee[2];
    s8 materialBlockIndex;
    u8 _f1[2];
    s8 optionBlockIndex;
    u8 _f4[4];
};

struct ShaderRange { const u32* begin; const u32* end; };

class ResShadingModel : public nn::util::AccessorBase<ResShadingModelData> {
public:
    // key receives the default packed static or dynamic shader option values.
    void WriteDefaultStaticKey(u32* key) const;
    void WriteDefaultDynamicKey(u32* key) const;
    void WriteInvalidDynamicKey(u32* key) const;
    // range limits program candidates; key supplies the requested dynamic options.
    int FindProgramIndex(const ShaderRange& range, const u32* key) const;
    // destination has capacity bytes; key contains length packed words.
    static int PrintKeyTo(char* destination, size_t capacity, const u32* key, int length);
    int PrintStaticOptionTo(char* destination, size_t capacity, const u32* key) const;
    int PrintDynamicOptionTo(char* destination, size_t capacity, const u32* key) const;
    int GetStaticKeyLength() const { return staticKeyLength; }
    // name selects a static shader option; return Npos for a missing entry.
    int FindStaticOptionIndex(const char* name) const {
        const nn::util::ResDic* dictionary = pStaticOptionDic.Get();
        return dictionary ? dictionary->FindIndex(name) : nn::util::ResDic::Npos;
    }
    // index selects a static option in the resource array.
    const ResShaderOption* GetStaticOption(int index) const { return &pStaticOptions.Get()[index]; }
    // name selects a static option; return null when its dictionary has no entry.
    const ResShaderOption* FindStaticOption(const char* name) const {
        int index = FindStaticOptionIndex(name);
        return index == nn::util::ResDic::Npos ? nullptr : GetStaticOption(index);
    }
    int GetMaterialBlockIndex() const { return materialBlockIndex; }
    // index selects a uniform block in the resource array.
    const ResUniformBlock* GetUniformBlock(int index) const { return &pUniformBlocks.Get()[index]; }

    int GetAttribCount() const { return ToData().attribCount; }
    int GetSamplerCount() const { return ToData().samplerCount; }

    const char* GetAttribName(int index) const {
        const nn::util::ResDic* pDic = ToData().pAttribDic.Get();
        return pDic ? pDic->GetKey(index).data() : nullptr;
    }
    const char* GetSamplerName(int index) const {
        const nn::util::ResDic* pDic = ToData().pSamplerDic.Get();
        return pDic ? pDic->GetKey(index).data() : nullptr;
    }
    const ResAttribVarData* GetAttrib(int index) const {
        return &ToData().pAttribArray.Get()[index];
    }

    int FindSamplerIndex(const char* pName) const {
        const nn::util::ResDic* pDic = ToData().pSamplerDic.Get();
        return pDic ? pDic->FindIndex(pName) : nn::util::ResDic::Npos;
    }

    int FindUniformBlockIndex(const char* pName) const {
        const nn::util::ResDic* pDic = ToData().pUniformBlockDic.Get();
        return pDic ? pDic->FindIndex(pName) : nn::util::ResDic::Npos;
    }
};

class ShadingModelObj {
public:
    struct InitializeArgument {
        const ResShadingModel* resource;
        size_t memorySize;
        size_t memoryAlignment;
        detail::WorkMemoryBlock blocks[4];
        int bufferCount;
        void CalculateMemorySize();
    };
    // argument describes the model and buffering; memory provides size bytes of workspace.
    bool Initialize(const InitializeArgument& argument, void* memory, size_t size);
    // device supplies GPU buffer alignment and allocation requirements.
    size_t GetBlockBufferAlignment(nn::gfx::Device* device) const;
    size_t CalculateBlockBufferSize(nn::gfx::Device* device);
    // device and pool own the buffer region starting at offset with size bytes available.
    void SetupBlockBufferImpl(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset, size_t size);
    bool SetupBlockBuffer(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset, size_t size);
    // device owns the buffers being finalized.
    void CleanupBlockBuffer(nn::gfx::Device* device);
    // bufferIndex selects the buffered option block to update.
    void CalculateOptionBlock(int bufferIndex);
    void ClearStaticKey();
    // option selects an option; choice identifies its value.
    void WriteStaticKey(int option, int choice);
    int ReadStaticKey(int option) const;
    const ResShadingModel* GetResource() const { return resource; }
    // pStr receives formatted text and strLength is its capacity in bytes.
    int PrintRawKeyTo(char* pStr, int strLength) const;
    int PrintKeyTo(char* pStr, int strLength) const;
    int PrintRawOptionTo(char* pStr, int strLength) const;
    int PrintOptionTo(char* pStr, int strLength) const;
private:
    friend class ShaderSelector;
    const ResShadingModel* resource;
    u32 flags;
    u32 _c;
    u32* staticKey;
    u32* optionKey;
    ShaderRange range;
    nn::gfx::MemoryPool* pool;
    ptrdiff_t poolOffset;
    BufferImpl* buffers;
    size_t blockSize;
    u8 bufferCount;
    u8 _51[7];
    detail::FlagSet dirtyFlags;
    void* user;
    void* work;
};

class ShaderSelector {
public:
    struct InitializeArgument {
        ShadingModelObj* model;
        size_t memorySize;
        size_t memoryAlignment;
        detail::WorkMemoryBlock blocks[3];
        void CalculateMemorySize();
    };
    // argument describes the model; memory supplies size bytes of workspace.
    bool Initialize(const InitializeArgument& argument, void* memory, size_t size);
    // device prepares the GPU program selected by the current options.
    bool UpdateVariation(nn::gfx::Device* device);
    void ClearDynamicKey();
    // option selects a dynamic option; choice identifies its value.
    void WriteDynamicKey(int option, int choice);
    int ReadDynamicKey(int option) const;
    // pStr receives formatted text and strLength is its capacity in bytes.
    int PrintRawKeyTo(char* pStr, int strLength) const;
    int PrintKeyTo(char* pStr, int strLength) const;
    int PrintRawOptionTo(char* pStr, int strLength) const;
    int PrintOptionTo(char* pStr, int strLength) const;
private:
    ShadingModelObj* model;
    u32* dynamicKey;
    u32* optionKey;
    u32* previousKey;
    ResShaderProgram* program;
    void* work;
};
static_assert(sizeof(ResShaderOption) == 0x28, "shader option size");
static_assert(sizeof(ResShaderProgram) == 0x40, "shader program size");
static_assert(sizeof(ShadingModelObj) == 0x88, "shading model size");
static_assert(sizeof(ShaderSelector) == 0x30, "shader selector size");
}  // namespace nn::g3d
