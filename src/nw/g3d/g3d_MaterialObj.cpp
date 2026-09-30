#include <nn/g3d/g3d_Resources.h>
#include <nn/gfx/gfx_MemoryPool.h>
#include <cstring>
#include <new>

namespace nn::g3d {
void MaterialObj::InitializeArgument::CalculateMemorySize() {
    int count = resource->ToData().shaderParamCount;
    int copies = bufferCount <= 1 ? 1 : bufferCount + 1;
    for (int i = 0; i < 5; ++i) blocks[i].Initialize(0);
    blocks[0].size = ((count + 31) >> 5) * 4 * copies;
    blocks[1].size = (resource->ToData().sourceParamSize + 7) & ~7;
    blocks[2].size = resource->ToData().textureCount * sizeof(void*);
    blocks[3].size = bufferCount * sizeof(nn::gfx::Buffer);
    blocks[4].size = resource->ToData().textureCount * sizeof(u64);
    memorySize = 0;
    memoryAlignment = 8;
    for (int i = 0; i < 5; ++i) {
        if (blocks[i].size) {
            size_t offset = (memorySize + 7) & ~size_t(7);
            memorySize = offset + blocks[i].size;
            memoryAlignment = 8;
            blocks[i].offset = offset;
        }
    }
}
// argument describes the resource and buffering; memory supplies size bytes of working storage.
bool MaterialObj::Initialize(const InitializeArgument& argument, void* memory, size_t size) {
    if (!argument.memoryAlignment || argument.memorySize > size) return false;
    const ResMaterial* resource = argument.resource;
    int textureCount = resource->ToData().textureCount;
    int parameterCount = resource->ToData().shaderParamCount;
    m_pRes = resource;
    m_Flag = 0;
    m_BufferingCount = argument.bufferCount;
    m_pParamSource = argument.blocks[1].GetPointer(memory);
    memcpy(m_pParamSource, resource->ToData().pSourceParamData.Get(), resource->ToData().sourceParamSize);
    m_ppTextureArray = static_cast<const nn::gfx::TextureView**>(argument.blocks[2].GetPointer(memory));
    m_pTextureSlotArray = static_cast<u64*>(argument.blocks[4].GetPointer(memory));
    BufferImpl* buffers = static_cast<BufferImpl*>(argument.blocks[3].GetPointer(memory));
    m_pMemoryPool = nullptr;
    m_pMaterialBlockArray = buffers;
    m_pCallbackUserData = nullptr;
    m_pWorkMemory = memory;
    m_MemoryPoolOffset = 0;
    m_MaterialBlockSize = 0;
    m_DirtyFlags.Initialize(parameterCount, m_BufferingCount, argument.blocks[0].GetPointer(memory), argument.blocks[0].size);
    for (int i = 0; i < textureCount; ++i) {
        m_ppTextureArray[i] = resource->ToData().pTextureArray.Get()[i];
        m_pTextureSlotArray[i] = resource->ToData().pTextureSlotArray.Get()[i];
    }
    InitializeDependPointer();
    return true;
}
void MaterialObj::InitializeDependPointer() {
    int count = m_pRes->ToData().shaderParamCount;
    for (int i = 0; i < count; ++i) {
        const ResShaderParamData* parameters = m_pRes->ToData().pShaderParamArray.Get();
        const ResShaderParamData& parameter = parameters[i];
        if (i != parameter.dependencyIndex) {
            u8* source = static_cast<u8*>(m_pParamSource) + parameter.sourceOffset;
            void* dependency = static_cast<u8*>(m_pParamSource) + parameters[parameter.dependencyIndex].sourceOffset;
            size_t size = ResShaderParam::GetSrcSize(static_cast<ResShaderParam::Type>(parameter.type));
            *reinterpret_cast<void**>((reinterpret_cast<uintptr_t>(source + size) + 7) & ~uintptr_t(7)) = dependency;
        }
    }
}
// device supplies the uniform-buffer alignment required by the graphics backend.
size_t MaterialObj::GetBlockBufferAlignment(nn::gfx::Device* device) const {
    nn::gfx::BufferInfo info;
    memset(&info, 0, sizeof(info));
    info.SetDefault();
    info.SetSize(m_pRes->ToData().materialBlockSize);
    info.SetGpuAccessFlags(16);
    return BufferImpl::GetBufferAlignment(device, info);
}
// device supplies alignment for each buffered copy of the material block.
size_t MaterialObj::CalculateBlockBufferSize(nn::gfx::Device* device) const {
    u32 size = m_pRes->ToData().materialBlockSize;
    u32 alignment = GetBlockBufferAlignment(device);
    return ((size + alignment - 1) & -alignment) * m_BufferingCount;
}
void MaterialObj::ResetDirtyFlags() {
    int count = m_pRes->ToData().shaderParamCount;
    if (m_DirtyFlags.mWordCount > 0) memset(m_DirtyFlags.mPending, 0, static_cast<unsigned>(m_DirtyFlags.mWordCount) * sizeof(u32));
    m_DirtyFlags.mFlags &= ~1;
    for (int i = 0; i < count; ++i) {
        if (m_pRes->ToData().pShaderParamArray.Get()[i].offset >= 0) {
            m_DirtyFlags.mPending[static_cast<unsigned>(i) >> 5] |= 1u << (i & 31);
            m_DirtyFlags.mFlags |= 1;
        }
    }
}
// device creates buffers in pool at offset; size has already been checked by SetupBlockBuffer.
void MaterialObj::SetupBlockBufferImpl(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset, size_t size) {
    ResetDirtyFlags();
    if (m_DirtyFlags.mBufferCount >= 2) {
        for (int i = 0; i < m_DirtyFlags.mBufferCount; ++i) {
            if (m_DirtyFlags.mWordCount > 0) memset(m_DirtyFlags.mBufferFlags + i * m_DirtyFlags.mWordCount, 0, static_cast<unsigned>(m_DirtyFlags.mWordCount) * sizeof(u32));
        }
    }
    m_MaterialBlockSize = m_pRes->ToData().materialBlockSize;
    for (int i = 0; i < m_BufferingCount; ++i) {
        nn::gfx::BufferInfo info;
        memset(&info, 0, sizeof(info));
        info.SetDefault();
        info.SetSize(m_MaterialBlockSize);
        info.SetGpuAccessFlags(16);
        nn::gfx::Buffer* buffer = new (&m_pMaterialBlockArray[i]) nn::gfx::Buffer;
        buffer->Initialize(device, info, pool, offset, m_MaterialBlockSize);
        nn::gfx::util::SetBufferDebugLabel(buffer, "g3d_MaterialUniformBlock");
        size_t blockSize = m_MaterialBlockSize;
        size_t alignment = GetBlockBufferAlignment(device);
        offset += (blockSize + alignment - 1) & -alignment;
    }
    m_Flag |= Flag_BlockBufferValid;
}
// device/pool select storage; offset and size delimit the available memory-pool region.
bool MaterialObj::SetupBlockBuffer(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset, size_t size) {
    size_t required = CalculateBlockBufferSize(device);
    if (required > size) return false;
    if (required) {
        m_pMemoryPool = pool;
        m_MemoryPoolOffset = offset;
        SetupBlockBufferImpl(device, pool, offset, size);
    }
    return true;
}
// device owns the buffered GPU objects to finalize.
void MaterialObj::CleanupBlockBuffer(nn::gfx::Device* device) {
    for (int i = 0; i < m_BufferingCount; ++i) {
        BufferImpl* buffer = &m_pMaterialBlockArray[i];
        buffer->Finalize(device);
        buffer->~BufferImpl();
    }
    m_Flag &= ~Flag_BlockBufferValid;
    m_MaterialBlockSize = 0;
    m_pMemoryPool = nullptr;
    m_MemoryPoolOffset = 0;
}
// bufferIndex selects the buffered material copy receiving dirty parameter values.
void MaterialObj::CalculateMaterial(int bufferIndex) {
    if (!m_MaterialBlockSize) return;
    m_DirtyFlags.Caclulate();
    int mask = 1 << bufferIndex;
    if (!(mask & m_DirtyFlags.mDirtyBuffers) && !m_pRes->ToData().volatileParamCount) return;
    void* mapped = GetMaterialBlock(bufferIndex)->Map();
    m_DirtyFlags.mDirtyBuffers &= ~mask;
    ConvertDirtyParams<false>(mapped, m_DirtyFlags.mBufferFlags + m_DirtyFlags.mWordCount * bufferIndex);
    if (m_DirtyFlags.mWordCount > 0) memset(m_DirtyFlags.mBufferFlags + m_DirtyFlags.mWordCount * bufferIndex, 0, static_cast<unsigned>(m_DirtyFlags.mWordCount) * sizeof(u32));
    GetMaterialBlock(bufferIndex)->FlushMappedRange(0, m_MaterialBlockSize);
    GetMaterialBlock(bufferIndex)->Unmap();
}
// value is a nonzero mask; return the index of its highest set bit.
static int HighestBit(u32 value) {
    value |= value >> 1; value |= value >> 2; value |= value >> 4;
    value |= value >> 8; value |= value >> 16;
    value = ~value;
    value = (value & 0x55555555) + ((value >> 1) & 0x55555555);
    value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
    value = (value & 0x07070707) + ((value >> 4) & 0x07070707);
    value = (value & 0x000f000f) + ((value >> 8) & 0x000f000f);
    return 31 - ((value & 31) + (value >> 16));
}
// destination is mapped GPU storage; dirtyFlags selects parameters in addition to volatile ones.
template <bool swap> void MaterialObj::ConvertDirtyParams(void* destination, u32* dirtyFlags) {
    int wordCount = (m_pRes->ToData().shaderParamCount + 31) >> 5;
    const u32* volatileFlags = m_pRes->ToData().pVolatileParamFlags.Get();
    for (int i = 0; i < wordCount; ++i) {
        u32 flags = dirtyFlags[i] | volatileFlags[i];
        while (flags) {
            int bit = HighestBit(flags);
            const ResShaderParam* parameter = reinterpret_cast<const ResShaderParam*>(&m_pRes->ToData().pShaderParamArray.Get()[i * 32 + bit]);
            void* target = static_cast<u8*>(destination) + parameter->ToData().offset;
            const void* source = static_cast<u8*>(m_pParamSource) + parameter->ToData().sourceOffset;
            if (parameter->ToData().callback) {
                u8 temporary[64];
                size_t size = parameter->ToData().callback(temporary, source, parameter, m_pCallbackUserData);
                int words = (size + 3) / 4;
                memcpy(target, temporary, static_cast<ptrdiff_t>(words) * 4);
            } else parameter->Convert<swap>(target, source);
            flags ^= 1u << bit;
        }
    }
}
}
