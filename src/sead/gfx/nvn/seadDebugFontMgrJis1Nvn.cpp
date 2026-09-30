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
SEAD_SINGLETON_DISPOSER_IMPL(DebugFontMgrJis1Nvn)

u32 DebugFontMgrJis1Nvn::sTextureID;

/**
 * Constructs the debug font manager.
 */
DebugFontMgrJis1Nvn::DebugFontMgrJis1Nvn() = default;

/**
 * Destroys the debug font manager.
 */
DebugFontMgrJis1Nvn::~DebugFontMgrJis1Nvn() = default;

/**
 * Sets the texture descriptor id used to register the font texture.
 * @param id texture descriptor id
 */
void DebugFontMgrJis1Nvn::setTextureID(u32 id)
{
    sTextureID = id;
}

/**
 * Loads the character table, font and shader files and initializes the manager from them.
 * @param pHeap heap to load into
 * @param pShaderPath path of the shader binary
 * @param pFontPath path of the font texture binary
 * @param pTablePath path of the character code table
 * @param uniformBufferSize size of the uniform buffer
 */
void DebugFontMgrJis1Nvn::initialize(Heap* pHeap, const char* pShaderPath, const char* pFontPath,
                                     const char* pTablePath, u32 uniformBufferSize)
{
    FileDevice::LoadArg load_arg = {};
    load_arg.heap = pHeap;

    load_arg.path = pTablePath;
    void* table_binary = FileDeviceMgr::instance()->tryLoad(load_arg);

    load_arg.path = pFontPath;
    load_arg.alignment = 0x1000;
    load_arg.buffer_size_alignment = 0x1000;
    void* font_binary = FileDeviceMgr::instance()->tryLoad(load_arg);
    u32 font_size = load_arg.read_size;

    load_arg.path = pShaderPath;
    void* shader_binary = FileDeviceMgr::instance()->tryLoad(load_arg);
    u32 shader_size = load_arg.read_size;

    mFontSizeType = font_size > 0x300000;
    initializeFromBinary(pHeap, shader_binary, shader_size, font_binary, font_size, table_binary,
                         uniformBufferSize);
}

/**
 * Creates the shader program, uniform buffer and font texture from loaded binaries.
 * @param pHeap heap to allocate from
 * @param pShaderBinary shader binary
 * @param shaderSize size of the shader binary
 * @param pFontBinary font texture binary
 * @param fontSize size of the font texture binary
 * @param pTableBinary character code table
 * @param uniformBufferSize size of the uniform buffer
 */
void DebugFontMgrJis1Nvn::initializeFromBinary(Heap* pHeap, void* pShaderBinary, u64 shaderSize,
                                               void* pFontBinary, u64 fontSize,
                                               const void* pTableBinary, u32 uniformBufferSize)
{
    struct ShaderBinaryHeader
    {
        u32 vertexControlOffset;
        u32 fragmentControlOffset;
        u32 vertexDataOffset;
        u32 fragmentDataOffset;
    };

    mCharCodeTable = pTableBinary;
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
    nvnTextureBuilderSetFormat(&textureBuilder, NVN_FORMAT_RGTC1_UNORM);
    s32 levels = mFontSizeType != 0 ? 2 : 1;
    nvnTextureBuilderSetSize2D(&textureBuilder, mFontSizeType == 0 ? 0x500 : 0x900,
                               mFontSizeType == 0 ? 0x512 : 0x8c2);
    nvnTextureBuilderSetLevels(&textureBuilder, levels);
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
void DebugFontMgrJis1Nvn::swapUniformBlockBuffer()
{
    mUniformBlockBuffer.swap(mUniformBlockBuffer.get_0(), mUniformBufferSize);
    mIsUniformBufferFull = false;
}

/**
 * Gets the glyph height.
 * @return glyph height in pixels
 */
float DebugFontMgrJis1Nvn::getHeight() const
{
    return 16.0f;
}

/**
 * Gets the glyph width.
 * @return glyph width in pixels
 */
float DebugFontMgrJis1Nvn::getWidth() const
{
    return 16.0f;
}

/**
 * Gets the width of a character.
 * @param c character
 * @return 8 for ASCII characters, 16 otherwise
 */
float DebugFontMgrJis1Nvn::getCharWidth(char16_t c) const
{
    return c < 0x7F ? 8.0f : 16.0f;
}

/**
 * Gets the maximum number of characters drawn per print call.
 * @return maximum character count
 */
u32 DebugFontMgrJis1Nvn::getMaxDrawNum() const
{
    return 0x80;
}

/**
 * Binds the font shader program.
 * @param pDrawContext draw context
 */
void DebugFontMgrJis1Nvn::begin(DrawContext* pDrawContext) const
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
void DebugFontMgrJis1Nvn::end(DrawContext* pDrawContext) const {}

/**
 * Draws a string of UTF-16 characters, looking up non-ASCII glyphs in the JIS table.
 * @param pDrawContext draw context
 * @param rProjection projection
 * @param rCamera camera
 * @param rMatrix model matrix
 * @param rColor text color
 * @param pText UTF-16 text
 * @param length number of characters
 */
void DebugFontMgrJis1Nvn::print(DrawContext* pDrawContext, const Projection& rProjection,
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
    u32 x = 2;
    for (s32 i = 0; i < num; i++)
    {
        u32 c = text[i];
        u32 index;
        if (c >= 0x7f)
        {
            index = searchCharIndexFormCharCode_(c);
            if (index == 0)
            {
                index = 0x1f;
            }
        }
        else
        {
            if (c < 0x20)
            {
                continue;
            }

            index = c - 0x20;
        }

        chars[count++] = index | x << 16;
        x += c < 0x7f ? 8 : 16;
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

/**
 * Binary searches the JIS character table for a character code.
 * @param code UTF-16 character code
 * @return the glyph index, or 0 if the code is not in the table
 */
u32 DebugFontMgrJis1Nvn::searchCharIndexFormCharCode_(u32 code) const
{
    const u16* table = static_cast<const u16*>(mCharCodeTable);
    u32 hi = 0xe88;
    u32 lo = 0x5e;
    u32 mid = (lo + hi) / 2;
    while (table[mid] != code)
    {
        if (table[mid] < code)
        {
            if (lo == mid)
            {
                return 0;
            }

            lo = mid;
        }
        else
        {
            if (hi == mid)
            {
                return 0;
            }

            hi = mid;
        }

        mid = (lo + hi) / 2;
    }

    return mid;
}

}  // namespace sead
