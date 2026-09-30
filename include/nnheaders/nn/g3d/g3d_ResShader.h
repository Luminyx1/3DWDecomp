#pragma once

#include <nn/types.h>
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
};

class ResShaderProgram : public nn::util::AccessorBase<ResShaderProgramData> {
public:
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
    // name selects a value in this shader option's choice dictionary.
    int FindChoiceIndex(const char* name) const {
        return choiceDictionary ? choiceDictionary->FindIndex(name) : nn::util::ResDic::Npos;
    }
    u8 _0[8];
    nn::util::ResDic* choiceDictionary;
    u8 _10[0x18];
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
    u8 _18[0x10];
    nn::util::BinTPtr<ResAttribVarData> pAttribArray;
    nn::util::BinTPtr<nn::util::ResDic> pAttribDic;
    char _38[0x40 - 0x38];
    nn::util::BinTPtr<nn::util::ResDic> pSamplerDic;
    char _48[0x58 - 0x48];
    nn::util::BinTPtr<ResUniformBlock> pUniformBlocks;
    nn::util::BinTPtr<nn::util::ResDic> pUniformBlockDic;
    char _68[0xea - 0x68];
    u8 staticKeyLength;
    u8 dynamicKeyLength;
    u8 attribCount;
    u8 samplerCount;
    u8 _ee[2];
    s8 materialBlockIndex;
    u8 _f1[7];
};

class ResShadingModel : public nn::util::AccessorBase<ResShadingModelData> {
public:
    // key receives the default packed static or dynamic shader option values.
    void WriteDefaultStaticKey(u32* key) const;
    void WriteDefaultDynamicKey(u32* key) const;
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
    void ClearStaticKey();
    // option and choice identify the static option value to write into this object's key.
    void WriteStaticKey(int option, int choice);
    const ResShadingModel* GetResource() const { return resource; }

    int PrintRawKeyTo(char* pStr, int strLength) const;
    int PrintKeyTo(char* pStr, int strLength) const;
    int PrintRawOptionTo(char* pStr, int strLength) const;
    int PrintOptionTo(char* pStr, int strLength) const;
private:
    const ResShadingModel* resource;
};

class ShaderSelector {
public:
    int PrintRawKeyTo(char* pStr, int strLength) const;
    int PrintKeyTo(char* pStr, int strLength) const;
    int PrintRawOptionTo(char* pStr, int strLength) const;
    int PrintOptionTo(char* pStr, int strLength) const;
};

}  // namespace nn::g3d
