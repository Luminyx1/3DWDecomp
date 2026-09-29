#pragma once

#include <basis/seadTypes.h>
#include <prim/seadNamable.h>

#include "common/aglShaderEnum.h"

namespace agl {
class DrawContext;
class ShaderProgram;

class ShaderLocation {
public:
    ShaderLocation() : mUniformLocation(-1) {}

    void setLocation(s32);
    void setLocation(ShaderType, s32);
    void setRegisterLocation(ShaderType, s32);

    s32 getLocation(ShaderType type) const { return mLocation[type]; }

protected:
    union {
        s8 mLocation[cShaderType_Num];
        s16 mRegisterLocation[2];
        s32 mUniformLocation;
    };
};

class UniformLocation : public ShaderLocation, public sead::INamable {
public:
    void search(const ShaderProgram&);
    void setUniformNVN(DrawContext*, u32, const void*) const;
};

class SamplerLocation : public ShaderLocation, public sead::INamable {
public:
    void search(const ShaderProgram&);
};

class ImageLocation : public ShaderLocation, public sead::INamable {
public:
    void search(const ShaderProgram&);
};

class UniformBlockLocation : public ShaderLocation, public sead::INamable {
public:
    void search(const ShaderProgram&);
};

class ShaderStorageBlockLocation : public ShaderLocation, public sead::INamable {
public:
    void search(const ShaderProgram&);
};

class AttributeLocation : public ShaderLocation, public sead::INamable {
public:
    AttributeLocation(const sead::SafeString&, s32);

    void search(const ShaderProgram&);
};

}  // namespace agl
