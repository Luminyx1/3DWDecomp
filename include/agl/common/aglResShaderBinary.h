#pragma once

#include "common/aglResCommon.h"
#include "common/aglShaderEnum.h"

namespace agl {

#pragma pack(push, 4)

/// A symbol entry of a compiled shader whose name is stored as an offset until resolved.
struct ResShaderBinaryVar16 {
    const char* mName;
    u32 _8;
    u32 _c;
};
static_assert(sizeof(ResShaderBinaryVar16) == 0x10);

/// A smaller symbol entry of a compiled shader.
struct ResShaderBinaryVar12 {
    const char* mName;
    u32 _8;
};
static_assert(sizeof(ResShaderBinaryVar12) == 0xc);

/// Header of a compiled shader; pointer fields hold 32-bit offsets until resolvePtr is called.
struct ResShaderBinaryInfo {
    u64 mDataOffset;
    u32 _8;
    const void* mCode;
    s32 mVar0Num;
    ResShaderBinaryVar16* mVar0;
    s32 mVar1Num;
    ResShaderBinaryVar16* mVar1;
    s32 mVar2Num;
    ResShaderBinaryVar12* mVar2;
    s32 mVar3Num;
    ResShaderBinaryVar12* mVar3;
    s32 mVar4Num;
    ResShaderBinaryVar12* mVar4;
    s32 mVar5Num;
    ResShaderBinaryVar12* mVar5;
};
static_assert(sizeof(ResShaderBinaryInfo) == 0x5c);

#pragma pack(pop)

struct ResShaderBinaryData {
    u32 mSize;
    u32 mShaderType;
    s32 mDataOffset;  // Relative to end of struct
    u32 mDataSize;
};
static_assert(sizeof(ResShaderBinaryData) == 0x10, "agl::ResShaderBinaryData size mismatch");

class ResShaderBinary : public ResCommon<ResShaderBinaryData> {
public:
    using ResCommon::ResCommon;

    ShaderType getShaderType() const { return ShaderType(ref().mShaderType); }

    void* getData() const {
        const DataType* const data = ptr();
        return (void*)((uintptr_t)(data + 1) + data->mDataOffset);
    }

    ResShaderBinaryInfo* getInfo() const {
        return reinterpret_cast<ResShaderBinaryInfo*>(const_cast<DataType*>(ptr() + 1));
    }

    void modifyBinaryEndian();
    void resolvePtr(const void* pNameBase, bool);
    void setUp();
};

using ResShaderBinaryArray = ResArray<ResShaderBinary>;

using ResShaderBinaryArrayData = ResShaderBinaryArray::DataType;
static_assert(sizeof(ResShaderBinaryArrayData) == 8, "agl::ResShaderBinaryArrayData size mismatch");

}  // namespace agl
