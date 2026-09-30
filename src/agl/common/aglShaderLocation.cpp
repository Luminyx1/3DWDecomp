#include "common/aglShaderLocation.h"

#include <cstring>
#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglDrawContext.h"
#include "common/aglResShaderArchive.h"
#include "common/aglShaderCompileInfo.h"
#include "common/aglShaderProgram.h"

namespace agl
{

namespace detail
{
class DynamicUniformBlock
{
public:
    static DynamicUniformBlock* sInstance;

    u8 _0[0x38];
    u64 mSize;
    u8 _40[0x90 - 0x40];
    NVNbufferAddress mAddress;
};
}  // namespace detail

namespace
{

struct ReflectionEntry
{
    const char* mName;
    s32 mLocation;
} __attribute__((packed));

struct ReflectionBlockEntry
{
    const char* mName;
    s32 mLocation;
    u32 mSize;
} __attribute__((packed));

template <typename Entry>
struct ReflectionList
{
    u32 mNum;
    const Entry* mEntries;
} __attribute__((packed));

struct ShaderReflection
{
    u8 _0[0x14];
    ReflectionList<ReflectionBlockEntry> mUniformBlocks;
    ReflectionList<ReflectionBlockEntry> mShaderStorageBlocks;
    ReflectionList<ReflectionEntry> mAttributes;
    ReflectionList<ReflectionEntry> mUniforms;
    ReflectionList<ReflectionEntry> mSamplers;
    ReflectionList<ReflectionEntry> mImages;
} __attribute__((packed));

template <typename Entry>
s32 findLocation(const sead::INamable& rLoc, const ShaderProgram& rProgram, ShaderType type,
                ReflectionList<Entry> ShaderReflection::*pList)
{
    if (!rProgram.hasStage(type))
    {
        return -1;
    }

    const auto* reflection =
        static_cast<const ShaderReflection*>(rProgram.getShader(type).getShaderBinary());
    if (!reflection)
    {
        return -1;
    }

    const char* name = rLoc.getName().cstr();
    const ReflectionList<Entry>& list = reflection->*pList;

    for (u32 i = 0; i < list.mNum; i++)
    {
        if (strcmp(list.mEntries[i].mName, name) == 0)
        {
            if (&list.mEntries[i])
            {
                return list.mEntries[i].mLocation;
            }

            break;
        }
    }

    return -1;
}

const char* getUniformName(const ResShaderUniformData* pData)
{
    return reinterpret_cast<const char*>(pData + 1);
}

template <typename T>
const T* nextUniform(const T* pData)
{
    return reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(pData) + pData->mSize);
}

s32 searchRegisterUniform(const sead::INamable& rLoc, const ShaderProgram& rProgram)
{
    const auto* array = static_cast<const ResShaderUniformBlockArray::DataType*>(
        rProgram.getRegisterUniformArray());
    if (!array)
    {
        return -1;
    }

    u32 num = array->mNum;
    const auto* block = reinterpret_cast<const ResShaderUniformBlockData*>(array + 1);

    for (u32 i = 0; i != num; ++i)
    {
        const char* blockName = reinterpret_cast<const char*>(block + 1);

        if (ShaderCompileInfo::getRegitserUniformBlockName().isEqual(sead::SafeString(blockName)))
        {
            const auto* uniforms = reinterpret_cast<const ResShaderUniformArray::DataType*>(
                blockName + block->mNameLen);
            u32 uniformNum = uniforms->mNum;
            const auto* uniform = reinterpret_cast<const ResShaderUniformData*>(uniforms + 1);

            for (u32 j = 0; j != uniformNum; ++j)
            {
                if (rLoc.getName().isEqual(sead::SafeString(getUniformName(uniform))))
                {
                    return uniform->mLocation;
                }

                uniform = nextUniform(uniform);
            }
        }

        block = nextUniform(block);
    }

    return -1;
}

}  // namespace

/**
 * Sets the location for all shader stages.
 * @param location location to set
 */
void ShaderLocation::setLocation(s32 location)
{
    for (s32 i = 0; i < cShaderType_Num; ++i)
    {
        mLocation[i] = location;
    }
}

/**
 * Sets the location for one shader stage.
 * @param type shader stage
 * @param location location to set
 */
void ShaderLocation::setLocation(ShaderType type, s32 location)
{
    mLocation[type] = location;
}

/**
 * Sets the register location for the vertex or non-vertex stages.
 * @param type shader stage
 * @param location register location to set
 */
void ShaderLocation::setRegisterLocation(ShaderType type, s32 location)
{
    mRegisterLocation[type != cShaderType_Vertex] = location;
}

void UniformLocation::search(const ShaderProgram& rProgram)
{
    for (s32 i = 0; i < cShaderType_Num; ++i)
    {
        auto type = static_cast<ShaderType>(i);

        if (!rProgram.hasStage(type))
        {
            continue;
        }

        const auto* reflection =
            static_cast<const ShaderReflection*>(rProgram.getShader(type).getShaderBinary());
        const char* name = getName().cstr();
        const auto& list = reflection->mUniforms;

        for (u32 j = 0; j < list.mNum; j++)
        {
            if (strcmp(list.mEntries[j].mName, name) == 0)
            {
                if (&list.mEntries[j])
                {
                    mUniformLocation = list.mEntries[j].mLocation;
                    return;
                }

                break;
            }
        }
    }

    mUniformLocation = searchRegisterUniform(*this, rProgram);
}

/**
 * Updates the uniform in the dynamic uniform buffer.
 * @param pCtx draw context
 * @param num number of 32-bit words to write
 * @param pData data to write
 */
void UniformLocation::setUniformNVN(DrawContext* pCtx, u32 num, const void* pData) const
{
    nvnCommandBufferUpdateUniformBuffer(
        *reinterpret_cast<NVNcommandBuffer* const*>(reinterpret_cast<uintptr_t>(pCtx) + 0xb8),
        detail::DynamicUniformBlock::sInstance->mAddress,
        detail::DynamicUniformBlock::sInstance->mSize, mUniformLocation, num * sizeof(u32),
        pData);
}

/**
 * Looks up the sampler's location in every stage of a shader program.
 * @param rProgram shader program to search
 */
void SamplerLocation::search(const ShaderProgram& rProgram)
{
    for (s32 i = 0; i < cShaderType_Num; ++i)
    {
        auto type = static_cast<ShaderType>(i);
        setLocation(type, findLocation(*this, rProgram, type, &ShaderReflection::mSamplers));
    }
}

/**
 * Looks up the image's location in every stage of a shader program.
 * @param rProgram shader program to search
 */
void ImageLocation::search(const ShaderProgram& rProgram)
{
    for (s32 i = 0; i < cShaderType_Num; ++i)
    {
        auto type = static_cast<ShaderType>(i);
        setLocation(type, findLocation(*this, rProgram, type, &ShaderReflection::mImages));
    }
}

/**
 * Looks up the uniform block's location in every stage of a shader program.
 * @param rProgram shader program to search
 */
void UniformBlockLocation::search(const ShaderProgram& rProgram)
{
    for (s32 i = 0; i < cShaderType_Num; ++i)
    {
        auto type = static_cast<ShaderType>(i);
        setLocation(type, findLocation(*this, rProgram, type, &ShaderReflection::mUniformBlocks));
    }
}

/**
 * Looks up the storage block's location in every stage of a shader program.
 * @param rProgram shader program to search
 */
void ShaderStorageBlockLocation::search(const ShaderProgram& rProgram)
{
    for (s32 i = 0; i < cShaderType_Num; ++i)
    {
        auto type = static_cast<ShaderType>(i);
        setLocation(type, findLocation(*this, rProgram, type, &ShaderReflection::mShaderStorageBlocks));
    }
}

/**
 * Constructs an attribute location with a vertex stage location.
 * @param rName attribute name
 * @param location vertex stage location
 */
AttributeLocation::AttributeLocation(const sead::SafeString& rName, s32 location)
    : INamable(rName)
{
    setLocation(cShaderType_Vertex, location);
}

/**
 * Looks up the attribute's location in every stage of a shader program.
 * @param rProgram shader program to search
 */
void AttributeLocation::search(const ShaderProgram& rProgram)
{
    for (s32 i = 0; i < cShaderType_Num; ++i)
    {
        auto type = static_cast<ShaderType>(i);
        setLocation(type, findLocation(*this, rProgram, type, &ShaderReflection::mAttributes));
    }
}

}  // namespace agl
