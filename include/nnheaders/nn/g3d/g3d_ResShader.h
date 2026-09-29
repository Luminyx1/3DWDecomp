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

struct ResShadingModelData {
    char _0[0x28];
    nn::util::BinTPtr<ResAttribVarData> pAttribArray;
    nn::util::BinTPtr<nn::util::ResDic> pAttribDic;
    char _38[0x40 - 0x38];
    nn::util::BinTPtr<nn::util::ResDic> pSamplerDic;
    char _48[0x60 - 0x48];
    nn::util::BinTPtr<nn::util::ResDic> pUniformBlockDic;
    char _68[0xec - 0x68];
    u8 attribCount;
    u8 samplerCount;
};

class ResShadingModel : public nn::util::AccessorBase<ResShadingModelData> {
public:
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
    int PrintRawKeyTo(char* pStr, int strLength) const;
    int PrintKeyTo(char* pStr, int strLength) const;
    int PrintRawOptionTo(char* pStr, int strLength) const;
    int PrintOptionTo(char* pStr, int strLength) const;
};

class ShaderSelector {
public:
    int PrintRawKeyTo(char* pStr, int strLength) const;
    int PrintKeyTo(char* pStr, int strLength) const;
    int PrintRawOptionTo(char* pStr, int strLength) const;
    int PrintOptionTo(char* pStr, int strLength) const;
};

}  // namespace nn::g3d
