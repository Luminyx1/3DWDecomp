#pragma once

#include <basis/seadTypes.h>

namespace agl {

class ShaderCompileInfo;

class Shader {
public:
    Shader();

    virtual ~Shader() {}

    virtual s32 getShaderType() const = 0;
    virtual s32 getShaderMode() const { return 4; }
    virtual s32 getRingItemSize() const { return 0; }

    /// Packed values taken from the header of a compiled shader binary.
    struct BinaryInfo {
        u64 _0 : 16;
        u64 _2 : 8;
        u64 : 24;
        u64 _6 : 6;
        u64 : 2;
        u64 _7 : 6;
        u64 : 2;
    };
    static_assert(sizeof(BinaryInfo) == 8);

    void setBinary(const void* pShaderBinary);
    void setBinaryInfo(const BinaryInfo& rInfo) { mBinaryInfo = rInfo; }

    const void* getShaderBinary() const { return mShaderBinary; }
    ShaderCompileInfo* getCompileInfo() const { return mCompileInfo; }
    void setCompileInfo(ShaderCompileInfo* pCompileInfo) { mCompileInfo = pCompileInfo; }

private:
    const void* mShaderBinary;
    ShaderCompileInfo* mCompileInfo;
    BinaryInfo mBinaryInfo;
};
static_assert(sizeof(Shader) == 0x20);

class VertexShader : public Shader {
public:
    ~VertexShader() override {}

    s32 getShaderType() const override { return 0; }
};

class FragmentShader : public Shader {
public:
    ~FragmentShader() override {}

    s32 getShaderType() const override { return 1; }
};

class GeometryShader : public Shader {
public:
    ~GeometryShader() override {}

    s32 getShaderType() const override { return 2; }
};

class ComputeShader : public Shader {
public:
    ~ComputeShader() override {}

    s32 getShaderType() const override { return 3; }
};

}  // namespace agl
