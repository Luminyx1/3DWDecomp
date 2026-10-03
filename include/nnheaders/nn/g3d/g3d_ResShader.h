#pragma once

#include <nn/gfx/gfx_Buffer.h>
#include <nn/gfx/gfx_Types.h>
#include <nn/types.h>
#include <nn/g3d/g3d_Resources.h>
#include <nn/util/AccessorBase.h>
#include <nn/util/util_BinTypes.h>
#include <nn/util/util_ResDic.h>
#include <nn/gfx/gfx_ResShader.h>
#include <nn/gfx/gfx_CommandBuffer.h>
#include <nn/os/os_MutexTypes.h>

namespace nn::g3d {

class ResShadingModel;
struct ShaderSymbolInfo {
    nn::util::BinPtrToString* samplerNames;
    nn::util::BinPtrToString* imageNames;
    nn::util::BinPtrToString* uniformBlockNames;
    nn::util::BinPtrToString* storageBlockNames;
};
struct ResShaderArchive {
    using ProgramUpdate = void (*)(nn::gfx::Device*, ResShadingModel*, int);
    u8 _0[0x10];
    ResShadingModel* models;
    u8 _18[0x10];
    // device owns the GPU shader; model and index select the program to prepare.
    ProgramUpdate updateProgram;
    void* work;
    u8 _38[8];
    u16 modelCount;
    u16 flags;
    u8 _44[4];
    // device owns the shaders; memory provides size bytes for optional model mutexes.
    void Setup(nn::gfx::Device* device, void* memory, size_t size);
    // pool provides poolSize bytes at offset; memory provides size bytes of mutex workspace.
    void Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset,
               size_t poolSize, void* memory, size_t size);
    // device owns the programs and containers being finalized.
    void Cleanup(nn::gfx::Device* device);
};
struct ResShaderFile {
    nn::util::BinaryFileHeader fileHeader;
    ResShaderArchive* archive;
    void Relocate();
    void Unrelocate();
    // file is a mutable shader resource image whose pointers will be relocated.
    static ResShaderFile* ResCast(void* file);
    // file points to the header whose signature and format version are checked.
    static bool IsValid(const void* file);
    // device and pool provide poolSize bytes at offset; memory supplies size bytes for mutexes.
    void Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset,
               size_t poolSize, void* memory, size_t size);
};

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
    nn::util::BinTPtr<s32> pStorageBlockTable;
    nn::gfx::ResShaderVariation* variation;
    ResShadingModel* model;
    u32 attribActiveFlag;
    u16 flags;
    u8 _36[10];
};

class ResShaderProgram : public nn::util::AccessorBase<ResShaderProgramData> {
public:
    // device owns shader objects; type selects binary, intermediate, or source code.
    bool IsBinaryAvailable(nn::gfx::Device* device);
    bool InitializePerType(nn::gfx::Device* device, nn::gfx::ShaderCodeType type);
    void Cleanup(nn::gfx::Device* device);
    void Setup(nn::gfx::Device* device);
    // skipBinary forces initialization from intermediate or source code on device.
    void Initialize(nn::gfx::Device* device, bool skipBinary);
    void UpdateTable();
    // commandBuffer receives the initialized shader for all graphics stages.
    void Load(nn::gfx::CommandBuffer* commandBuffer) const;
    const nn::gfx::Shader* GetShader() const;
    nn::gfx::ShaderCodeType GetCodeType() const {
        return (flags & 4) ? nn::gfx::ShaderCodeType_Binary
             : (flags & 16) ? nn::gfx::ShaderCodeType_Ir
             : (flags & 8) ? nn::gfx::ShaderCodeType_Source : nn::gfx::ShaderCodeType_End;
    }
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
    // key contains the packed static option values to decode.
    int ReadStaticKey(const u32* key) const;
    // key contains dynamic option words; choice selects the value to encode.
    void WriteDynamicKey(u32* key, int choice) const;
    // key contains the packed dynamic option values to decode.
    int ReadDynamicKey(const u32* key) const;
    // name selects a value in this shader option's choice dictionary.
    int FindChoiceIndex(const char* name) const {
        return (choiceDictionary != nullptr) ? choiceDictionary->FindIndex(name) : nn::util::ResDic::Npos;
    }
    nn::util::BinTPtr<nn::util::BinString> name;
    nn::util::ResDic* choiceDictionary;
    const u32* values;
    u8 _18;
    u8 defaultChoice;
    u16 blockOffset;
    u8 flags;
    u8 dynamicWordOffset;
    u8 wordIndex;
    u8 shift;
    u32 mask;
    u8 _24[4];
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
    u16 uniformCount;
    u8 _1e[2];
    int GetUniformCount() const { return uniformCount; }
    // index selects a uniform in the block dictionary.
    const char* GetUniformName(int index) const {
        return (dictionary != nullptr) ? dictionary->GetKey(index).data() : nullptr;
    }
    // name selects a uniform in the block; return null if it has no matching entry.
    const ResUniformVar* FindUniform(const char* name) const {
        int index = (dictionary != nullptr) ? dictionary->FindIndex(name) : nn::util::ResDic::Npos;
        if (index == nn::util::ResDic::Npos) return nullptr;
        return &uniforms[index];
    }
};

struct ResShadingModelData {
    nn::util::BinPtr pName;
    nn::util::BinTPtr<ResShaderOption> pStaticOptions;
    nn::util::BinTPtr<nn::util::ResDic> pStaticOptionDic;
    nn::util::BinTPtr<ResShaderOption> pDynamicOptions;
    nn::util::BinTPtr<nn::util::ResDic> pDynamicOptionDic;
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
    ResShaderArchive* archive;
    ShaderSymbolInfo* symbolInfo;
    nn::gfx::ResShaderFile* shaderFile;
    nn::os::MutexType* mutex;
    u8 _b8[0xe0 - 0xb8];
    s32 defaultProgramIndex;
    u16 staticOptionCount;
    u16 dynamicOptionCount;
    u16 programCount;
    u8 staticKeyLength;
    u8 dynamicKeyLength;
    u8 attribCount;
    u8 samplerCount;
    u8 imageCount;
    u8 uniformBlockCount;
    s8 materialBlockIndex;
    s8 shapeBlockIndex;
    s8 skeletonBlockIndex;
    s8 optionBlockIndex;
    u8 storageBlockCount;
    u8 _f5[11];
};

struct ShaderRange { const u32* begin; const u32* end; };

class ResShadingModel : public nn::util::AccessorBase<ResShadingModelData> {
public:
    void Relocate();
    void Unrelocate();
    // device owns shader objects; mutex optionally serializes program updates.
    bool IsBinaryAvailable(nn::gfx::Device* device);
    void Setup(nn::gfx::Device* device, nn::os::MutexType* mutex);
    // pool supplies size bytes at offset; mutex optionally serializes updates on device.
    void Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset,
               size_t size, nn::os::MutexType* mutex);
    // device owns the shader objects being released or updated; index selects a program.
    void Cleanup(nn::gfx::Device* device);
    void UpdateProgram(nn::gfx::Device* device, int index);
    // index selects a program's packed key words.
    const u32* GetStaticKey(int index) const;
    const u32* GetDynamicKey(int index) const;
    const u32* GetKey(int index) const;
    // key supplies all static and dynamic option words for the requested program.
    int FindProgramIndex(const u32* key) const;
    // range receives programs sharing the static option words in key.
    bool FindProgramRange(ShaderRange* range, const u32* key) const;
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
    int GetDynamicKeyLength() const { return dynamicKeyLength; }
    int GetStaticOptionCount() const { return staticOptionCount; }
    int GetDynamicOptionCount() const { return dynamicOptionCount; }
    // index selects a dynamic option in the resource array.
    const ResShaderOption* GetDynamicOption(int index) const { return &pDynamicOptions.Get()[index]; }
    // index selects a program; shader initialization may mutate the GPU object through a const resource.
    ResShaderProgram* GetProgram(int index) const {
        return const_cast<ResShaderProgram*>(&pPrograms.Get()[index]);
    }
    // name selects a static shader option; return Npos for a missing entry.
    int FindStaticOptionIndex(const char* name) const {
        const nn::util::ResDic* dictionary = pStaticOptionDic.Get();
        return (dictionary != nullptr) ? dictionary->FindIndex(name) : nn::util::ResDic::Npos;
    }
    // index selects a static option in the resource array.
    const ResShaderOption* GetStaticOption(int index) const { return &pStaticOptions.Get()[index]; }
    // name selects a static option; return null when its dictionary has no entry.
    const ResShaderOption* FindStaticOption(const char* name) const {
        int index = FindStaticOptionIndex(name);
        return index == nn::util::ResDic::Npos ? nullptr : GetStaticOption(index);
    }
    ResShaderOption* GetStaticOption(int index) { return &pStaticOptions.Get()[index]; }
    NOINLINE ResShaderOption* FindStaticOption(const char* name) {
        int index = FindStaticOptionIndex(name);
        return index == nn::util::ResDic::Npos ? nullptr : GetStaticOption(index);
    }
    // name selects a dynamic shader option; return Npos for a missing entry.
    int FindDynamicOptionIndex(const char* name) const {
        const nn::util::ResDic* dictionary = pDynamicOptionDic.Get();
        return (dictionary != nullptr) ? dictionary->FindIndex(name) : nn::util::ResDic::Npos;
    }
    const char* GetName() const { return static_cast<const char*>(pName.Get()) + 2; }
    // name selects a uniform block; return null when its dictionary has no entry.
    const ResUniformBlock* FindUniformBlock(const char* name) const {
        int index = FindUniformBlockIndex(name);
        return index == nn::util::ResDic::Npos ? nullptr : GetUniformBlock(index);
    }
    int GetMaterialBlockIndex() const { return materialBlockIndex; }
    int GetShapeBlockIndex() const { return shapeBlockIndex; }
    int GetSkeletonBlockIndex() const { return skeletonBlockIndex; }
    int GetOptionBlockIndex() const { return optionBlockIndex; }
    size_t GetUniformBlockSize(int index) const { return pUniformBlocks.Get()[index].size; }
    const char* GetUniformBlockName(int index) const {
        const nn::util::ResDic* pDic = pUniformBlockDic.Get();
        return pDic ? pDic->GetKey(index).data() : nullptr;
    }
    // index selects a uniform block in the resource array.
    const ResUniformBlock* GetUniformBlock(int index) const { return &pUniformBlocks.Get()[index]; }

    int GetAttribCount() const { return ToData().attribCount; }
    int GetSamplerCount() const { return ToData().samplerCount; }

    const char* GetAttribName(int index) const {
        const nn::util::ResDic* pDic = ToData().pAttribDic.Get();
        return (pDic != nullptr) ? pDic->GetKey(index).data() : nullptr;
    }
    const char* GetSamplerName(int index) const {
        const nn::util::ResDic* pDic = ToData().pSamplerDic.Get();
        return (pDic != nullptr) ? pDic->GetKey(index).data() : nullptr;
    }
    const ResAttribVarData* GetAttrib(int index) const {
        return &ToData().pAttribArray.Get()[index];
    }

    int FindSamplerIndex(const char* pName) const {
        const nn::util::ResDic* pDic = ToData().pSamplerDic.Get();
        return (pDic != nullptr) ? pDic->FindIndex(pName) : nn::util::ResDic::Npos;
    }

    int FindUniformBlockIndex(const char* pName) const {
        const nn::util::ResDic* pDic = ToData().pUniformBlockDic.Get();
        return (pDic != nullptr) ? pDic->FindIndex(pName) : nn::util::ResDic::Npos;
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
    // Builder computes the workspace layout of a shading model object.
    class Builder : public InitializeArgument {
    public:
        /**
         * @brief Initialize an argument for a single-buffered shading model object.
         * @param pResource Shading model resource.
         */
        explicit Builder(const ResShadingModel* pResource) {
            resource = pResource;
            for (int i = 0; i < 4; ++i) {
                blocks[i].Initialize(0);
            }
            bufferCount = 1;
            memorySize = 0;
            memoryAlignment = 0;
        }
        size_t GetWorkMemorySize() const { return memorySize; }
        bool Build(ShadingModelObj* pObj, void* pMemory, size_t size) const {
            return pObj->Initialize(*this, pMemory, size);
        }
    };
    /**
     * @brief Construct an uninitialized shading model object.
     */
    ShadingModelObj()
        : resource(nullptr), flags(0), staticKey(nullptr), optionKey(nullptr), range{nullptr, nullptr},
          pool(nullptr), poolOffset(0), buffers(nullptr), blockSize(0), bufferCount(0), user(nullptr),
          work(nullptr) {}
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
    /**
     * @brief Update the range of programs matching the current static key.
     * @return True if a matching program range was found.
     */
    bool UpdateShaderRange() { return resource->FindProgramRange(&range, staticKey); }
    const ResShadingModel* GetResource() const { return resource; }
    const gfx::Buffer* GetOptionBlock() const { return reinterpret_cast<const gfx::Buffer*>(buffers); }
    bool IsBlockBufferValid() const { return (flags & 1) != 0; }
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
    /**
     * @brief Construct an uninitialized shader selector.
     */
    ShaderSelector()
        : model(nullptr), dynamicKey(nullptr), optionKey(nullptr), previousKey(nullptr),
          program(nullptr), work(nullptr) {}
    const ShadingModelObj* GetShadingModel() const { return model; }
    ShadingModelObj* GetShadingModel() { return model; }
    const ResShaderProgram* GetProgram() const { return program; }
    struct InitializeArgument {
        ShadingModelObj* model;
        size_t memorySize;
        size_t memoryAlignment;
        detail::WorkMemoryBlock blocks[3];
        void CalculateMemorySize();
    };
    // Builder computes the workspace layout of a shader selector.
    class Builder : public InitializeArgument {
    public:
        /**
         * @brief Initialize an argument for a shader selector.
         * @param pModel Shading model object the selector selects programs of.
         */
        explicit Builder(ShadingModelObj* pModel) {
            model = pModel;
            memorySize = 0;
            memoryAlignment = 0;
            for (int i = 0; i < 3; ++i) {
                blocks[i].Initialize(0);
            }
        }
        size_t GetWorkMemorySize() const { return memorySize; }
        bool Build(ShaderSelector* pSelector, void* pMemory, size_t size) const {
            return pSelector->Initialize(*this, pMemory, size);
        }
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
static_assert(sizeof(ResShadingModel) == 0x100, "shading model resource size");
static_assert(sizeof(ResShaderProgram) == 0x40, "shader program size");
static_assert(sizeof(ShadingModelObj) == 0x88, "shading model size");
static_assert(sizeof(ShaderSelector) == 0x30, "shader selector size");
}  // namespace nn::g3d
