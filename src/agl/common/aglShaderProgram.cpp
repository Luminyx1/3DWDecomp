#include "common/aglShaderProgram.h"
#include <codec/seadHashCRC32.h>
#include <nvn/nvn_FuncPtrInline.h>
#include <prim/seadScopedLock.h>
#include "common/aglDrawContext.h"
#include "common/aglResShaderBinary.h"
#include "common/aglUniformBlock.h"
#include "driver/aglNVNMgr.h"

extern void* sDynamicUniformBlockInstance asm("_ZN3agl6detail19DynamicUniformBlock9sInstanceE");

namespace agl {

namespace {

const u32 cStageBits[cShaderType_Num] = {
    NVN_SHADER_STAGE_VERTEX_BIT,
    NVN_SHADER_STAGE_FRAGMENT_BIT,
    NVN_SHADER_STAGE_GEOMETRY_BIT,
    NVN_SHADER_STAGE_COMPUTE_BIT,
};

NVNbufferAddress getDynamicUniformBlockAddress()
{
    return *reinterpret_cast<const NVNbufferAddress*>(
        static_cast<const u8*>(sDynamicUniformBlockInstance) + 0x90);
}

u64 getDynamicUniformBlockSize()
{
    return *reinterpret_cast<const u64*>(static_cast<const u8*>(sDynamicUniformBlockInstance) +
                                         0x38);
}

const ResShaderBinaryVar16* findUniformBlock(const ResShaderBinaryInfo* pInfo, const char* pName)
{
    for (u32 i = 0; i < u32(pInfo->mVar0Num); i++) {
        if (std::strcmp(pInfo->mVar0[i].mName, pName) == 0) {
            return &pInfo->mVar0[i];
        }
    }

    return nullptr;
}

}  // namespace

/**
 * Constructs an empty shader program.
 */
ShaderProgram::ShaderProgram()
    : mVariation(nullptr), mFlags(0), mVariationIndex(0), mBufferAddress(0), mStageFlags(0)
{
}

/**
 * Destroys the shader program.
 */
ShaderProgram::~ShaderProgram()
{
    destroyLocationBuffers();

    if (mVariation->_68 != nullptr && mDisplayList.getBuffer().isValid()) {
        mDisplayList.getBuffer().deleteGPUMemBlock();
    }

    if (mVariationIndex == 0) {
        if (mVariation != nullptr) {
            delete mVariation;
        }

        mVariation = nullptr;
    }

    if (mFlags & cFlag_Initialized) {
        nvnProgramFinalize(&mProgram);
        mFlags &= ~cFlag_Initialized;
    }
}

/**
 * Releases resources (nothing to do on this platform).
 */
void ShaderProgram::cleanUp() {}

/**
 * Creates the shared variation data.
 * @param rName program name
 * @param macroNum number of variation macros
 * @param pHeap heap to allocate from
 */
void ShaderProgram::initializeVariation(const sead::SafeString& rName, s32 macroNum,
                                        sead::Heap* pHeap)
{
    mVariation = new (pHeap, 8) Variation();
    mVariation->mListener = nullptr;
    mVariation->mName = rName;
    mVariation->mVariationBuffer.mProgram = this;
    mVariation->mShaderMode = cShaderMode_Invalid + 1;
    mVariation->mRegisterUniformBlockLocation = 0;
    mVariation->_68 = nullptr;
    mDisplayList.setName(mVariation->mName.cstr());
    mVariation->mDefaultVariationArray = nullptr;
    mVariation->mUniformBlockArray = nullptr;
    mVariation->mVariationBuffer.initialize(macroNum, pHeap);
}

/**
 * Allocates the variation macros.
 * @param macroNum number of macros
 * @param pHeap heap to allocate from
 */
void ShaderProgram::VariationBuffer::initialize(s32 macroNum, sead::Heap* pHeap)
{
    mMacros.tryAllocBuffer(macroNum, pHeap);
}

/**
 * Sets up a variation macro.
 * @param index macro index
 * @param rName macro name
 * @param rID macro ID
 * @param valueNum number of values
 * @param pHeap heap to allocate from
 */
void ShaderProgram::createVariationMacro(s32 index, const sead::SafeString& rName,
                                         const sead::SafeString& rID, s32 valueNum,
                                         sead::Heap* pHeap)
{
    mVariation->mVariationBuffer.createMacro(index, rName, rID, valueNum, pHeap);
}

/**
 * Sets up a variation macro.
 * @param index macro index
 * @param rName macro name
 * @param rID macro ID
 * @param valueNum number of values
 * @param pHeap heap to allocate from
 */
void ShaderProgram::VariationBuffer::createMacro(s32 index, const sead::SafeString& rName,
                                                 const sead::SafeString& rID, s32 valueNum,
                                                 sead::Heap* pHeap)
{
    Macro& macro = mMacros[index];
    macro.mName = rName;
    macro.mID = rID;
    macro.mStride = 1;
    macro.mValues.tryAllocBuffer(valueNum, pHeap);
}

/**
 * Sets a value of a variation macro.
 * @param macroIndex macro index
 * @param valueIndex value index
 * @param rValue value
 */
void ShaderProgram::setVariationMacroValue(s32 macroIndex, s32 valueIndex,
                                           const sead::SafeString& rValue)
{
    mVariation->mVariationBuffer.setMacroValue(macroIndex, valueIndex, rValue);
}

/**
 * Sets a value of a variation macro.
 * @param macroIndex macro index
 * @param valueIndex value index
 * @param rValue value
 */
void ShaderProgram::VariationBuffer::setMacroValue(s32 macroIndex, s32 valueIndex,
                                                   const sead::SafeString& rValue)
{
    mMacros[macroIndex].mValues[valueIndex] = rValue;
}

/**
 * Creates the programs of all variations.
 * @param pHeap heap to allocate from
 */
void ShaderProgram::createVariation(sead::Heap* pHeap)
{
    mVariation->mVariationBuffer.create(pHeap);
}

/**
 * Calculates the macro strides and creates the programs of all variations except the default one.
 * @param pHeap heap to allocate from
 */
void ShaderProgram::VariationBuffer::create(sead::Heap* pHeap)
{
    s32 program_num = 1;

    for (auto it = mMacros.begin(), it_end = mMacros.end(); it != it_end; ++it) {
        for (s32 i = it.getIndex() + 1; i < mMacros.size(); i++) {
            it->mStride *= mMacros[i].mValues.size();
        }

        program_num *= it->mValues.size();
    }

    mPrograms.tryAllocBuffer(program_num - 1, pHeap);

    for (auto it = mPrograms.begin(), it_end = mPrograms.end(); it != it_end; ++it) {
        Variation* variation = mProgram->mVariation;
        it->mVariationIndex = it.getIndex() + 1;
        it->mVariation = variation;
        it->mDisplayList.setName(it->mVariation->mName.cstr());
    }
}

/**
 * Sets up the variation macros from resource variations.
 * @param rName program name
 * @param variations resource variations
 * @param pHeap heap to allocate from
 */
void ShaderProgram::initialize_(const sead::SafeString& rName,
                                ResArray<ResShaderVariation> variations, sead::Heap* pHeap)
{
    initializeVariation(rName, variations.getNum(), pHeap);

    for (auto it = variations.begin(), it_end = variations.end(); it != it_end; ++it) {
        ResShaderVariation variation(&(*it));
        const s32 index = it.getIndex();
        createVariationMacro(index, variation.getName(), variation.getID(),
                             variation.ref().mValueNum, pHeap);
        for (s32 i = 0; i < variation.ref().mValueNum; i++) {
            setVariationMacroValue(index, i, variation.getValue(i));
        }
    }

    createVariation(pHeap);
}

/**
 * Sets up the program from a source shader program resource.
 * @param program shader program resource
 * @param pHeap heap to allocate from
 */
void ShaderProgram::initialize(ResShaderProgram program, sead::Heap* pHeap)
{
    if (mVariation == nullptr) {
        initialize_(program.getName(), program.getResShaderVariationArray(), pHeap);
    }

    ResShaderUniformBlockArray block_array = reinterpret_cast<const ResShaderUniformBlockArray::DataType*>(
        reinterpret_cast<uintptr_t>(program.getResShaderVariationDefaultArray().ptr()) +
        program.getResShaderVariationDefaultArray().ref().mSize);
    mVariation->mDefaultVariationArray = program.getResShaderVariationDefaultArray().ptr();
    mVariation->mUniformBlockArray = block_array.ptr();

    if (block_array.isValid()) {
        for (auto it = block_array.begin(), it_end = block_array.end(); it != it_end; ++it) {
            ResShaderUniformBlock block(&(*it));
            const char* name = block.getName();

            if (ShaderCompileInfo::getRegitserUniformBlockName().isEqual(name)) {
                mVariation->mRegisterUniformBlockLocation = block.getLocation();
            }
        }
    }
}

/**
 * Sets up the program from a binary shader program resource.
 * @param program binary shader program resource
 * @param pHeap heap to allocate from
 */
void ShaderProgram::initialize(ResBinaryShaderProgram program, sead::Heap* pHeap)
{
    initialize_(program.getName(), program.getResShaderVariationArray(), pHeap);
}

/**
 * Allocates the attribute locations.
 * @param num number of attributes
 * @param pHeap heap to allocate from
 */
void ShaderProgram::createAttribute(s32 num, sead::Heap* pHeap)
{
    mAttributeLocation.tryAllocBuffer(num, pHeap);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mAttributeLocation.tryAllocBuffer(num, pHeap);
    }
}

/**
 * Sets the name of an attribute.
 * @param index attribute index
 * @param rName attribute name
 */
void ShaderProgram::setAttributeName(s32 index, const sead::SafeString& rName)
{
    mAttributeLocation[index].setName(rName);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mAttributeLocation[index].setName(rName);
    }
}

/**
 * Frees the attribute locations.
 */
void ShaderProgram::destroyAttribute()
{
    mAttributeLocation.freeBuffer();

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mAttributeLocation.freeBuffer();
    }
}

/**
 * Allocates the uniform locations.
 * @param num number of uniforms
 * @param pHeap heap to allocate from
 */
void ShaderProgram::createUniform(s32 num, sead::Heap* pHeap)
{
    mUniformLocation.tryAllocBuffer(num, pHeap);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mUniformLocation.tryAllocBuffer(num, pHeap);
    }
}

/**
 * Sets the name of a uniform.
 * @param index uniform index
 * @param rName uniform name
 */
void ShaderProgram::setUniformName(s32 index, const sead::SafeString& rName)
{
    mUniformLocation[index].setName(rName);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mUniformLocation[index].setName(rName);
    }
}

/**
 * Frees the uniform locations.
 */
void ShaderProgram::destroyUniform()
{
    mUniformLocation.freeBuffer();

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mUniformLocation.freeBuffer();
    }
}

/**
 * Allocates the sampler locations.
 * @param num number of samplers
 * @param pHeap heap to allocate from
 */
void ShaderProgram::createSamplerLocation(s32 num, sead::Heap* pHeap)
{
    mSamplerLocation.tryAllocBuffer(num, pHeap);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mSamplerLocation.tryAllocBuffer(num, pHeap);
    }
}

/**
 * Sets the name of a sampler.
 * @param index sampler index
 * @param rName sampler name
 */
void ShaderProgram::setSamplerLocationName(s32 index, const sead::SafeString& rName)
{
    mSamplerLocation[index].setName(rName);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mSamplerLocation[index].setName(rName);
    }
}

/**
 * Frees the sampler locations.
 */
void ShaderProgram::destroySamplerLocation()
{
    mSamplerLocation.freeBuffer();

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mSamplerLocation.freeBuffer();
    }
}

/**
 * Allocates the image locations.
 * @param num number of images
 * @param pHeap heap to allocate from
 */
void ShaderProgram::createImageLocation(s32 num, sead::Heap* pHeap)
{
    mImageLocation.tryAllocBuffer(num, pHeap);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mImageLocation.tryAllocBuffer(num, pHeap);
    }
}

/**
 * Sets the name of an image.
 * @param index image index
 * @param rName image name
 */
void ShaderProgram::setImageLocationName(s32 index, const sead::SafeString& rName)
{
    mImageLocation[index].setName(rName);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mImageLocation[index].setName(rName);
    }
}

/**
 * Frees the image locations.
 */
void ShaderProgram::destroyImageLocation()
{
    mImageLocation.freeBuffer();

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mImageLocation.freeBuffer();
    }
}

/**
 * Allocates the uniform block locations.
 * @param num number of uniform blocks
 * @param pHeap heap to allocate from
 */
void ShaderProgram::createUniformBlock(s32 num, sead::Heap* pHeap)
{
    mUniformBlockLocation.tryAllocBuffer(num, pHeap);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mUniformBlockLocation.tryAllocBuffer(num, pHeap);
    }
}

/**
 * Sets the name of a uniform block.
 * @param index uniform block index
 * @param rName uniform block name
 */
void ShaderProgram::setUniformBlockName(s32 index, const sead::SafeString& rName)
{
    mUniformBlockLocation[index].setName(rName);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mUniformBlockLocation[index].setName(rName);
    }
}

/**
 * Frees the uniform block locations.
 */
void ShaderProgram::destroyUniformBlock()
{
    mUniformBlockLocation.freeBuffer();

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mUniformBlockLocation.freeBuffer();
    }
}

/**
 * Allocates the shader storage block locations.
 * @param num number of shader storage blocks
 * @param pHeap heap to allocate from
 */
void ShaderProgram::createShaderStorageBlock(s32 num, sead::Heap* pHeap)
{
    mShaderStorageBlockLocation.tryAllocBuffer(num, pHeap);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mShaderStorageBlockLocation.tryAllocBuffer(num, pHeap);
    }
}

/**
 * Sets the name of a shader storage block.
 * @param index shader storage block index
 * @param rName shader storage block name
 */
void ShaderProgram::setShaderStorageBlockName(s32 index, const sead::SafeString& rName)
{
    mShaderStorageBlockLocation[index].setName(rName);

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mShaderStorageBlockLocation[index].setName(rName);
    }
}

/**
 * Frees the shader storage block locations.
 */
void ShaderProgram::destroyShaderStorageBlock()
{
    mShaderStorageBlockLocation.freeBuffer();

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mShaderStorageBlockLocation.freeBuffer();
    }
}

/**
 * Creates the location buffers of every variation from those of the base program.
 */
void ShaderProgram::setUpForVariation_() const
{
    const char* macros[32];
    const char* values[32];
    const ShaderProgram* base = mVariation->mVariationBuffer.mProgram;
    const s32 macro_num =
        mVariation->mVariationBuffer.getMacroAndValueArray(mVariationIndex, macros, values);

    for (s32 type = 0; type < cShaderType_Num; type++) {
        ShaderCompileInfo* info = base->getShader(ShaderType(type)).getCompileInfo();

        if (info != nullptr) {
            info->clearVariation();

            for (s32 i = 0; i < macro_num; i++) {
                info->pushBackVariation(macros[i], values[i]);
            }
        }

        const_cast<ShaderProgram*>(this)->getShader(ShaderType(type))->setCompileInfo(info);
    }
}

/**
 * Gets the macro names and values of a variation.
 * @param programIndex variation index
 * @param pMacros receives the macro names
 * @param pValues receives the macro values
 * @return the number of macros
 */
s32 ShaderProgram::VariationBuffer::getMacroAndValueArray(s32 programIndex, const char** pMacros,
                                                         const char** pValues) const
{
    for (const auto& macro : mMacros) {
        const s32 value = programIndex / macro.mStride;
        *pMacros++ = macro.mName.cstr();
        *pValues++ = macro.mValues[value].cstr();
        programIndex -= value * macro.mStride;
    }

    return mMacros.size();
}

/**
 * Gets the shader of a stage.
 * @param type shader stage
 * @return the shader
 */
const Shader& ShaderProgram::getShader(ShaderType type) const
{
    switch (type) {
    case cShaderType_Vertex:
        return mVertexShader;
    case cShaderType_Fragment:
        return mFragmentShader;
    case cShaderType_Geometry:
        return mGeometryShader;
    case cShaderType_Compute:
        return mComputeShader;
    default:
        return *static_cast<const Shader*>(nullptr);
    }
}

/**
 * Sets the shader on a draw context (not used on this platform).
 * @param pDrawContext draw context
 */
void ShaderProgram::setShaderGX2_(DrawContext* pDrawContext) const {}

/**
 * Sets up the program if a setup has been reserved.
 * @return 0
 */
s32 ShaderProgram::validate_() const
{
    if (mFlags & cFlag_ReserveSetUp) {
        sead::ScopedCriticalSectionLock lock(
            driver::NVNMgr::instance()->getCriticalSection());
        if (mFlags & cFlag_ReserveSetUp) {
            forceValidate_(false);
        }
    }

    return 0;
}

/**
 * Sets up the NVN program from the shader binaries.
 * @return 0
 */
s32 ShaderProgram::forceValidate_(bool) const
{
    const bool initialized = mFlags & cFlag_Initialized;
    setUpForVariation_();

    const Shader* shaders[cShaderType_Num] = {&mVertexShader, &mFragmentShader, &mGeometryShader,
                                              &mComputeShader};
    for (s32 type = 0; type < cShaderType_Num; type++) {
        const auto* info = static_cast<const ResShaderBinaryInfo*>(shaders[type]->getShaderBinary());

        if (info == nullptr) {
            continue;
        }

        const ResShaderBinaryVar16* block =
            findUniformBlock(info, ShaderCompileInfo::getRegitserUniformBlockName().cstr());
        if (block != nullptr) {
            mVariation->mRegisterUniformBlockLocation = block->_c;
        }
    }

    if (initialized) {
        nvnProgramFinalize(&mProgram);
        mFlags &= ~cFlag_Initialized;
    }

    nvnProgramInitialize(&mProgram, driver::NVNMgr::instance()->getNvnDevice());

    NVNshaderData data[cShaderType_Num];
    u16 stages = 0;
    s32 count = 0;

    for (s32 type = 0; type < cShaderType_Num; type++) {
        const auto* info = static_cast<const ResShaderBinaryInfo*>(shaders[type]->getShaderBinary());

        if (info == nullptr) {
            continue;
        }

        const NVNbufferAddress address = mBufferAddress + info->mDataOffset;
        data[count].data = address;
        data[count].control = info->mCode;
        stages |= cStageBits[type];
        count++;
    }

    if (nvnProgramSetShaders(&mProgram, count, data)) {
        mFlags |= cFlag_Initialized;
        mStageFlags = stages;
    }

    mFlags &= ~cFlag_ReserveSetUp;

    if (mFlags & cFlag_Initialized) {
        updateLocation();
    }

    if (mVariation->mListener != nullptr) {
        mVariation->mListener->getDelegate()->invoke(this);
    }

    return 0;
}

/**
 * Compiles the program offline (not supported on this platform).
 */
void ShaderProgram::compileOffline_() const {}

/**
 * Searches the locations of all symbols.
 */
void ShaderProgram::updateLocation() const
{
    updateUniformLocation();
    updateUniformBlockLocation();
    updateShaderStorageBlockLocation();
    updateAttributeLocation();
    updateSamplerLocation();
    updateImageLocation();
}

/**
 * Searches the locations of the uniforms.
 */
void ShaderProgram::updateUniformLocation() const
{
    for (auto& location : mUniformLocation) {
        location.search(*this);
    }
}

/**
 * Searches the locations of the uniform blocks.
 */
void ShaderProgram::updateUniformBlockLocation() const
{
    for (auto& location : mUniformBlockLocation) {
        location.search(*this);
    }
}

/**
 * Searches the locations of the shader storage blocks.
 */
void ShaderProgram::updateShaderStorageBlockLocation() const
{
    for (auto& location : mShaderStorageBlockLocation) {
        location.search(*this);
    }
}

/**
 * Searches the locations of the attributes.
 */
void ShaderProgram::updateAttributeLocation() const
{
    for (auto& location : mAttributeLocation) {
        location.search(*this);
    }
}

/**
 * Searches the locations of the samplers.
 */
void ShaderProgram::updateSamplerLocation() const
{
    for (auto& location : mSamplerLocation) {
        location.search(*this);
    }
}

/**
 * Searches the locations of the images.
 */
void ShaderProgram::updateImageLocation() const
{
    for (auto& location : mImageLocation) {
        location.search(*this);
    }
}

/**
 * Binds the program and the register uniform block.
 * @param pDrawContext draw context
 */
void ShaderProgram::activate(DrawContext* pDrawContext, bool) const
{
    validate_();

    if (!(mFlags & cFlag_Initialized)) {
        return;
    }

    if (pDrawContext->getShaderMode() != mVariation->mShaderMode || pDrawContext->get_fa()) {
        pDrawContext->changeShaderMode(ShaderMode(mVariation->mShaderMode), ShaderOptimizeType(0));
    }

    NVNcommandBuffer* command_buffer = pDrawContext->getNvnCommandBuffer();
    nvnCommandBufferBindProgram(command_buffer, &mProgram, NVN_SHADER_STAGE_ALL_GRAPHICS_BITS | NVN_SHADER_STAGE_COMPUTE_BIT);

    if (mVariation->mRegisterUniformBlockLocation > 0) {
        for (s32 type = 0; type < cShaderType_Num; type++) {
            nvnCommandBufferBindUniformBuffer(
                command_buffer, driver::NVNMgr::getNVNshaderStage(ShaderType(type)), 0,
                getDynamicUniformBlockAddress(), getDynamicUniformBlockSize());
        }
    }
}

/**
 * Sets up the programs of all variations.
 * @param force whether to set up programs that have no reserved setup
 * @return 0
 */
s32 ShaderProgram::setUpAllVariation(bool force)
{
    ShaderProgram* base = mVariation->mVariationBuffer.mProgram;

    if (force) {
        base->forceValidate_(false);
    } else {
        base->validate_();
    }

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        if (force) {
            program.forceValidate_(false);
        } else {
            program.validate_();
        }
    }

    return 0;
}

/**
 * Reserves a setup of the programs of all variations.
 */
void ShaderProgram::reserveSetUpAllVariation()
{
    mVariation->mVariationBuffer.mProgram->mFlags |= cFlag_ReserveSetUp;

    for (auto& program : mVariation->mVariationBuffer.mPrograms) {
        program.mFlags |= cFlag_ReserveSetUp;
    }
}

/**
 * Gets the shader of a stage.
 * @param type shader stage
 * @return the shader, or nullptr for an invalid stage
 */
Shader* ShaderProgram::getShader(ShaderType type)
{
    switch (type) {
    case cShaderType_Vertex:
        return &mVertexShader;
    case cShaderType_Fragment:
        return &mFragmentShader;
    case cShaderType_Geometry:
        return &mGeometryShader;
    case cShaderType_Compute:
        return &mComputeShader;
    default:
        return nullptr;
    }
}

/**
 * Dispatches compute work.
 * @param pDrawContext draw context
 * @param x number of work groups in X
 * @param y number of work groups in Y
 * @param z number of work groups in Z
 */
void ShaderProgram::dispatchCompute(DrawContext* pDrawContext, s32 x, s32 y, s32 z) const
{
    nvnCommandBufferDispatchCompute(pDrawContext->getNvnCommandBuffer(), x, y, z);
}

/**
 * Dispatches this compute program with work group counts read from a shader storage block.
 * @param pDrawContext Draw context whose command buffer receives the dispatch.
 * @param rBlock Shader storage block holding the indirect dispatch arguments.
 * @param blockIndex Index of the block within the current buffer.
 * @param offset Byte offset of the arguments within the block.
 */
void ShaderProgram::dispatchComputeIndirect(DrawContext* pDrawContext,
                                            const ShaderStorageBlock& rBlock, s32 blockIndex,
                                            s64 offset) const
{
    NVNbufferAddress address = nvnBufferGetAddress(rBlock.getNvnBuffer()) +
                               rBlock.getCurrentBlockOffset(blockIndex) + offset;
    nvnCommandBufferDispatchComputeIndirect(pDrawContext->getNvnCommandBuffer(), address);
}

/**
 * Calculates the CRC32 hash of some data.
 * @param pData data
 * @param size size of the data
 * @return the hash
 */
u32 ShaderProgram::calcHash(const void* pData, u32 size)
{
    return sead::HashCRC32::calcHash(pData, size);
}

/**
 * Gets information about a uniform (not supported on this platform).
 * @param pSymbol receives the symbol
 * @param index uniform index
 * @return 0 if the program is set up, -1 otherwise
 */
s32 ShaderProgram::getUniformSymbol(UniformSymbol* pSymbol, s32 index, bool) const
{
    validate_();
    return (mFlags & cFlag_Initialized) - 1;
}

/**
 * Gets information about a uniform block (not supported on this platform).
 * @param pSymbol receives the symbol
 * @param index uniform block index
 * @return 0 if the program is set up, -1 otherwise
 */
s32 ShaderProgram::getUniformBlockSymbol(UniformBlockSymbol* pSymbol, s32 index) const
{
    validate_();
    return (mFlags & cFlag_Initialized) - 1;
}

/**
 * Gets information about a shader storage block (not supported on this platform).
 * @param pSymbol receives the symbol
 * @param index shader storage block index
 * @return 0 if the program is set up, -1 otherwise
 */
s32 ShaderProgram::getShaderStorageBlockSymbol(ShaderStorageBlockSymbol* pSymbol, s32 index) const
{
    validate_();
    return (mFlags & cFlag_Initialized) - 1;
}

/**
 * Collects the samplers used by all stages.
 * @param pSymbol receives the samplers
 * @param maxNum maximum number of samplers to collect
 * @return the number of samplers, or -1 if the program is not set up
 */
s32 ShaderProgram::getSamplerSymbol(SamplerSymbol* pSymbol, s32 maxNum) const
{
    validate_();

    if (!(mFlags & cFlag_Initialized)) {
        return -1;
    }

    s32 num = 0;

    for (s32 type = 0; type < cShaderType_Num; type++) {
        const auto* info =
            static_cast<const ResShaderBinaryInfo*>(getShader(ShaderType(type)).getShaderBinary());
        if (info == nullptr) {
            continue;
        }

        for (u32 i = 0; i < u32(info->mVar4Num); i++) {
            const char* name = info->mVar4[i].mName;
            const s32 location = info->mVar4[i]._8;

            s32 j = 0;

            for (; j < num; j++) {
                if (pSymbol[j].mName.isEqual(name)) {
                    break;
                }
            }

            if (j < num) {
                if (location != -1) {
                    pSymbol[j].mLocation.setLocation(ShaderType(type), location);
                }
            } else if (num < maxNum) {
                pSymbol[num].mName.copy(name);
                pSymbol[num].mLocation.setLocation(ShaderType(type), location);
                num++;
            }
        }
    }

    return num;
}

/**
 * Gets information about an attribute (not supported on this platform).
 * @param pSymbol receives the symbol
 * @param index attribute index
 * @return 0 if the program is set up, -1 otherwise
 */
s32 ShaderProgram::getAttributeSymbol(AttributeSymbol* pSymbol, s32 index) const
{
    validate_();
    return (mFlags & cFlag_Initialized) - 1;
}

/**
 * Gets information about a buffer variable (not supported on this platform).
 * @param pSymbol receives the symbol
 * @param index buffer variable index
 * @return 0 if the program is set up, -1 otherwise
 */
s32 ShaderProgram::getBufferVariableSymbol(BufferVariableSymbol* pSymbol, s32 index) const
{
    validate_();
    return (mFlags & cFlag_Initialized) - 1;
}

/**
 * Builds the compile source of a stage for the current variation.
 * @param type shader stage
 * @param pSource receives the source
 * @param target compile target
 * @return true if the stage has compile information
 */
bool ShaderProgram::calcCompileSource(ShaderType type, sead::BufferedSafeString* pSource,
                                      ShaderCompileInfo::Target target) const
{
    setUpForVariation_();

    const ShaderCompileInfo* info = getShader(type).getCompileInfo();

    if (info != nullptr) {
        info->calcCompileSource(type, pSource, target, true);
        return true;
    }

    pSource->copy(sead::SafeString::cEmptyString);
    return false;
}

/**
 * Builds the compile source of a stage without variation macros.
 * @param type shader stage
 * @param pSource receives the source
 * @param target compile target
 * @return true if the stage has compile information
 */
bool ShaderProgram::calcCompileSourceNoVariation(ShaderType type,
                                                 sead::BufferedSafeString* pSource,
                                                 ShaderCompileInfo::Target target) const
{
    const ShaderProgram* base = mVariation->mVariationBuffer.mProgram;

    if (ShaderCompileInfo* info = base->mVertexShader.getCompileInfo()) {
        info->clearVariation();
    }

    if (ShaderCompileInfo* info = base->mFragmentShader.getCompileInfo()) {
        info->clearVariation();
    }

    if (ShaderCompileInfo* info = base->mGeometryShader.getCompileInfo()) {
        info->clearVariation();
    }

    if (ShaderCompileInfo* info = base->mComputeShader.getCompileInfo()) {
        info->clearVariation();
    }

    const ShaderCompileInfo* info = getShader(type).getCompileInfo();

    if (info != nullptr) {
        info->calcCompileSource(type, pSource, target, false);
        return true;
    }

    pSource->copy(sead::SafeString::cEmptyString);
    return false;
}

/**
 * Prints information about the program (no-op in release builds).
 */
void ShaderProgram::dump() const {}

/**
 * Checks whether the program has a shader for a stage.
 * @param type shader stage
 * @return true if the stage is present
 */
bool ShaderProgram::hasStage(ShaderType type) const
{
    validate_();
    return (mStageFlags & cStageBits[type]) != 0;
}

/**
 * Constructs an empty variation buffer.
 */
ShaderProgram::VariationBuffer::VariationBuffer() : mProgram(nullptr) {}

/**
 * Destroys the variation buffer.
 */
ShaderProgram::VariationBuffer::~VariationBuffer()
{
    mPrograms.freeBuffer();

    for (auto& macro : mMacros) {
        macro.mValues.freeBuffer();
    }

    mMacros.freeBuffer();
}

/**
 * Searches the index of the variation that uses the given macro values.
 * @param macroNum number of macros to search
 * @param pMacros macro names
 * @param pValues macro values
 * @param baseIndex variation whose values are used for macros that are not given, or -1
 * @return the variation index
 */
s32 ShaderProgram::VariationBuffer::searchShaderProgramIndex(s32 macroNum,
                                                             const char* const* pMacros,
                                                             const char* const* pValues,
                                                             s32 baseIndex) const
{
    s32 value_index[34];

    if (baseIndex == -1) {
        std::memset(value_index, 0, mMacros.size() * sizeof(s32));
    } else {
        s32* value = value_index;

        for (const auto& macro : mMacros) {
            *value = baseIndex / macro.mStride;
            baseIndex -= *value * macro.mStride;
            value++;
        }
    }

    if (mMacros.size() == 0) {
        return 0;
    }

    for (auto it = mMacros.begin(), it_end = mMacros.end(); it != it_end; ++it) {
        for (s32 i = 0; i < macroNum; i++) {
            if (!it->mName.isEqual(pMacros[i])) {
                continue;
            }

            for (auto value = it->mValues.begin(), value_end = it->mValues.end();
                 value != value_end; ++value) {
                if (value->isEqual(pValues[i])) {
                    value_index[it.getIndex()] = value.getIndex();
                    break;
                }
            }

            break;
        }
    }

    s32 index = 0;

    for (s32 i = 0; i < mMacros.size(); i++) {
        index += value_index[i] * mMacros(i).mStride;
    }

    return index;
}

/**
 * Gets the value that a macro has in a variation.
 * @param programIndex variation index
 * @param pName macro name
 * @return the value, or an empty string if the macro does not exist
 */
const char* ShaderProgram::VariationBuffer::searchMacroValue(s32 programIndex,
                                                             const char* pName) const
{
    for (const auto& macro : mMacros) {
        const s32 value = programIndex / macro.mStride;
        programIndex -= value * macro.mStride;

        if (macro.mName.isEqual(pName)) {
            return macro.mValues[value].cstr();
        }
    }

    return sead::SafeString::cEmptyString.cstr();
}

/**
 * Searches a macro by ID.
 * @param rID macro ID
 * @return the macro name, or an empty string if it does not exist
 */
const sead::SafeString&
ShaderProgram::VariationBuffer::searchMacroName(const sead::SafeString& rID) const
{
    for (const auto& macro : mMacros) {
        if (rID.isEqual(macro.mID)) {
            return macro.mName;
        }
    }

    return sead::SafeString::cEmptyString;
}

/**
 * Searches a macro by name.
 * @param rName macro name
 * @return the macro ID, or an empty string if it does not exist
 */
const sead::SafeString&
ShaderProgram::VariationBuffer::searchMacroID(const sead::SafeString& rName) const
{
    for (const auto& macro : mMacros) {
        if (rName.isEqual(macro.mName)) {
            return macro.mID;
        }
    }

    return sead::SafeString::cEmptyString;
}

/**
 * Searches the index of a macro by name.
 * @param rName macro name
 * @return the macro index, or -1 if it does not exist
 */
s32 ShaderProgram::VariationBuffer::searchMacroIndex(const sead::SafeString& rName) const
{
    for (auto it = mMacros.begin(), it_end = mMacros.end(); it != it_end; ++it) {
        if (rName.isEqual(it->mName)) {
            return it.getIndex();
        }
    }

    return -1;
}

}  // namespace agl
