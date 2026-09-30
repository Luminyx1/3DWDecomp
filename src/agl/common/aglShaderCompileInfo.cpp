#include "common/aglShaderCompileInfo.h"
#include "detail/aglPrivateResource.h"
#include "detail/aglShaderTextUtil.h"
#include "shader_text/aglShaderTextPreprocessor.h"

namespace agl {

namespace {

const sead::SafeString cRegisterUniformBlockName = "RegisterUBO";

// TODO: use sead::PtrArray<const char>::data() once it compiles for const element types.
class MacroArray : public sead::PtrArrayImpl {
public:
    static const char* const* getData(const sead::PtrArray<const char>& rArray)
    {
        return reinterpret_cast<const char* const*>(static_cast<const MacroArray&>(
                                                        static_cast<const sead::PtrArrayImpl&>(rArray))
                                                        .mPtrs);
    }
};

const char* const cVersionText[ShaderCompileInfo::cTarget_Num] = {
    "#version 400\n"
    "#extension GL_ARB_texture_cube_map_array : enable\n"
    "#extension GL_ARB_shading_language_420pack : enable\n"
    "#extension GL_ARB_shader_image_load_store : enable\n"
    "#extension GL_ARB_shader_image_size : enable\n"
    "#extension GL_ARB_shader_storage_buffer_object : enable\n",
    "#version 330\n"
    "#extension GL_ARB_texture_cube_map_array : enable\n",
    "#version 300 es\n",
    "#version 100\n",
    "#version 440 core\n"
    "#extension GL_NV_gpu_shader5:require\n",
};

const char* const cShaderTypeText[ShaderCompileInfo::cTarget_Num][cShaderType_Num] = {
    {
        "#define AGL_VERTEX_SHADER\n"
        "#define AGL_VARYING out\n",
        "#define AGL_FRAGMENT_SHADER\n"
        "#define AGL_VARYING in\n",
        "#define AGL_GEOMETRY_SHADER\n",
        "#define AGL_COMPUTE_SHADER\n"
        "#extension GL_ARB_compute_shader : enable",
    },
    {
        "#define AGL_VERTEX_SHADER\n"
        "#define AGL_VARYING out\n",
        "#define AGL_FRAGMENT_SHADER\n"
        "#define AGL_VARYING in\n",
        "#define AGL_GEOMETRY_SHADER\n",
        "#define AGL_COMPUTE_SHADER\n",
    },
    {
        "#define AGL_VERTEX_SHADER\n"
        "#define AGL_VARYING out\n"
        "precision highp float;\n"
        "precision highp int;\n",
        "#define AGL_FRAGMENT_SHADER\n"
        "#define AGL_VARYING in\n"
        "precision highp float;\n"
        "precision highp int;\n",
        "#define AGL_GEOMETRY_SHADER\n",
        "#define AGL_COMPUTE_SHADER\n",
    },
    {
        "#define AGL_VERTEX_SHADER\n"
        "#define AGL_VARYING varying\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "vec4 textureLod( sampler2D   sampler, vec2 coord, float lod ) { return texture2D( sampler, coord ); }\n"
        "vec4 textureLod( samplerCube sampler, vec3 coord, float lod ) { return textureCube( sampler, coord.xyz ); }\n"
        "vec4 textureLod( samplerCube sampler, vec4 coord, float lod ) { return textureCube( sampler, coord.xyz ); }\n",
        "#define AGL_FRAGMENT_SHADER\n"
        "#define AGL_VARYING varying\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "vec4 textureLod( sampler2D   sampler, vec2 coord, float lod ) { return texture2D( sampler, coord, lod ); }\n"
        "vec4 textureLod( samplerCube sampler, vec3 coord, float lod ) { return textureCube( sampler, coord.xyz, lod ); }\n"
        "vec4 textureLod( samplerCube sampler, vec4 coord, float lod ) { return textureCube( sampler, coord.xyz, lod ); }\n"
        "#define AGL_GEOMETRY_SHADER\n",
        "#define AGL_COMPUTE_SHADER\n",
    },
    {
        "#define AGL_VERTEX_SHADER\n"
        "#define AGL_VARYING out\n",
        "#define AGL_FRAGMENT_SHADER\n"
        "#define AGL_VARYING in\n",
        "#define AGL_GEOMETRY_SHADER\n",
        "#define AGL_COMPUTE_SHADER\n",
    },
};

const char* const cTargetText[ShaderCompileInfo::cTarget_Num] = {
    "#define AGL_SHADER\n"
    "#define AGL_TARGET_GL\n"
    "#define AGL_REVERSE_RENDER_TEXTURE\n"
    "#define AGL_ATTRIBUTE( loc )         layout( location = loc ) in\n"
    "#define AGL_UNIFORM_BLOCK( name )    layout( std140 ) uniform name\n",
    "#define AGL_SHADER\n"
    "#define AGL_TARGET_GX2\n"
    "#define AGL_ATTRIBUTE( loc )      layout( location = loc ) in\n"
    "#define AGL_UNIFORM_BLOCK( name ) layout( std140 ) uniform name\n",
    "#define AGL_SHADER\n"
    "#define AGL_TARGET_GL\n"
    "#define AGL_TARGET_GLES\n"
    "#define AGL_TARGET_GLES3\n"
    "#define AGL_REVERSE_RENDER_TEXTURE\n"
    "#define AGL_ATTRIBUTE( loc )      layout( location = loc ) in\n"
    "#define AGL_UNIFORM_BLOCK( name ) layout( std140 ) uniform name\n"
    "#define noperspective smooth\n"
    "#define samplerCubeArray samplerCube\n"
    "vec4 texture2D( sampler2D sampler, vec2 coord ) { return texture( sampler, coord ); }\n"
    "vec4 textureGather( sampler2D sampler, vec2 coord ) { return texture( sampler, coord ); }\n"
    "vec4 textureGather( sampler3D sampler, vec2 coord ) { return texture( sampler, coord ); }\n",
    "#define AGL_SHADER\n"
    "#define AGL_TARGET_GL\n"
    "#define AGL_TARGET_GLES\n"
    "#define AGL_TARGET_GLES2\n"
    "#define AGL_REVERSE_RENDER_TEXTURE\n"
    "#define AGL_ATTRIBUTE( loc ) attribute\n"
    "#define AGL_UNIFORM_BLOCK( name ) \n"
    "#define sampler2DArray   sampler2D\n"
    "#define sampler3D        sampler2D\n"
    "#define samplerCubeArray samplerCube\n"
    "#define sampler2DMS      sampler2D\n"
    "#define gl_VertexID      0\n"
    "#define gl_InstanceID    0\n"
    "#define gl_FragDepth     float temp_gl_frag_depth\n"
    "vec4 texture( sampler2D   sampler, vec2 coord )               { return texture2D( sampler, coord );    }\n"
    "vec4 texture( sampler2D   sampler, vec3 coord )               { return texture2D( sampler, coord.xy ); }\n"
    "vec4 texture( sampler2D   sampler, vec4 coord )               { return texture2D( sampler, coord.xy ); }\n"
    "vec4 texture( samplerCube sampler, vec3 coord )               { return textureCube( sampler, coord );  }\n"
    "vec4 texture( samplerCube sampler, vec4 coord )               { return textureCube( sampler, coord.xyz ); }\n"
    "vec2 textureSize( sampler3D sampler )                         { return vec2( 1.0 ); }\n"
    "vec4 textureGather( sampler2D sampler, vec2 coord )           { return texture2D( sampler, coord ); }\n"
    "vec4 textureGather( sampler2D sampler, vec3 coord )           { return texture2D( sampler, coord.xy ); }\n"
    "vec4 texelFetch( sampler3D sampler, ivec2 c, int i )          { return texture2D( sampler, vec2( c.x, c.y ) ); }\n"
    "vec4 textureLod( sampler3D sampler, vec3 coord, float lod )   { return texture2D( sampler, coord.xy, lod ); }\n",
    "#define AGL_SHADER\n"
    "#define AGL_TARGET_NVN\n"
    "#define AGL_ATTRIBUTE( loc )      layout( location = loc ) in\n"
    "#define AGL_UNIFORM_BLOCK( name ) layout( std140 ) uniform name\n",
};

}  // namespace

/**
 * Constructs empty compile information.
 */
ShaderCompileInfo::ShaderCompileInfo() : mName("unititled"), mSource(nullptr), mCompiledSource(nullptr) {}

/**
 * Destroys the compile information.
 */
ShaderCompileInfo::~ShaderCompileInfo()
{
    destroy();
}

/**
 * Frees the macro and variation arrays.
 */
void ShaderCompileInfo::destroy()
{
    if (mMacroName.isBufferReady()) {
        mMacroName.freeBuffer();
        mMacroValue.freeBuffer();
    }

    if (mVariationName.isBufferReady()) {
        mVariationName.freeBuffer();
        mVariationValue.freeBuffer();
    }
}

/**
 * Allocates the macro and variation arrays.
 * @param macroNum maximum number of macros
 * @param variationNum maximum number of variation macros
 * @param pHeap heap to allocate from
 */
void ShaderCompileInfo::create(s32 macroNum, s32 variationNum, sead::Heap* pHeap)
{
    if (macroNum > 0) {
        mMacroName.allocBuffer(macroNum, pHeap);
        mMacroValue.allocBuffer(macroNum, pHeap);
    }

    if (variationNum > 0) {
        mVariationName.allocBuffer(variationNum, pHeap);
        mVariationValue.allocBuffer(variationNum, pHeap);
    }
}

/**
 * Removes all variation macros.
 */
void ShaderCompileInfo::clearVariation()
{
    mVariationName.clear();
    mVariationValue.clear();
}

/**
 * Adds a variation macro.
 * @param pName macro name
 * @param pValue macro value
 */
void ShaderCompileInfo::pushBackVariation(const char* pName, const char* pValue)
{
    mVariationName.pushBack(pName);
    mVariationValue.pushBack(pValue);
}

// NON_MATCHING: "" literal gets 4-byte alignment; register swap in the variation macro copy loop
void ShaderCompileInfo::calcCompileSource(ShaderType type, sead::BufferedSafeString* pDst,
                                          Target target, bool usePreprocessor) const
{
    pDst->copy("", 0);

    if (!mSource) {
        return;
    }

    sead::SafeString text = *mSource;
    if (detail::ShaderTextUtil::isUTF8(mSource->cstr())) {
        text = text.cstr() + 3;
    }

    s32 targetIndex = target;
    if (target == cTarget_NVNBinary) {
        targetIndex = cTarget_NVN;
    }

    s32 versionIndex = text.findIndex("#version");
    if (versionIndex != -1) {
        s32 lineFeedLength;
        s32 lineLength =
            detail::ShaderTextUtil::findLineFeedCode(text.cstr() + versionIndex, &lineFeedLength);
        if (lineLength != -1) {
            text = text.cstr() + versionIndex + lineLength + lineFeedLength;
        }
    }

    pDst->append("// ----- These macros are auto defined by AGL.-----\n");
    pDst->append(cVersionText[targetIndex]);
    pDst->append(cShaderTypeText[targetIndex][type]);
    pDst->append(cTargetText[targetIndex]);
    pDst->append("// ------------------------------------------------\n");
    pDst->append(text);

    if (usePreprocessor) {
        sead::BufferedSafeString work(detail::PrivateResource::instance()->getWorkBuffer(),
                                      detail::PrivateResource::instance()->getWorkBufferSize());

        const char* names[256];
        const char* values[256];
        s32 num = 0;
        for (s32 i = 0; i < mMacroName.size(); i++) {
            names[num] = mMacroName.unsafeAt(i);
            values[num] = mMacroValue.unsafeAt(i);
            num++;
        }

        for (s32 i = 0; i < mVariationName.size(); i++, num++) {
            names[num] = mVariationName.unsafeAt(i);
            values[num] = mVariationValue.unsafeAt(i);
        }

        sead::Heap* heap = detail::PrivateResource::instance()->getShaderTextHeap();
        shtxt::Preprocessor preprocessor(heap, heap);
        preprocessor.initialize(pDst->cstr());
        preprocessor.setReplacedMacro(names, values, num);
        preprocessor.preprocess(0x367, 0, 0);
        preprocessor.construct(&work);
        preprocessor.finalize();
        pDst->copy(work);
    } else {
        if (mMacroName.size() > 0) {
            detail::PrivateResource* resource = detail::PrivateResource::instance();
            detail::ShaderTextUtil::replaceMacro(
                pDst, MacroArray::getData(mMacroName), MacroArray::getData(mMacroValue), mMacroName.size(),
                resource->getWorkBuffer(), resource->getWorkBufferSize());
        }

        if (mVariationName.size() > 0) {
            detail::PrivateResource* resource = detail::PrivateResource::instance();
            detail::ShaderTextUtil::replaceMacro(
                pDst, MacroArray::getData(mVariationName), MacroArray::getData(mVariationValue), mVariationName.size(),
                resource->getWorkBuffer(), resource->getWorkBufferSize());
        }
    }

    if (mCompiledSource) {
        mCompiledSource->copy(*pDst);
    }
}

/**
 * Gets the name of the uniform block that holds register uniforms.
 * @return "RegisterUBO"
 */
const sead::SafeString& ShaderCompileInfo::getRegitserUniformBlockName()
{
    return cRegisterUniformBlockName;
}

}  // namespace agl
