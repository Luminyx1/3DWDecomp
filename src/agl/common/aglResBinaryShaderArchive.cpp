#include "common/aglResBinaryShaderArchive.h"
#include <nvn/nvn_FuncPtrInline.h>
#include "common/aglResShaderArchive.h"
#include "driver/aglNVNMgr.h"

// agl::ResShaderArchiveData::cEndianCheckBit is defined out of line in the original
extern const u32 sResShaderArchiveEndianCheckBit asm("_ZN3agl20ResShaderArchiveData15cEndianCheckBitE");

namespace agl {

namespace {

s32 toOffset(const void* ptr)
{
    return static_cast<s32>(reinterpret_cast<uintptr_t>(ptr));
}

template <typename T>
T* resolveVarPtr(const void* pNameBase, const void* pBase, T* pVar, s32 num)
{
    if (pVar == nullptr) {
        return nullptr;
    }

    const char* name_base = static_cast<const char*>(pNameBase ? pNameBase : pBase);
    T* var = reinterpret_cast<T*>(const_cast<char*>(static_cast<const char*>(pBase)) +
                                  toOffset(pVar));

    for (s32 i = 0; i < num; i++) {
        var[i].mName = name_base + toOffset(var[i].mName);
    }

    return var;
}

agl::ResShaderUniformBlockArray getResShaderUniformBlockArray(const agl::ResBinaryShaderProgram& rProgram)
{
    const auto* const data = rProgram.getResShaderVariationDefaultArray().ptr();
    return reinterpret_cast<const agl::ResShaderUniformBlockArray::DataType*>(
        reinterpret_cast<const char*>(data) + data->mSize);
}

size_t calcMemoryPoolSize(u32 size)
{
    return size_t(size) + 0xfff - ((size + 0xfff) % 0x1000);
}

}  // namespace

/**
 * Gets the supported archive version.
 * @return the archive version
 */
u32 ResBinaryShaderArchiveData::getVersion()
{
    return cVersion;
}

/**
 * Gets the archive signature ("SHAB").
 * @return the archive signature
 */
u32 ResBinaryShaderArchiveData::getSignature()
{
    return cSignature;
}

/**
 * Gets the archive file extension.
 * @return the file extension
 */
const char* ResBinaryShaderArchiveData::getExtension()
{
    return "sharcb";
}

/**
 * Resolves the offsets of the archive and creates the shader code buffer.
 * @param le_resolve_pointers whether to resolve pointers
 * @return whether the archive was set up
 */
// NON_MATCHING: block layout of the unswitched resolvePtr loops, spill slot order
bool ResBinaryShaderArchive::setUp(bool le_resolve_pointers)
{
    const bool endian_resolved = (ref().mEndian & sResShaderArchiveEndianCheckBit) != 0;

    if (!endian_resolved) {
        ModifyEndianU32(modifyEndian(), ptr(), sizeof(DataType));
    }

    createMemoryPoolBuffer_();

    const u32 flags = ref().mEndian;
    ResShaderBinaryArray binary_arr = getResShaderBinaryArray();

    if (endian_resolved) {
        if (!(flags & DataType::cPtrResolvedBit)) {
            for (auto it = binary_arr.begin(), it_end = binary_arr.end(); it != it_end; ++it) {
                ResShaderBinary(&(*it)).resolvePtr(
                    (flags & DataType::cNameBaseBit) ? ptr() + 1 : nullptr, false);
            }

            ref().mEndian |= DataType::cPtrResolvedBit;
        }
    } else {
        binary_arr.modifyEndianArray(modifyEndian());

        ResBinaryShaderProgramArray binary_prog_arr = getResBinaryShaderProgramArray();
        binary_prog_arr.modifyEndianArray(modifyEndian());

        for (auto it = binary_prog_arr.begin(), it_end = binary_prog_arr.end(); it != it_end; ++it) {
            ResBinaryShaderProgram binary_prog(&(*it));

            binary_prog.getResShaderVariationArray().modifyEndianArray(modifyEndian());
            binary_prog.getResShaderVariationDefaultArray().modifyEndianArray(modifyEndian());

            ResShaderUniformBlockArray block_arr = getResShaderUniformBlockArray(binary_prog);
            block_arr.modifyEndianArray(modifyEndian());
            for (auto block = block_arr.begin(), block_end = block_arr.end(); block != block_end;
                 ++block) {
                ResShaderUniformBlock(&(*block)).getResShaderUniformArray().modifyEndianArray(
                    modifyEndian());
            }
        }

        for (auto it = binary_arr.begin(), it_end = binary_arr.end(); it != it_end; ++it) {
            ResShaderBinary binary(&(*it));
            binary.modifyBinaryEndian();

            if (le_resolve_pointers && !(flags & DataType::cPtrResolvedBit)) {
                binary.resolvePtr((flags & DataType::cNameBaseBit) ? ptr() + 1 : nullptr, false);
            }
        }

        if (le_resolve_pointers && !(flags & DataType::cPtrResolvedBit)) {
            ref().mEndian |= DataType::cPtrResolvedBit;
        }

        setEndianResolved();
    }

    return true;
}

/**
 * Creates the memory pool and buffer holding the shader code of the archive.
 */
void ResBinaryShaderArchive::createMemoryPoolBuffer_()
{
    void* storage = reinterpret_cast<void*>(ref().mMemoryPoolOffset + reinterpret_cast<uintptr_t>(ptr()));
    NVNdevice* device = driver::NVNMgr::instance()->getNvnDevice();
    NVNmemoryPool* pool = &ref().mMemoryPool;

    NVNmemoryPoolBuilder pool_builder;
    nvnMemoryPoolBuilderSetDefaults(&pool_builder);
    nvnMemoryPoolBuilderSetDevice(&pool_builder, device);
    nvnMemoryPoolBuilderSetFlags(&pool_builder, NVN_MEMORY_POOL_FLAGS_CPU_UNCACHED |
                                                    NVN_MEMORY_POOL_FLAGS_GPU_CACHED |
                                                    NVN_MEMORY_POOL_FLAGS_SHADER_CODE);
    nvnMemoryPoolBuilderSetStorage(&pool_builder, storage,
                                   calcMemoryPoolSize(ref().mMemoryPoolSize));
    nvnMemoryPoolInitialize(pool, &pool_builder);

    NVNbufferBuilder buffer_builder;
    nvnBufferBuilderSetDevice(&buffer_builder, device);
    nvnBufferBuilderSetDefaults(&buffer_builder);
    nvnBufferBuilderSetStorage(&buffer_builder, pool, 0,
                               calcMemoryPoolSize(ref().mMemoryPoolSize));
    nvnBufferInitialize(&ref().mBuffer, &buffer_builder);
}

/**
 * Converts the relative offsets of the compiled shader header and its symbol tables to pointers.
 * @param pNameBase base address of the symbol names, or nullptr if they are relative to the header
 */
void ResShaderBinary::resolvePtr(const void* pNameBase, bool)
{
    ResShaderBinaryInfo* info = getInfo();

    info->mCode = reinterpret_cast<const char*>(info) + toOffset(info->mCode);
    info->mVar0 = resolveVarPtr(pNameBase, info, info->mVar0, info->mVar0Num);
    info->mVar1 = resolveVarPtr(pNameBase, info, info->mVar1, info->mVar1Num);
    info->mVar2 = resolveVarPtr(pNameBase, info, info->mVar2, info->mVar2Num);
    info->mVar3 = resolveVarPtr(pNameBase, info, info->mVar3, info->mVar3Num);
    info->mVar4 = resolveVarPtr(pNameBase, info, info->mVar4, info->mVar4Num);
    info->mVar5 = resolveVarPtr(pNameBase, info, info->mVar5, info->mVar5Num);
}

/**
 * Converts the compiled shader to host endianness (nothing to do on this platform).
 */
void ResShaderBinary::modifyBinaryEndian() {}

/**
 * Releases the NVN buffer and memory pool of the archive.
 */
void ResBinaryShaderArchive::cleanUp()
{
    nvnBufferFinalize(&ref().mBuffer);
    nvnMemoryPoolFinalize(&ref().mMemoryPool);
}

}  // namespace agl
