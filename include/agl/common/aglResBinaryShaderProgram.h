#pragma once

#include "common/aglResShaderSymbol.h"
#include "common/aglResShaderVariation.h"

namespace agl {

struct ResBinaryShaderProgramData {
    u32 mSize;
    u32 mNameLen;
    u32 mKind;
    u32 mBaseIndex;
    // char mName[];
};
static_assert(sizeof(ResBinaryShaderProgramData) == 0x10,
              "agl::ResBinaryShaderProgramData size mismatch");

class ResBinaryShaderProgram : public ResCommon<ResBinaryShaderProgramData> {
public:
    using ResCommon::ResCommon;

    const char* getName() const { return (const char*)(ptr() + 1); }

    ResShaderVariationArray getResShaderVariationArray() const {
        const DataType* const data = ptr();
        return reinterpret_cast<const ResShaderVariationArrayData*>(
            reinterpret_cast<const char*>(data + 1) + data->mNameLen);
    }

    ResShaderVariationArray getResShaderVariationDefaultArray() const {
        const ResShaderVariationArrayData* const data = getResShaderVariationArray().ptr();
        return reinterpret_cast<const ResShaderVariationArrayData*>(
            reinterpret_cast<const char*>(data) + data->mSize);
    }

    ResShaderSymbolArray getResShaderSymbolArray(ShaderSymbolType type) const;

    s32 getShaderBinaryIndex(s32 variation, ShaderType type) const {
        const DataType* const data = ptr();
        if (!(data->mKind & (1 << type))) {
            return -1;
        }

        switch (type) {
        case cShaderType_Vertex:
            return ((data->mKind & 4) ? 3 : 2) * variation + data->mBaseIndex;
        case cShaderType_Fragment:
            return ((data->mKind & 4) ? 3 : 2) * variation + data->mBaseIndex + 1;
        case cShaderType_Geometry:
            return 3 * variation + 2 + data->mBaseIndex;
        default:
            return data->mBaseIndex + variation;
        }
    }
};

using ResBinaryShaderProgramArray = ResArray<ResBinaryShaderProgram>;

using ResBinaryShaderProgramArrayData = ResBinaryShaderProgramArray::DataType;
static_assert(sizeof(ResBinaryShaderProgramArrayData) == 8,
              "agl::ResBinaryShaderProgramArrayData size mismatch");

}  // namespace agl
