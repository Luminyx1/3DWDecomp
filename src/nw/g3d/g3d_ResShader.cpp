#include <nn/g3d/g3d_ResShader.h>
#include <nn/os/os_Mutex.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace nn::g3d {
// device owns the temporary shader used to test the binary representation.
bool ResShaderProgram::IsBinaryAvailable(nn::gfx::Device* device) {
    bool available = InitializePerType(device, nn::gfx::ShaderCodeType_Binary);
    Cleanup(device);
    return available;
}

// device owns the shader; type selects its stored code representation.
bool ResShaderProgram::InitializePerType(nn::gfx::Device* device, nn::gfx::ShaderCodeType type) {
    nn::gfx::ResShaderProgram* program = variation->GetResShaderProgram(type);
    if ((program == nullptr) || program->Initialize(device) != 0)
        return false;
    switch (type) {
    case nn::gfx::ShaderCodeType_Binary:
        flags |= 4;
        break;
    case nn::gfx::ShaderCodeType_Ir:
        flags |= 16;
        break;
    case nn::gfx::ShaderCodeType_Source:
        flags |= 8;
        break;
    default:
        break;
    }

    flags |= 2;
    return true;
}

// device owns each initialized shader representation being released.
void ResShaderProgram::Cleanup(nn::gfx::Device* device) {
    if (flags & 2) {
        nn::gfx::ResShaderVariation* resource = variation;
        if (flags & 4)
            resource->GetResShaderProgram(nn::gfx::ShaderCodeType_Binary)->Finalize(device);
        if (flags & 16)
            resource->GetResShaderProgram(nn::gfx::ShaderCodeType_Ir)->Finalize(device);
        if (flags & 8)
            resource->GetResShaderProgram(nn::gfx::ShaderCodeType_Source)->Finalize(device);
    }

    flags = 0;
}

// device is unused until the pending program update is performed.
void ResShaderProgram::Setup(nn::gfx::Device* device) { flags |= 1; }
// device owns the shader; skipBinary starts the fallback sequence with intermediate code.
void ResShaderProgram::Initialize(nn::gfx::Device* device, bool skipBinary) {
    if (!skipBinary && InitializePerType(device, nn::gfx::ShaderCodeType_Binary))
        return;
    if (InitializePerType(device, nn::gfx::ShaderCodeType_Ir))
        return;
    InitializePerType(device, nn::gfx::ShaderCodeType_Source);
}

// shader supplies Interface bindings; name identifies a resource in stage, or null for an absent binding.
template <nn::gfx::ShaderInterfaceType Interface>
static inline int GetInterfaceSlot(const nn::gfx::Shader* shader, const nn::util::BinPtrToString& name,
                                   nn::gfx::ShaderStage stage) {
    return (name.Get() != nullptr) ? shader->GetInterfaceSlot(stage, Interface, name.Get()->GetData()) : -1;
}

// names supplies six stage names for each of count resources; table receives slots from shader.
template <nn::gfx::ShaderInterfaceType Interface>
static void UpdateSlots(const nn::gfx::Shader* shader, s32* table, const nn::util::BinPtrToString* names,
                        int count) {
    for (int i = 0; i < count; ++i, names += 6) {
        const nn::util::BinPtrToString* stageNames = names;
        s32* slots = table + i * 6;
        slots[0] = GetInterfaceSlot<Interface>(shader, stageNames[0], nn::gfx::ShaderStage_Vertex);
        slots[1] = GetInterfaceSlot<Interface>(shader, stageNames[1], nn::gfx::ShaderStage_Geometry);
        slots[2] = GetInterfaceSlot<Interface>(shader, stageNames[2], nn::gfx::ShaderStage_Pixel);
        slots[3] = GetInterfaceSlot<Interface>(shader, stageNames[3], nn::gfx::ShaderStage_Compute);
        slots[4] = GetInterfaceSlot<Interface>(shader, stageNames[4], nn::gfx::ShaderStage_Hull);
        slots[5] = GetInterfaceSlot<Interface>(shader, stageNames[5], nn::gfx::ShaderStage_Domain);
    }
}

void ResShaderProgram::UpdateTable() {
    ResShadingModel* resource = model;
    nn::gfx::ShaderCodeType type = GetCodeType();
    ShaderSymbolInfo* symbols = resource->ToData().symbolInfo;
    const nn::gfx::Shader* shader = variation->GetResShaderProgram(type)->GetShader();
    {
        int count = resource->ToData().samplerCount;
        if (count) {
            s32* table = pSamplerTable.Get();
            const nn::util::BinPtrToString* names = symbols->samplerNames;
            UpdateSlots<nn::gfx::ShaderInterfaceType_Sampler>(shader, table, names, count);
        }
    }

    {
        int count = resource->ToData().imageCount;
        if (count) {
            s32* table = pImageTable.Get();
            const nn::util::BinPtrToString* names = symbols->imageNames;
            UpdateSlots<nn::gfx::ShaderInterfaceType_Image>(shader, table, names, count);
        }
    }

    {
        int count = resource->ToData().uniformBlockCount;
        if (count) {
            s32* table = pUniformBlockTable.Get();
            const nn::util::BinPtrToString* names = symbols->uniformBlockNames;
            UpdateSlots<nn::gfx::ShaderInterfaceType_ConstantBuffer>(shader, table, names, count);
        }
    }

    {
        int count = resource->ToData().storageBlockCount;
        if (count) {
            s32* table = pStorageBlockTable.Get();
            const nn::util::BinPtrToString* names = symbols->storageBlockNames;
            UpdateSlots<nn::gfx::ShaderInterfaceType_UnorderedAccessBuffer>(shader, table, names, count);
        }
    }
}

// device is passed to the archive callback; the model mutex protects lazy initialization.
void ResShaderProgram::Update(nn::gfx::Device* device) {
    if (!(flags & 1))
        return;
    ResShadingModel* resource = model;
    nn::os::MutexType* lock = resource->ToData().mutex;
    if (lock != nullptr) {
        nn::os::LockMutex(lock);
        if (flags & 1)
            resource->UpdateProgram(device, this - resource->ToData().pPrograms.Get());
        nn::os::UnlockMutex(lock);
    } else {
        resource->UpdateProgram(device, this - resource->ToData().pPrograms.Get());
    }
}

// device and index identify the shader program requested from the archive callback.
void ResShadingModel::UpdateProgram(nn::gfx::Device* device, int index) {
    if (archive->updateProgram != nullptr)
        archive->updateProgram(device, this, index);
}

const nn::gfx::Shader* ResShaderProgram::GetShader() const {
    nn::gfx::ShaderCodeType type = GetCodeType();
    if (type == nn::gfx::ShaderCodeType_End)
        return nullptr;
    return variation->GetResShaderProgram(type)->GetShader();
}

// commandBuffer receives the selected shader at every stage.
void ResShaderProgram::Load(nn::gfx::CommandBuffer* commandBuffer) const {
    nn::gfx::ShaderCodeType type = GetCodeType();
    if (type == nn::gfx::ShaderCodeType_End)
        return;
    const nn::gfx::Shader* shader = variation->GetResShaderProgram(type)->GetShader();
    commandBuffer->SetShader(shader, 0x3f);
}

// key receives choice in this option's packed static word and bit range.
void ResShaderOption::WriteStaticKey(u32* key, int choice) const {
    key[wordIndex] = (key[wordIndex] & ~mask) | (u32(choice) << shift);
}

// key contains packed static words from which this option is decoded.
int ResShaderOption::ReadStaticKey(const u32* key) const { return (key[wordIndex] & mask) >> shift; }
// key receives choice relative to the first dynamic key word.
void ResShaderOption::WriteDynamicKey(u32* key, int choice) const {
    ptrdiff_t index = ptrdiff_t(wordIndex) - ptrdiff_t(dynamicWordOffset);
    key[index] = (key[index] & ~mask) | (u32(choice) << shift);
}

// key starts at the dynamic portion of a program key.
int ResShaderOption::ReadDynamicKey(const u32* key) const {
    ptrdiff_t index = ptrdiff_t(wordIndex) - ptrdiff_t(dynamicWordOffset);
    return (key[index] & mask) >> shift;
}

void ResShadingModel::Relocate() {
    nn::gfx::ResShaderFile* file = shaderFile;
    if ((file != nullptr) && !file->GetBinaryFileHeader()->IsRelocated())
        file->GetBinaryFileHeader()->GetRelocationTable()->Relocate();
}

void ResShadingModel::Unrelocate() {
    nn::gfx::ResShaderFile* file = shaderFile;
    if ((file != nullptr) && file->GetBinaryFileHeader()->IsRelocated())
        file->GetBinaryFileHeader()->GetRelocationTable()->Unrelocate();
}

// device is used to probe the first program while its container is temporarily initialized.
bool ResShadingModel::IsBinaryAvailable(nn::gfx::Device* device) {
    if (!programCount)
        return false;
    nn::gfx::ResShaderContainer* container = shaderFile->GetShaderContainer();
    container->Initialize(device);
    bool available = pPrograms.Get()[0].IsBinaryAvailable(device);
    container->Finalize(device);
    return available;
}

// device owns the shader container; lock optionally protects later lazy updates.
void ResShadingModel::Setup(nn::gfx::Device* device, nn::os::MutexType* lock) {
    shaderFile->GetShaderContainer()->Initialize(device);
    int count = programCount;
    for (int i = 0; i < count; ++i)
        pPrograms.Get()[i].Setup(device);
    mutex = lock;
}

// device and pool provide size bytes at offset; lock serializes lazy program updates.
void ResShadingModel::Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset, size_t size,
                            nn::os::MutexType* lock) {
    nn::gfx::ResShaderContainer* container = shaderFile->GetShaderContainer();
    container->Initialize(device, pool,
                          offset + (reinterpret_cast<u8*>(container) - reinterpret_cast<u8*>(this)), size);
    int count = programCount;
    for (int i = 0; i < count; ++i)
        pPrograms.Get()[i].Setup(device);
    mutex = lock;
}

// device owns the programs and container being finalized.
void ResShadingModel::Cleanup(nn::gfx::Device* device) {
    int count = programCount;
    for (int i = 0; i < count; ++i)
        pPrograms.Get()[i].Cleanup(device);
    shaderFile->GetShaderContainer()->Finalize(device);
    mutex = nullptr;
}

// index selects a program's complete packed key; static words precede dynamic words.
const u32* ResShadingModel::GetKey(int index) const {
    const u8* base = reinterpret_cast<const u8*>(pKeyTable.Get());
    int byteOffset = index * (staticKeyLength + dynamicKeyLength) * sizeof(u32);
    return reinterpret_cast<const u32*>(base + byteOffset);
}

// index selects the program whose static key is returned.
const u32* ResShadingModel::GetStaticKey(int index) const { return GetKey(index); }
// index selects the program whose dynamic key is returned.
const u32* ResShadingModel::GetDynamicKey(int index) const {
    const u32* first = pKeyTable.Get() + staticKeyLength;
    return first + index * (staticKeyLength + dynamicKeyLength);
}

// key receives either the default program key or the defaults encoded by individual options.
void ResShadingModel::WriteDefaultStaticKey(u32* key) const {
    if (defaultProgramIndex != -1) {
        const u32* first = GetStaticKey(defaultProgramIndex);
        std::copy(first, first + staticKeyLength, key);
    } else {
        std::fill(key, key + staticKeyLength, 0);
        int count = staticOptionCount;
        for (int i = 0; i < count; ++i) {
            const ResShaderOption& option = pStaticOptions.Get()[i];
            option.WriteStaticKey(key, option.defaultChoice);
        }
    }
}

// key receives the dynamic defaults for the model's default program or option descriptions.
void ResShadingModel::WriteDefaultDynamicKey(u32* key) const {
    if (defaultProgramIndex != -1) {
        const u32* first = GetDynamicKey(defaultProgramIndex);
        std::copy(first, first + dynamicKeyLength, key);
    } else {
        std::fill(key, key + dynamicKeyLength, 0);
        int count = dynamicOptionCount;
        for (int i = 0; i < count; ++i) {
            const ResShaderOption& option = pDynamicOptions.Get()[i];
            option.WriteDynamicKey(key, option.defaultChoice);
        }
    }
}

// key receives all-one words to invalidate a previously selected dynamic variant.
void ResShadingModel::WriteInvalidDynamicKey(u32* key) const {
    std::fill(key, key + dynamicKeyLength, ~u32(0));
}

// first and second contain length unsigned key words in lexicographic order.
static bool KeyLess(const u32* first, const u32* second, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        if (first[i] != second[i])
            return first[i] < second[i];
    }

    return false;
}

// first/end delimit stride-word keys; advance first to the lower bound of key using length words.
static const u32* LowerKey(const u32*& first, const u32* end, const u32* key, size_t stride, size_t length) {
    if (length && size_t(end - first) >= stride) {
        size_t count = size_t(end - first) / stride;
        while (count > 0) {
            size_t half = count / 2;
            const u32* middle = first + half * stride;
            if (KeyLess(middle, key, length)) {
                first = middle + stride;
                count -= half + 1;
            } else
                count = half;
        }
    }

    return first;
}

// first/end delimit stride-word keys; find the first key above the requested length words.
static const u32* UpperKey(const u32* first, const u32* end, const u32* key, size_t stride, size_t length) {
    if (size_t(end - first) < stride)
        return first;
    size_t count = size_t(end - first) / stride;
    while (count) {
        size_t half = count / 2;
        const u32* middle = first + half * stride;
        if (!KeyLess(key, middle, length)) {
            first = middle + stride;
            count -= half + 1;
        } else
            count = half;
    }

    return first;
}

// key specifies all static and dynamic options of the desired program.
int ResShadingModel::FindProgramIndex(const u32* key) const {
    size_t stride = size_t(staticKeyLength) + dynamicKeyLength;
    const u32* first = pKeyTable.Get();
    const u32* end = first + stride * programCount;
    const u32* begin = first;
    const u32* found = LowerKey(begin, end, key, stride, stride);
    if (end == found || KeyLess(key, found, stride))
        return -1;
    return size_t(found - first) / stride;
}

// range limits static-compatible programs; key specifies their dynamic option words.
int ResShadingModel::FindProgramIndex(const ShaderRange& range, const u32* key) const {
    const u32* begin = range.begin;
    size_t stride = size_t(staticKeyLength) + dynamicKeyLength;
    const u32* found = LowerKey(begin, range.end, key, stride, dynamicKeyLength);
    if (range.end == found || KeyLess(key, found, dynamicKeyLength))
        return -1;
    return size_t(found - (pKeyTable.Get() + staticKeyLength)) / stride;
}

// range receives dynamic-key pointers spanning every program with the static words in key.
bool ResShadingModel::FindProgramRange(ShaderRange* range, const u32* key) const {
    size_t length = staticKeyLength;
    size_t stride = length + dynamicKeyLength;
    const u32* first = pKeyTable.Get();
    const u32* end = first + stride * programCount;
    const u32* last = first;
    if (size_t(end - first) >= stride) {
        size_t count = size_t(end - first) / stride;
        while (count) {
            size_t half = count / 2;
            const u32* middle = first + half * stride;
            if (KeyLess(middle, key, length)) {
                first = middle + stride;
                count -= half + 1;
            } else if (KeyLess(key, middle, length)) {
                end = middle;
                count = half;
            } else {
                first = LowerKey(first, middle, key, stride, length);
                last = UpperKey(middle + stride, end, key, stride, length);
                range->begin = first + length;
                range->end = last + length;
                return first != last;
            }
        }

        last = first;
    }

    range->begin = first + length;
    range->end = last + length;
    return first != last;
}

// destination receives capacity bytes of text; key contains length packed key words.
int ResShadingModel::PrintKeyTo(char* destination, size_t capacity, const u32* key, int length) {
    size_t required = length < 1 ? size_t(0) : size_t(length * 9 - 1);
    if (destination == nullptr)
        return required;
    if (required + 1 > capacity) {
        *destination = 0;
        return -1;
    }

    if (!required) {
        *destination = 0;
        return 0;
    }

    {
        ptrdiff_t i = 0;
        do {
            std::sprintf(destination, "%08X_", key[i]);
            destination += 9;
        } while (++i < length);
        destination[-1] = 0;
    }

    return required;
}

// destination receives capacity bytes of option text; key supplies packed static choices.
int ResShadingModel::PrintStaticOptionTo(char* destination, size_t capacity, const u32* key) const {
    int count = staticOptionCount;
    size_t required = 0;
    if (count) {
        const ResShaderOption* options = pStaticOptions.Get();
        for (int i = 0; i < count; ++i) {
            size_t nameLength = std::strlen(options[i].name.Get()->GetData());
            int choice = options[i].ReadStaticKey(key);
            required += nameLength + std::strlen(options[i].choiceDictionary->GetKey(choice).data()) + 2;
        }
    }

    if (required)
        --required;
    if (destination == nullptr)
        return required;
    if (required + 1 > capacity) {
        *destination = 0;
        return -1;
    }

    if (!required) {
        *destination = 0;
        return 0;
    }

    {
        for (int i = 0; i < count; ++i) {
            const ResShaderOption* option = &pStaticOptions.Get()[i];
            const char* name = option->name.Get()->GetData();
            size_t size = std::strlen(name);
            std::memcpy(destination, name, size);
            destination += size;
            *destination++ = ':';
            int choice = option->ReadStaticKey(key);
            name = option->choiceDictionary->GetKey(choice).data();
            size = std::strlen(name);
            std::memcpy(destination, name, size);
            destination += size;
            *destination++ = '\t';
        }

        destination[-1] = 0;
    }

    return required;
}

// destination receives capacity bytes of option text; key supplies packed dynamic choices.
int ResShadingModel::PrintDynamicOptionTo(char* destination, size_t capacity, const u32* key) const {
    int count = dynamicOptionCount;
    size_t required = 0;
    if (count) {
        const ResShaderOption* options = pDynamicOptions.Get();
        for (int i = 0; i < count; ++i) {
            size_t nameLength = std::strlen(options[i].name.Get()->GetData());
            int choice = options[i].ReadDynamicKey(key);
            required += nameLength + std::strlen(options[i].choiceDictionary->GetKey(choice).data()) + 2;
        }
    }

    if (required)
        --required;
    if (destination == nullptr)
        return required;
    if (required + 1 > capacity) {
        *destination = 0;
        return -1;
    }

    if (!required) {
        *destination = 0;
        return 0;
    }

    {
        for (int i = 0; i < count; ++i) {
            const ResShaderOption* option = &pDynamicOptions.Get()[i];
            const char* name = option->name.Get()->GetData();
            size_t size = std::strlen(name);
            std::memcpy(destination, name, size);
            destination += size;
            *destination++ = ':';
            int choice = option->ReadDynamicKey(key);
            name = option->choiceDictionary->GetKey(choice).data();
            size = std::strlen(name);
            std::memcpy(destination, name, size);
            destination += size;
            *destination++ = '\t';
        }

        destination[-1] = 0;
    }

    return required;
}

void ResShaderFile::Relocate() {
    if (!fileHeader.IsRelocated())
        fileHeader.GetRelocationTable()->Relocate();
    ResShaderArchive* resource = archive;
    int count = resource->modelCount;
    for (int i = 0; i < count; ++i)
        resource->models[i].Relocate();
}

void ResShaderFile::Unrelocate() {
    ResShaderArchive* resource = archive;
    int count = resource->modelCount;
    for (int i = 0; i < count; ++i)
        resource->models[i].Unrelocate();
    if (fileHeader.IsRelocated())
        fileHeader.GetRelocationTable()->Unrelocate();
}

// file supplies mutable resource storage whose offsets are converted to pointers.
ResShaderFile* ResShaderFile::ResCast(void* file) {
    ResShaderFile* resource = static_cast<ResShaderFile*>(file);
    resource->Relocate();
    resource->fileHeader.IsEndianReverse();
    return resource;
}

// file points to a shader archive file header; validate signature and version.
bool ResShaderFile::IsValid(const void* file) {
    return static_cast<const nn::util::BinaryFileHeader*>(file)->IsValid(0x2020202041485346, 8, 0, 0);
}

// device and pool provide poolSize bytes at offset; memory supplies size bytes of mutex storage.
void ResShaderFile::Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset,
                          size_t poolSize, void* memory, size_t size) {
    archive->Setup(device, pool, offset + (reinterpret_cast<u8*>(archive) - reinterpret_cast<u8*>(this)),
                   poolSize, memory, size);
}

// device owns model resources; release their shaders and the optional workspace mutexes.
void ResShaderArchive::Cleanup(nn::gfx::Device* device) {
    int count = modelCount;
    nn::os::MutexType* lock = static_cast<nn::os::MutexType*>(work);
    for (int i = 0; i < count; ++i) {
        models[i].Cleanup(device);
        if (work != nullptr) {
            nn::os::FinalizeMutex(lock);
            ++lock;
        }
    }

    work = nullptr;
    flags &= ~0x80;
}

// device owns the shader; resource and index select a lazily initialized program.
static void DefaultUpdate(nn::gfx::Device* device, ResShadingModel* resource,
                          int index) asm("sub_7100608DD0");
static void DefaultUpdate(nn::gfx::Device* device, ResShadingModel* resource, int index) {
    ResShaderProgram* program = &resource->ToData().pPrograms.Get()[index];
    program->Initialize(device, !(resource->ToData().archive->flags & 8));
    const nn::gfx::ResShaderFile* file = resource->ToData().shaderFile;
    if (file->GetShaderContainer()->ToData().targetApiType == 1)
        program->UpdateTable();
    // Publish shader and interface-table writes before clearing the pending-update flag.
    __builtin_arm_dmb(0xa);
    program->ToData().flags &= ~1;
}

// archive supplies model resources; device probes whether their binary shaders are usable.
static bool CheckBinary(ResShaderArchive* archive, nn::gfx::Device* device) {
    if (!archive->modelCount)
        return false;
    if (archive->flags & 8)
        return true;
    bool available = archive->models[0].IsBinaryAvailable(device);
    if (available)
        archive->flags |= 8;
    return available;
}

// device owns shaders; memory supplies size bytes for optional per-model mutexes.
void ResShaderArchive::Setup(nn::gfx::Device* device, void* memory, size_t size) {
    updateProgram = DefaultUpdate;
    int count = modelCount;
    if (flags & 2)
        CheckBinary(this, device);
    if (count > 0) {
        if (memory != nullptr) {
            nn::os::MutexType* lock = static_cast<nn::os::MutexType*>(memory);
            for (int i = 0; i < count; ++i) {
                ResShadingModel* resource = &models[i];
                nn::os::InitializeMutex(lock, false, 0);
                nn::os::MutexType* current = lock++;
                resource->Setup(device, current);
            }
        } else {
            for (int i = 0; i < count; ++i)
                models[i].Setup(device, nullptr);
        }
    }

    work = memory;
    flags |= 0x80;
}

// device and pool supply poolSize bytes at offset; memory holds size bytes of mutex workspace.
void ResShaderArchive::Setup(nn::gfx::Device* device, nn::gfx::MemoryPool* pool, ptrdiff_t offset,
                             size_t poolSize, void* memory, size_t size) {
    updateProgram = DefaultUpdate;
    int count = modelCount;
    if (flags & 2)
        CheckBinary(this, device);
    if (count > 0) {
        if (memory != nullptr) {
            nn::os::MutexType* lock = static_cast<nn::os::MutexType*>(memory);
            for (int i = 0; i < count; ++i) {
                ResShadingModel* resource = &models[i];
                ptrdiff_t modelOffset =
                    offset + (reinterpret_cast<u8*>(resource) - reinterpret_cast<u8*>(this));
                nn::os::InitializeMutex(lock, false, 0);
                nn::os::MutexType* current = lock++;
                resource->Setup(device, pool, modelOffset, poolSize, current);
            }
        } else {
            for (int i = 0; i < count; ++i) {
                ResShadingModel* resource = &models[i];
                ptrdiff_t modelOffset =
                    offset + (reinterpret_cast<u8*>(resource) - reinterpret_cast<u8*>(this));
                resource->Setup(device, pool, modelOffset, poolSize, nullptr);
            }
        }
    }

    work = memory;
    flags |= 0x80;
}
} // namespace nn::g3d
