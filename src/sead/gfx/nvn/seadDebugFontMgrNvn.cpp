#include "gfx/nvn/seadDebugFontMgrNvn.h"
#include "filedevice/seadFileDevice.h"
#include "filedevice/seadFileDeviceMgr.h"
#include "gfx/nin/seadGraphicsNvn.h"
#include "gfx/seadGraphics.h"
#include "math/seadMatrixCalcCommon.h"
#include "nn/os.h"
#include "nvn/nvn.h"
#include "nvn/nvn_FuncPtrInline.h"

namespace sead
{
SEAD_SINGLETON_DISPOSER_IMPL(DebugFontMgrNvn)

u32 DebugFontMgrNvn::sTextureID;

/**
 * Constructs the debug font manager.
 */
DebugFontMgrNvn::DebugFontMgrNvn() = default;

/**
 * Destroys the debug font manager.
 */
DebugFontMgrNvn::~DebugFontMgrNvn() = default;

/**
 * Sets the texture descriptor id used to register the font texture.
 * @param id texture descriptor id
 */
void DebugFontMgrNvn::setTextureID(u32 id)
{
    sTextureID = id;
}

/**
 * Loads the shader and font files and initializes the manager from them.
 * @param pHeap heap to load into
 * @param pShaderPath path of the shader binary
 * @param pFontPath path of the font texture binary
 * @param uniformBufferSize size of the uniform buffer
 */
void DebugFontMgrNvn::initialize(Heap* pHeap, const char* pShaderPath, const char* pFontPath,
                                 u32 uniformBufferSize)
{
    FileDevice::LoadArg load_arg = {};

    load_arg.path = pFontPath;
    load_arg.heap = pHeap;
    load_arg.alignment = 0x1000;
    load_arg.buffer_size_alignment = 0x1000;
    void* font_binary = FileDeviceMgr::instance()->tryLoad(load_arg);
    u32 font_size = load_arg.read_size;

    load_arg.path = pShaderPath;
    void* shader_binary = FileDeviceMgr::instance()->tryLoad(load_arg);
    u32 shader_size = load_arg.read_size;

    initializeFromBinary(pHeap, shader_binary, shader_size, font_binary, font_size,
                         uniformBufferSize);
}

/**
 * Creates the shader program, uniform buffer and font texture from loaded binaries.
 * @param pHeap heap to allocate from
 * @param pShaderBinary shader binary
 * @param shaderSize size of the shader binary
 * @param pFontBinary font texture binary
 * @param fontSize size of the font texture binary
 * @param uniformBufferSize size of the uniform buffer
 */
void DebugFontMgrNvn::initializeFromBinary(Heap* pHeap, void* pShaderBinary, u64 shaderSize,
                                           void* pFontBinary, u64 fontSize, u32 uniformBufferSize)
{
    struct ShaderBinaryHeader
    {
        u32 vertexControlOffset;
        u32 fragmentControlOffset;
        u32 vertexDataOffset;
        u32 fragmentDataOffset;
    };

    mUniformBufferSize = uniformBufferSize;
    NVNdevice* device = GraphicsNvn::instance()->getNvnDevice();
    nvnProgramInitialize(&mNvnProgram, device);

    {
        NVNmemoryPoolBuilder poolBuilder;
        nvnMemoryPoolBuilderSetDefaults(&poolBuilder);
        nvnMemoryPoolBuilderSetDevice(&poolBuilder, device);
        nvnMemoryPoolBuilderSetFlags(&poolBuilder, NVN_MEMORY_POOL_FLAGS_CPU_UNCACHED |
                                                       NVN_MEMORY_POOL_FLAGS_GPU_CACHED |
                                                       NVN_MEMORY_POOL_FLAGS_SHADER_CODE);
        shaderSize = (shaderSize + 0xfff) & ~0xfffull;
        nvnMemoryPoolBuilderSetStorage(&poolBuilder, pShaderBinary, shaderSize);
        nvnMemoryPoolInitialize(&mShaderMemoryPool, &poolBuilder);

        NVNbufferBuilder bufferBuilder;
        nvnBufferBuilderSetDevice(&bufferBuilder, device);
        nvnBufferBuilderSetDefaults(&bufferBuilder);
        nvnBufferBuilderSetStorage(&bufferBuilder, &mShaderMemoryPool, 0, shaderSize);
        nvnBufferInitialize(&mShaderBuffer, &bufferBuilder);
    }

    NVNbufferAddress address = nvnBufferGetAddress(&mShaderBuffer);
    const auto* header = static_cast<const ShaderBinaryHeader*>(pShaderBinary);
    NVNshaderData shaderData[2];
    shaderData[0].data = address + header->vertexDataOffset;
    shaderData[0].control =
        reinterpret_cast<void*>(header->vertexControlOffset + uintptr_t(pShaderBinary));
    shaderData[1].data = address + header->fragmentDataOffset;
    shaderData[1].control =
        reinterpret_cast<void*>(header->fragmentControlOffset + uintptr_t(pShaderBinary));
    nvnProgramSetShaders(&mNvnProgram, 2, shaderData);

    static size_t sUniformPoolSize = size_t(mUniformBufferSize + 0x200) + 0xfff - (mUniformBufferSize + 0x200 + 0xfff) % 0x1000;

    {
        NVNmemoryPoolBuilder poolBuilder;
        nvnMemoryPoolBuilderSetDefaults(&poolBuilder);
        nvnMemoryPoolBuilderSetDevice(&poolBuilder, device);
        nvnMemoryPoolBuilderSetFlags(&poolBuilder, NVN_MEMORY_POOL_FLAGS_CPU_UNCACHED |
                                                       NVN_MEMORY_POOL_FLAGS_GPU_CACHED);
        void* storage = new (pHeap, 0x1000) u8[sUniformPoolSize];
        nvnMemoryPoolBuilderSetStorage(&poolBuilder, storage, sUniformPoolSize);
        nvnMemoryPoolInitialize(&mUniformMemoryPool, &poolBuilder);
    }

    {
        NVNbufferBuilder bufferBuilder;
        nvnBufferBuilderSetDefaults(&bufferBuilder);
        nvnBufferBuilderSetDevice(&bufferBuilder, device);
        nvnBufferBuilderSetStorage(&bufferBuilder, &mUniformMemoryPool, 0, mUniformBufferSize);
        nvnBufferInitialize(&mUniformBuffer, &bufferBuilder);
        mUniformBufferMap = nvnBufferMap(&mUniformBuffer);
    }

    NVNtextureBuilder textureBuilder;
    nvnTextureBuilderSetDefaults(&textureBuilder);
    nvnTextureBuilderSetDevice(&textureBuilder, device);
    nvnTextureBuilderSetTarget(&textureBuilder, NVN_TEXTURE_TARGET_2D);
    nvnTextureBuilderSetFormat(&textureBuilder, NVN_FORMAT_R8);
    nvnTextureBuilderSetSize2D(&textureBuilder, 0x80, 0x80);
    nvnTextureBuilderSetPackagedTextureData(&textureBuilder,
                                            static_cast<u8*>(pFontBinary) + 0x200);

    {
        NVNmemoryPoolBuilder poolBuilder;
        nvnMemoryPoolBuilderSetDefaults(&poolBuilder);
        nvnMemoryPoolBuilderSetDevice(&poolBuilder, device);
        nvnMemoryPoolBuilderSetFlags(&poolBuilder, NVN_MEMORY_POOL_FLAGS_CPU_NO_ACCESS |
                                                       NVN_MEMORY_POOL_FLAGS_GPU_CACHED);
        nvnMemoryPoolBuilderSetStorage(&poolBuilder, pFontBinary, (fontSize + 0xfff) & ~0xfffull);
        nvnMemoryPoolInitialize(&mFontMemoryPool, &poolBuilder);
    }

    nvnTextureBuilderSetStorage(&textureBuilder, &mFontMemoryPool, 0x200);
    nvnTextureInitialize(&mNvnTexture, &textureBuilder);

    CriticalSection* cs = GraphicsNvn::instance()->getCriticalSection2();
    cs->lock();
    nvnTexturePoolRegisterTexture(GraphicsNvn::instance()->getTexturePool(), sTextureID,
                                  &mNvnTexture, nullptr);
    cs->unlock();
    mNvnTextureHandle = nvnDeviceGetTextureHandle(
        device, sTextureID, GraphicsNvn::instance()->getTextureSamplerID());
}

/**
 * Starts a new frame of the uniform ring buffer.
 */
void DebugFontMgrNvn::swapUniformBlockBuffer()
{
    mUniformBlockBuffer.swap(mUniformBlockBuffer.get_0(), mUniformBufferSize);
    mIsUniformBufferFull = false;
}

/**
 * Gets the glyph height.
 * @return glyph height in pixels
 */
float DebugFontMgrNvn::getHeight() const
{
    return 16.0f;
}

/**
 * Gets the glyph width.
 * @return glyph width in pixels
 */
float DebugFontMgrNvn::getWidth() const
{
    return 8.0f;
}

/**
 * Gets the width of a character.
 * @param c character
 * @return character width in pixels
 */
float DebugFontMgrNvn::getCharWidth(char16_t c) const
{
    return 8.0f;
}

/**
 * Gets the maximum number of characters drawn per print call.
 * @return maximum character count
 */
u32 DebugFontMgrNvn::getMaxDrawNum() const
{
    return 0x80;
}

/**
 * Binds the font shader program.
 * @param pDrawContext draw context
 */
void DebugFontMgrNvn::begin(DrawContext* pDrawContext) const
{
    if (mIsUniformBufferFull)
    {
        return;
    }

    nvnCommandBufferBindProgram(pDrawContext->getNvnCommandBuffer(), &mNvnProgram,
                                NVN_SHADER_STAGE_ALL_GRAPHICS_BITS);
}

/**
 * Finishes drawing; does nothing.
 * @param pDrawContext draw context
 */
void DebugFontMgrNvn::end(DrawContext* pDrawContext) const {}

/**
 * Draws a string of ASCII characters.
 * @param pDrawContext draw context
 * @param rProjection projection
 * @param rCamera camera
 * @param rMatrix model matrix
 * @param rColor text color
 * @param pText UTF-16 text
 * @param length number of characters
 */
void DebugFontMgrNvn::print(DrawContext* pDrawContext, const Projection& rProjection,
                            const Camera& rCamera, const Matrix34f& rMatrix,
                            const Color4f& rColor, const void* pText, int length) const
{
    if (mIsUniformBufferFull)
    {
        return;
    }

    s32 num = length < 0x80 ? length : 0x80;
    NVNcommandBuffer* commandBuffer = pDrawContext->getNvnCommandBuffer();
    u32 vertexUniformSize = (num * 4 + 0x40 + 0xff) & ~0xff;
    u32 allocSize = vertexUniformSize + 0x100;
    u32 start = mUniformBlockBuffer.fetchAdd_0(allocSize);
    if (start + allocSize - mUniformBlockBuffer.get_4() > mUniformBufferSize)
    {
        mIsUniformBufferFull = true;
        return;
    }

    if (length < 1)
    {
        return;
    }

    u32 offset = start % mUniformBufferSize;
    u8* uniform = static_cast<u8*>(mUniformBufferMap) + offset;
    u32* chars = reinterpret_cast<u32*>(uniform + 0x40);
    const char16_t* text = static_cast<const char16_t*>(pText);
    s32 count = 0;
    for (s32 i = 0; i < num; i++)
    {
        u32 c = text[i];
        if (c < 0x20)
        {
            continue;
        }

        if (c > 0x7e)
        {
            c = '?';
        }

        chars[count++] = c - 0x20;
    }

    if (count == 0)
    {
        return;
    }

    Matrix44f projView;
    Matrix44CalcCommon<f32>::multiply(projView, rProjection.getDeviceProjectionMatrix(),
                                      rCamera.getMatrix());
    Matrix44CalcCommon<f32>::multiply(*reinterpret_cast<Matrix44f*>(uniform), projView, rMatrix);
    nvnCommandBufferBindUniformBuffer(commandBuffer, NVN_SHADER_STAGE_VERTEX, 0,
                                      nvnBufferGetAddress(&mUniformBuffer) + offset,
                                      vertexUniformSize);

    s32 fragmentOffset = offset + vertexUniformSize;
    u8* fragmentUniform = static_cast<u8*>(mUniformBufferMap) + fragmentOffset;
    *reinterpret_cast<Color4f*>(fragmentUniform) = rColor;
    *reinterpret_cast<NVNtextureHandle*>(fragmentUniform + 0x10) = mNvnTextureHandle;
    nvnCommandBufferBindUniformBuffer(commandBuffer, NVN_SHADER_STAGE_FRAGMENT, 0,
                                      nvnBufferGetAddress(&mUniformBuffer) + fragmentOffset,
                                      0x18);
    nvnCommandBufferDrawArrays(commandBuffer, NVN_DRAW_PRIMITIVE_QUADS, 0, count * 4);
}

}  // namespace sead
