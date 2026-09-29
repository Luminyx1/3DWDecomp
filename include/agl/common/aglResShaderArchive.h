#pragma once

#include "common/aglResCommon.h"
#include "common/aglResShaderProgram.h"
#include "common/aglResShaderSource.h"

namespace agl {

struct ResShaderUniformData {
    s32 mSize;
    s32 mLocation;
    u32 mNameLen;
    // char mName[];
};
static_assert(sizeof(ResShaderUniformData) == 0xc, "agl::ResShaderUniformData size mismatch");

class ResShaderUniform : public ResCommon<ResShaderUniformData> {
public:
    using ResCommon::ResCommon;

    const char* getName() const { return reinterpret_cast<const char*>(ptr() + 1); }
    s32 getLocation() const { return ref().mLocation; }
};

using ResShaderUniformArray = ResArray<ResShaderUniform>;

struct ResShaderUniformBlockData {
    s32 mSize;
    s32 mLocation;
    u32 mNameLen;
    // char mName[];
    // ResShaderUniformArrayData mUniforms;
};
static_assert(sizeof(ResShaderUniformBlockData) == 0xc,
              "agl::ResShaderUniformBlockData size mismatch");

class ResShaderUniformBlock : public ResCommon<ResShaderUniformBlockData> {
public:
    using ResCommon::ResCommon;

    const char* getName() const { return reinterpret_cast<const char*>(ptr() + 1); }
    s32 getLocation() const { return ref().mLocation; }

    ResShaderUniformArray getResShaderUniformArray() const {
        const DataType* const data = ptr();
        return reinterpret_cast<const char*>(data + 1) + data->mNameLen;
    }
};

using ResShaderUniformBlockArray = ResArray<ResShaderUniformBlock>;

struct ResShaderArchiveInfoData {
    u32 mSize;
    u32 _4;
    u32 _8;
    u32 _c;
};
static_assert(sizeof(ResShaderArchiveInfoData) == 0x10,
              "agl::ResShaderArchiveInfoData size mismatch");

class ResShaderArchiveInfo : public ResCommon<ResShaderArchiveInfoData> {
public:
    using ResCommon::ResCommon;
};

using ResShaderArchiveInfoArray = ResArray<ResShaderArchiveInfo>;

struct ResShaderArchiveData {
    union {
        char mSignature[4];
        u32 mSigWord;
    };
    u32 mVersion;
    u32 mFileSize;
    u32 mEndian;
    u32 mNameLen;
    // char mName[];

public:
    static u32 getVersion();
    static u32 getSignature();
    static const char* getExtension();

private:
    static const u32 cVersion = 13;
    static const u32 cSignature = 0x53484141;  // SHAA
#ifdef cafe
    static const u32 cEndianCheckBit = 0x01000001;
#endif
#ifdef SWITCH
    static const u32 cEndianCheckBit = 0x00000001;
#endif

    friend class ResCommon<ResShaderArchiveData>;
    friend class ResShaderArchive;
};
static_assert(sizeof(ResShaderArchiveData) == 0x14, "agl::ResShaderArchiveData size mismatch");

class ResShaderArchive : public ResCommon<ResShaderArchiveData> {
    AGL_RES_FILE_HEADER()

public:
    using ResCommon::ResCommon;

    const char* getName() const {
        const DataType* const data = ptr();
        return (const char*)(data + 1);
    }

    ResShaderProgramArray getResShaderProgramArray() const {
        const DataType* const data = ptr();
        return (const ResShaderProgramArrayData*)((uintptr_t)(data + 1) + data->mNameLen);
    }

    s32 getResShaderProgramNum() const { return getResShaderProgramArray().getNum(); }

    ResShaderSourceArray getResShaderSourceArray() const {
        const ResShaderProgramArrayData* const data = getResShaderProgramArray().ptr();
        return (const ResShaderSourceArrayData*)((uintptr_t)data + data->mSize);
    }

    s32 getResShaderSourceNum() const { return getResShaderSourceArray().getNum(); }

    ResShaderArchiveInfoArray getResShaderArchiveInfoArray() const {
        const ResShaderSourceArrayData* const data = getResShaderSourceArray().ptr();
        return (const ResShaderArchiveInfoArray::DataType*)((uintptr_t)data + data->mSize);
    }

    bool setUp();
};

}  // namespace agl
