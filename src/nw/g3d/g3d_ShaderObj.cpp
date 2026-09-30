#include <nn/g3d/g3d_ResShader.h>
#include <nn/gfx/gfx_MemoryPool.h>
#include <cstring>
#include <new>
#include <algorithm>

namespace nn::g3d {
void ShadingModelObj::InitializeArgument::CalculateMemorySize() {
    ptrdiff_t copies = bufferCount;
    int count = resource->ToData().staticOptionCount;
    int flagBytes = ((count + 31) / 32) * sizeof(u32);
    int flagCopies = copies > 1 ? copies + 1 : 1;
    blocks[0].Initialize(0);
    blocks[1].Initialize(0);
    blocks[2].Initialize(0);
    blocks[3].Initialize(0);
    blocks[0].size = resource->ToData().staticKeyLength * sizeof(u32);
    blocks[1].size = resource->ToData().staticKeyLength * sizeof(u32);
    blocks[2].size = flagCopies * flagBytes;
    blocks[3].size = bufferCount * sizeof(nn::gfx::Buffer);
    memorySize = 0;
    memoryAlignment = 8;
    for (int i = 0; i < 4; ++i) {
        if (blocks[i].size) {
            size_t offset = (memorySize + 7) & ~size_t(7);
            memoryAlignment = 8;
            memorySize = offset + blocks[i].size;
            blocks[i].offset = offset;
        }
    }
}
// argument supplies resource and buffering; memory is a workspace of size bytes.
bool ShadingModelObj::Initialize(const InitializeArgument& argument, void* memory, size_t size) {
    if (!argument.memoryAlignment || argument.memorySize > size) return false;
    resource = argument.resource;
    flags = 0;
    bufferCount = argument.bufferCount;
    staticKey = static_cast<u32*>(argument.blocks[0].GetPointer(memory));
    optionKey = static_cast<u32*>(argument.blocks[1].GetPointer(memory));
    BufferImpl* array = static_cast<BufferImpl*>(argument.blocks[3].GetPointer(memory));
    user = nullptr;
    work = memory;
    buffers = array;
    blockSize = 0;
    pool = nullptr;
    poolOffset = 0;
    dirtyFlags.Initialize(resource->ToData().staticOptionCount, bufferCount, argument.blocks[2].GetPointer(memory), argument.blocks[2].size);
    resource->WriteDefaultStaticKey(staticKey);
    std::copy(staticKey, staticKey + resource->ToData().staticKeyLength, optionKey);
    const u32* table = resource->ToData().pKeyTable.Get();
    int length = resource->ToData().staticKeyLength;
    range.begin = table + length;
    range.end = range.begin + (resource->ToData().staticKeyLength + resource->ToData().dynamicKeyLength) * resource->ToData().programCount;
    return true;
}
// device supplies the alignment of a uniform buffer containing option values.
size_t ShadingModelObj::GetBlockBufferAlignment(nn::gfx::Device* device) const {
    size_t size = resource->ToData().optionBlockIndex >= 0 ? resource->GetUniformBlock(resource->ToData().optionBlockIndex)->size : 0;
    nn::gfx::BufferInfo info;
    memset(&info, 0, sizeof(info));
    info.SetDefault();
    info.SetSize(size);
    info.SetGpuAccessFlags(16);
    return BufferImpl::GetBufferAlignment(device, info);
}
// device determines alignment for all buffered copies of the option block.
size_t ShadingModelObj::CalculateBlockBufferSize(nn::gfx::Device* device) {
    if (resource->ToData().optionBlockIndex < 0) return 0;
    size_t size = resource->GetUniformBlock(resource->ToData().optionBlockIndex)->size;
    nn::gfx::BufferInfo info;
    memset(&info, 0, sizeof(info));
    info.SetDefault();
    info.SetSize(size);
    info.SetGpuAccessFlags(16);
    size_t alignment = BufferImpl::GetBufferAlignment(device, info);
    return ((size + alignment - 1) & -alignment) * bufferCount;
}
// device creates buffers in pool at offset; size has already been validated by the caller.
void ShadingModelObj::SetupBlockBufferImpl(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset, size_t size) {
    size_t bytes = CalculateBlockBufferSize(device) / bufferCount;
    if (dirtyFlags.mWordCount > 0) memset(dirtyFlags.mPending, 0, static_cast<unsigned>(dirtyFlags.mWordCount) * sizeof(u32));
    dirtyFlags.mFlags &= ~1;
    if (dirtyFlags.mBufferCount > 1) {
        for (int i = 0; i < dirtyFlags.mBufferCount; ++i) {
            if (dirtyFlags.mWordCount > 0) memset(dirtyFlags.mBufferFlags + i * dirtyFlags.mWordCount, 0, static_cast<unsigned>(dirtyFlags.mWordCount) * sizeof(u32));
        }
    }
    nn::gfx::BufferInfo info;
    memset(&info, 0, sizeof(info));
    info.SetDefault();
    info.SetSize(bytes);
    info.SetGpuAccessFlags(16);
    for (size_t i = 0; i < bufferCount; ++i) {
        nn::gfx::Buffer* buffer = new (&buffers[i]) nn::gfx::Buffer;
        buffer->Initialize(device, info, pool, offset, bytes);
        nn::gfx::util::SetBufferDebugLabel(buffer, "g3d_OptionUniformBlock");
        size_t alignment = GetBlockBufferAlignment(device);
        offset += (bytes + alignment - 1) & -alignment;
    }
    blockSize = bytes;
    const ResUniformBlock* block = resource->ToData().optionBlockIndex >= 0 ? resource->GetUniformBlock(resource->ToData().optionBlockIndex) : nullptr;
    if (resource->ToData().optionBlockIndex >= 0 && block->size) {
        for (size_t i = 0; i < bufferCount; ++i) {
            BufferImpl* buffer = &buffers[i];
            void* mapped = buffer->Map();
            memset(mapped, 0, block->size);
            buffer->FlushMappedRange(0, blockSize);
            buffer->Unmap();
        }
    }
    flags |= 1;
}
// device/pool identify storage; offset and size delimit the available region.
bool ShadingModelObj::SetupBlockBuffer(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset, size_t size) {
    size_t required = CalculateBlockBufferSize(device);
    if (required > size) return false;
    if (required) {
        this->pool = pool;
        poolOffset = offset;
        SetupBlockBufferImpl(device, pool, offset, size);
    }
    return true;
}
// device owns the option buffers to finalize.
void ShadingModelObj::CleanupBlockBuffer(nn::gfx::Device* device) {
    for (int i = 0; i < bufferCount; ++i) buffers[i].Finalize(device);
    flags ^= 1;
    pool = nullptr;
    poolOffset = 0;
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
// bufferIndex selects the buffered option block receiving changed values.
void ShadingModelObj::CalculateOptionBlock(int bufferIndex) {
    if (!blockSize) return;
    dirtyFlags.Caclulate();
    int mask = 1 << bufferIndex;
    if (!(dirtyFlags.mDirtyBuffers & mask)) return;
    void* mapped = buffers[bufferIndex].Map();
    int words = (resource->ToData().staticOptionCount + sizeof(u32) * 8 - 1) / (sizeof(u32) * 8);
    for (int i = 0; i < words; ++i) {
        dirtyFlags.mDirtyBuffers &= ~mask;
        u32 value = (dirtyFlags.mBufferFlags + dirtyFlags.mWordCount * bufferIndex)[static_cast<ptrdiff_t>(i)];
        while (value) {
            int bit = HighestBit(value);
            const ResShaderOption* option = &resource->ToData().pStaticOptions.Get()[static_cast<int>(i * 32) + bit];
            if (option->blockOffset) {
                const u32* values = option->values;
                int choice = option->ReadStaticKey(optionKey);
                ptrdiff_t offset = static_cast<int>(option->blockOffset) - 1;
                u32* destination = reinterpret_cast<u32*>(static_cast<u8*>(mapped) + offset);
                *destination = values[choice];
            }
            value ^= 1u << bit;
        }
    }
    int wordCount = dirtyFlags.mWordCount;
    if (!(wordCount < 1)) memset(dirtyFlags.mBufferFlags + wordCount * bufferIndex, 0, static_cast<unsigned>(wordCount) * sizeof(u32));
    size_t size = blockSize;
    BufferImpl* buffer = &buffers[bufferIndex];
    buffer->FlushMappedRange(0, size);
    buffers[bufferIndex].Unmap();
}
void ShadingModelObj::ClearStaticKey() {
    int count = resource->ToData().staticOptionCount;
    for (int i = 0; i < count; ++i) {
        const ResShaderOption* option = &resource->ToData().pStaticOptions.Get()[i];
        int choice = option->defaultChoice;
        if (!(option->flags & 1)) option->WriteStaticKey(staticKey, choice);
        else {
            option->WriteStaticKey(optionKey, choice);
            if (option->blockOffset) {
                dirtyFlags.mPending[static_cast<unsigned>(i) >> 5] |= 1u << (i & 31);
                dirtyFlags.mFlags |= 1;
            }
        }
    }
}
// optionIndex selects a static option and choice selects its value.
void ShadingModelObj::WriteStaticKey(int optionIndex, int choice) {
    const ResShaderOption* option = &resource->ToData().pStaticOptions.Get()[optionIndex];
    if (!(option->flags & 1)) option->WriteStaticKey(staticKey, choice);
    else {
        option->WriteStaticKey(optionKey, choice);
        if (option->blockOffset) {
            dirtyFlags.mPending[optionIndex >> 5] |= 1u << (optionIndex & 31);
            dirtyFlags.mFlags |= 1;
        }
    }
}
// optionIndex selects the static option whose current choice is returned.
int ShadingModelObj::ReadStaticKey(int optionIndex) const {
    const ResShaderOption* option = &resource->ToData().pStaticOptions.Get()[optionIndex];
    return option->ReadStaticKey(option->flags & 1 ? optionKey : staticKey);
}
void ShaderSelector::InitializeArgument::CalculateMemorySize() {
    const ResShadingModel* resource = model->GetResource();
    blocks[0].Initialize(0);
    blocks[1].Initialize(0);
    blocks[2].Initialize(0);
    blocks[0].size = resource->ToData().dynamicKeyLength * sizeof(u32);
    blocks[1].size = resource->ToData().dynamicKeyLength * sizeof(u32);
    blocks[2].size = resource->ToData().dynamicKeyLength * sizeof(u32);
    memorySize = 0;
    memoryAlignment = 8;
    for (int i = 0; i < 3; ++i) {
        if (blocks[i].size) {
            size_t offset = (memorySize + 7) & ~size_t(7);
            memoryAlignment = 8;
            memorySize = offset + blocks[i].size;
            blocks[i].offset = offset;
        }
    }
}
// argument selects a model; memory provides size bytes for three packed dynamic keys.
bool ShaderSelector::Initialize(const InitializeArgument& argument, void* memory, size_t size) {
    if (!argument.memoryAlignment || argument.memorySize > size) return false;
    const ResShadingModel* resource = argument.model->GetResource();
    model = argument.model;
    dynamicKey = static_cast<u32*>(argument.blocks[0].GetPointer(memory));
    optionKey = static_cast<u32*>(argument.blocks[1].GetPointer(memory));
    previousKey = static_cast<u32*>(argument.blocks[2].GetPointer(memory));
    work = memory;
    int count = model->GetResource()->ToData().dynamicKeyLength;
    if (count) {
        resource->WriteDefaultDynamicKey(dynamicKey);
        std::copy(dynamicKey, dynamicKey + count, optionKey);
        resource->WriteInvalidDynamicKey(previousKey);
        program = nullptr;
    } else {
        int index = resource->FindProgramIndex(model->range, dynamicKey);
        program = const_cast<ResShaderProgram*>(&resource->ToData().pPrograms.Get()[index]);
    }
    return true;
}
// device initializes the program matching the current dynamic key.
bool ShaderSelector::UpdateVariation(nn::gfx::Device* device) {
    const ResShadingModel* resource = model->GetResource();
    ptrdiff_t count = resource->ToData().dynamicKeyLength;
    const u32* current = dynamicKey;
    u32* previous = previousKey;
    const u32* end = current + count;
    for (const u32* cursor = current; cursor != end; ++cursor) {
        if (*cursor != previous[cursor - current]) {
            int index = resource->FindProgramIndex(model->range, current);
            if (index == -1) {
                resource->WriteInvalidDynamicKey(previousKey);
                program = nullptr;
                return false;
            }
            std::copy(current, current + count, previous);
            program = const_cast<ResShaderProgram*>(&resource->ToData().pPrograms.Get()[index]);
            break;
        }
    }
    program->Update(device);
    return true;
}
void ShaderSelector::ClearDynamicKey() {
    int count = model->GetResource()->ToData().dynamicOptionCount;
    for (int i = 0; i < count; ++i) {
        const ResShaderOption* option = &model->GetResource()->ToData().pDynamicOptions.Get()[i];
        int choice = option->defaultChoice;
        if (!(option->flags & 1)) option->WriteDynamicKey(dynamicKey, choice);
        else option->WriteDynamicKey(optionKey, choice);
    }
}
// optionIndex selects a dynamic option and choice identifies its value.
void ShaderSelector::WriteDynamicKey(int optionIndex, int choice) {
    const ResShaderOption* option = &model->GetResource()->ToData().pDynamicOptions.Get()[optionIndex];
    if (!(option->flags & 1)) option->WriteDynamicKey(dynamicKey, choice);
    else option->WriteDynamicKey(optionKey, choice);
}
// optionIndex selects the dynamic option whose current choice is returned.
int ShaderSelector::ReadDynamicKey(int optionIndex) const {
    const ResShaderOption* option = &model->GetResource()->ToData().pDynamicOptions.Get()[optionIndex];
    return option->ReadDynamicKey(option->flags & 1 ? optionKey : dynamicKey);
}
// destination receives formatted text; capacity is its size in bytes.
int ShadingModelObj::PrintRawKeyTo(char* destination, int capacity) const {
    return ResShadingModel::PrintKeyTo(destination, capacity, staticKey, resource->ToData().staticKeyLength);
}
// destination receives formatted text; capacity is its size in bytes.
int ShadingModelObj::PrintRawOptionTo(char* destination, int capacity) const {
    return resource->PrintStaticOptionTo(destination, capacity, staticKey);
}
// destination receives formatted text; capacity is its size in bytes.
int ShadingModelObj::PrintKeyTo(char* destination, int capacity) const {
    u32 key[32] = {};
    int count = resource->ToData().staticOptionCount;
    for (int i = 0; i < count; ++i) {
        const ResShaderOption* option = &resource->ToData().pStaticOptions.Get()[i];
        int choice = option->ReadStaticKey(option->flags & 1 ? optionKey : staticKey);
        option->WriteStaticKey(key, choice);
    }
    return ResShadingModel::PrintKeyTo(destination, capacity, key, resource->ToData().staticKeyLength);
}
// destination receives formatted text; capacity is its size in bytes.
int ShadingModelObj::PrintOptionTo(char* destination, int capacity) const {
    u32 key[32] = {};
    int count = resource->ToData().staticOptionCount;
    const ResShadingModel* current = resource;
    for (int i = 0; i < count; ++i) {
        const ResShaderOption* option = &current->ToData().pStaticOptions.Get()[i];
        int choice = option->ReadStaticKey(option->flags & 1 ? optionKey : staticKey);
        option->WriteStaticKey(key, choice);
        current = resource;
    }
    return current->PrintStaticOptionTo(destination, capacity, key);
}
// destination receives formatted text; capacity is its size in bytes.
int ShaderSelector::PrintRawKeyTo(char* destination, int capacity) const {
    const u32* key = dynamicKey;
    int length = model->GetResource()->ToData().dynamicKeyLength;
    return ResShadingModel::PrintKeyTo(destination, capacity, key, length);
}
// destination receives formatted text; capacity is its size in bytes.
int ShaderSelector::PrintRawOptionTo(char* destination, int capacity) const {
    return model->GetResource()->PrintDynamicOptionTo(destination, capacity, dynamicKey);
}
// destination receives formatted text; capacity is its size in bytes.
int ShaderSelector::PrintKeyTo(char* destination, int capacity) const {
    const ResShadingModel* resource = model->GetResource();
    u32 key[32] = {};
    int count = model->GetResource()->ToData().dynamicOptionCount;
    for (int i = 0; i < count; ++i) {
        const ResShaderOption* option = &model->GetResource()->ToData().pDynamicOptions.Get()[i];
        int choice = option->ReadDynamicKey(option->flags & 1 ? optionKey : dynamicKey);
        option->WriteDynamicKey(key, choice);
    }
    return ResShadingModel::PrintKeyTo(destination, capacity, key, resource->ToData().dynamicKeyLength);
}
// destination receives formatted text; capacity is its size in bytes.
int ShaderSelector::PrintOptionTo(char* destination, int capacity) const {
    const ResShadingModel* resource = model->GetResource();
    u32 key[32] = {};
    int count = model->GetResource()->ToData().dynamicOptionCount;
    for (int i = 0; i < count; ++i) {
        const ResShaderOption* option = &model->GetResource()->ToData().pDynamicOptions.Get()[i];
        int choice = option->ReadDynamicKey(option->flags & 1 ? optionKey : dynamicKey);
        option->WriteDynamicKey(key, choice);
    }
    return resource->PrintDynamicOptionTo(destination, capacity, key);
}
}
