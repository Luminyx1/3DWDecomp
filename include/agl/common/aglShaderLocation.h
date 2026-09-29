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
    bool isValid() const { return mUniformLocation != -1; }
    bool operator==(const ShaderLocation& rOther) const
    {
        return mUniformLocation == rOther.mUniformLocation;
    }

protected:
    union {
        s8 mLocation[cShaderType_Num];
        s16 mRegisterLocation[2];
        s32 mUniformLocation;
    };
};

class UniformLocation : public ShaderLocation, public sead::INamable {
public:
    UniformLocation() : INamable("Undefined") {}

    void search(const ShaderProgram&);
    void setUniformNVN(DrawContext*, u32, const void*) const;

    void setUniform(DrawContext* pDrawContext, u32 num, const void* pData) const
    {
        if (isValid())
        {
            setUniformNVN(pDrawContext, num, pData);
        }
    }

    void setUniform(DrawContext* pDrawContext, f32 value) const
    {
        if (isValid())
        {
            setUniformNVN(pDrawContext, 1, &value);
        }
    }
};

class SamplerLocation : public ShaderLocation, public sead::INamable {
public:
    SamplerLocation() : INamable("Undefined") {}
    SamplerLocation(const ShaderLocation& rLocation, const sead::SafeString& rName)
        : ShaderLocation(rLocation), INamable(rName)
    {
    }

    void search(const ShaderProgram&);
};

class ImageLocation : public ShaderLocation, public sead::INamable {
public:
    ImageLocation() : INamable("Undefined") {}

    void search(const ShaderProgram&);
};

class UniformBlockLocation : public ShaderLocation, public sead::INamable {
public:
    UniformBlockLocation() : INamable("Undefined") {}
    UniformBlockLocation(const ShaderLocation& rLocation, const sead::SafeString& rName)
        : ShaderLocation(rLocation), INamable(rName)
    {
    }

    void search(const ShaderProgram&);
};

class ShaderStorageBlockLocation : public ShaderLocation, public sead::INamable {
public:
    ShaderStorageBlockLocation() : INamable("Undefined") {}

    void search(const ShaderProgram&);
};

class AttributeLocation : public ShaderLocation, public sead::INamable {
public:
    AttributeLocation() : INamable("Undefined") {}
    AttributeLocation(const sead::SafeString&, s32);

    void search(const ShaderProgram&);
};

}  // namespace agl
