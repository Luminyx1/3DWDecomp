#include <nn/gfx/detail/gfx_Shader-api.nvn.8.h>

#include <nn/gfx/detail/gfx_Device-api.nvn.8.h>
#include <nn/gfx/detail/gfx_NvnHelper.h>
#include <nn/gfx/gfx_ResShaderData-api.nvn.h>
#include <nn/gfx/gfx_ResShaderData.h>
#include <nn/gfx/gfx_ShaderInfo.h>
#include <nn/os/os_Mutex.h>
#include <nn/util/util_BitUtil.h>
#include <nn/util/util_BytePtr.h>
#include <nn/util/util_ResDic.h>
#include <nvn/nvn_FuncPtrInline.h>
#include <nvnTool/nvnTool_GlslcInterface.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>

/// Shader stage data with an attached debug data hash (nvnProgramSetShadersExt).
typedef struct {
    NVNbufferAddress data;
    const void* control;
    const void* debugDataHash;
} NVNshaderDataExt;

typedef NVNboolean (*PFNNVNPROGRAMSETSHADERSEXTPROC)(NVNprogram*, int, const NVNshaderDataExt*);

// Entry point loaded by nvnLoadCProcs that nvn_FuncPtrBase.h does not declare.
extern "C" {
extern PFNNVNPROGRAMSETSHADERSEXTPROC pfnc_nvnProgramSetShadersExt;
}

/**
 * Sets the shaders of a program, including their debug data hashes.
 *
 * @param pProgram Program to set the shaders of.
 * @param count Number of entries in pStageData.
 * @param pStageData Per-stage shader data.
 * @return Whether the shaders were accepted.
 */
static inline NVNboolean nvnProgramSetShadersExt(NVNprogram* pProgram, int count,
                                                 const NVNshaderDataExt* pStageData) {
    return pfnc_nvnProgramSetShadersExt(pProgram, count, pStageData);
}

namespace nn::gfx::detail {

typedef ShaderImpl<ApiVariationNvn8> ShaderImplNvn8;
typedef DeviceImpl<ApiVariationNvn8> DeviceImplNvn8;

namespace {

/**
 * Returns the NVN device of a device.
 *
 * @param pDevice Device.
 * @return NVN device.
 */
NVNdevice* GetNvnDevice(DeviceImplNvn8* pDevice) {
    return static_cast<NVNdevice*>(pDevice->ToData()->pNvnDevice.ptr);
}

/**
 * Returns the NVN program of a shader.
 *
 * @param pShader Shader.
 * @return NVN program.
 */
NVNprogram* GetNvnProgram(ShaderImplNvn8* pShader) {
    return static_cast<NVNprogram*>(pShader->ToData()->pNvnProgram.ptr);
}

/**
 * Returns the binding slot of a reflected shader interface for a stage.
 *
 * @param rPiq Reflected interface.
 * @param stage NVN shader stage.
 * @return Binding slot.
 */
template <typename TInterface>
int GetShaderSlot(const TInterface& rPiq, NVNshaderStage stage) {
    return rPiq.bindings[stage];
}

/**
 * Returns the location of a reflected program input.
 *
 * @param rPiq Reflected program input.
 * @param stage NVN shader stage (unused).
 * @return Input location.
 */
template <>
int GetShaderSlot<GLSLCProgramInputInfo>(const GLSLCProgramInputInfo& rPiq,
                                         NVNshaderStage stage) {
    return rPiq.location;
}

/**
 * Returns the location of a reflected program output.
 *
 * @param rPiq Reflected program output.
 * @param stage NVN shader stage (unused).
 * @return Output location.
 */
template <>
int GetShaderSlot<GLSLCProgramOutputInfo>(const GLSLCProgramOutputInfo& rPiq,
                                          NVNshaderStage stage) {
    return rPiq.location;
}

/**
 * Returns whether a reflected interface lives inside a uniform block.
 *
 * @param rPiq Reflected interface.
 * @return Always false for interfaces other than uniforms.
 */
template <typename TInterface>
bool IsInUniformBlock(const TInterface& rPiq) {
    return false;
}

/**
 * Returns whether a reflected uniform lives inside a uniform block.
 *
 * @param rPiq Reflected uniform.
 * @return Whether the uniform is a member of a uniform block.
 */
template <>
bool IsInUniformBlock<GLSLCuniformInfo>(const GLSLCuniformInfo& rPiq) {
    return rPiq.isInUBO != 0;
}

/// Shader program compiled at runtime from source by GLSLC.
class OnlineCompiledShader {
public:
    /**
     * Constructs an empty compiled shader.
     */
    OnlineCompiledShader() {
        m_pMemory = nullptr;
        m_pOutput = nullptr;
    }

    /**
     * Destroys the compiled shader, releasing its memory.
     */
    ~OnlineCompiledShader() { Finalize(); }

    bool Initialize(DeviceImplNvn8* pDevice, const GLSLCoutput* pOutput);
    void Finalize();

    /**
     * Sets the compiled stages as the shaders of an NVN program.
     *
     * @param pNvnProgram Program to set the shaders of.
     * @return Whether the shaders were accepted.
     */
    NVNboolean SetShader(NVNprogram* pNvnProgram) const {
        return nvnProgramSetShaders(pNvnProgram, m_StageCount, m_NvnShaderData);
    }

    int GetInterfaceSlot(ShaderStage stage, ShaderInterfaceType interfaceType,
                         const char* pName) const;

    /**
     * Returns the scratch memory needed per warp.
     *
     * @return Scratch memory size in bytes.
     */
    size_t GetScratchMemorySizePerWarp() const { return m_ScratchMemoryPerWarp; }

    void GetWorkGroupSize(int* pOutWorkGroupSizeX, int* pOutWorkGroupSizeY,
                          int* pOutWorkGroupSizeZ) const;

private:
    /**
     * Searches a reflection table for an interface referenced by a stage.
     *
     * @param pFirst First entry of the table.
     * @param count Number of entries in the table.
     * @param pStringPool Reflection string pool.
     * @param pName Interface name.
     * @param stage NVN shader stage.
     * @return Slot of the interface, or -1 if it was not found.
     */
    template <typename TInterface>
    int SearchInterfaceSlot(const TInterface* pFirst, int count, const void* pStringPool,
                            const char* pName, NVNshaderStage stage) const {
        int stageBits = 1 << stage;
        for (int idx = 0; idx < count; ++idx) {
            const TInterface& rPiq = pFirst[idx];
            const GLSLCpiqName& rNameInfo = rPiq.nameInfo;

            if (!IsInUniformBlock(rPiq) && (rPiq.stagesReferencedIn & stageBits) &&
                strncmp(pName,
                        nn::util::ConstBytePtr(pStringPool, rNameInfo.nameOffset).Get<char>(),
                        rNameInfo.nameLength) == 0) {
                int result = GetShaderSlot(rPiq, stage);
                if (result >= 0) {
                    return result;
                }
            }
        }

        return -1;
    }

    NVNmemoryPool m_NvnMemoryPool;
    void* m_pMemoryBase;
    void* m_pMemory;
    GLSLCoutput* m_pOutput;
    NVNshaderData m_NvnShaderData[6];
    int m_StageCount;
    size_t m_ScratchMemoryPerWarp;
    const GLSLCprogramReflectionHeader* m_pReflectionHeader;
};

/**
 * Copies the GLSLC output and uploads its GPU code into a shader code memory pool.
 *
 * @param pDevice Device to create the memory pool on.
 * @param pOutput GLSLC compilation output.
 * @return Whether the shader was set up successfully.
 */
bool OnlineCompiledShader::Initialize(DeviceImplNvn8* pDevice, const GLSLCoutput* pOutput) {
    m_pOutput = static_cast<GLSLCoutput*>(malloc(pOutput->size));
    if (m_pOutput == nullptr) {
        return false;
    }

    memcpy(m_pOutput, pOutput, pOutput->size);

    size_t stageDataSize[6] = {};
    const void* pStageData[6] = {};
    m_StageCount = 0;
    size_t memoryPoolSize = 0;
    m_ScratchMemoryPerWarp = 0;

    for (int idxSection = 0; idxSection < static_cast<int>(m_pOutput->numSections);
         ++idxSection) {
        GLSLCsectionTypeEnum type = m_pOutput->headers[idxSection].genericHeader.common.type;
        if (type == GLSLC_SECTION_TYPE_GPU_CODE) {
            const GLSLCgpuCodeHeader& rGpuCodeHeader = m_pOutput->headers[idxSection].gpuCodeHeader;

            memoryPoolSize += rGpuCodeHeader.dataSize;
            memoryPoolSize =
                nn::util::align_up(memoryPoolSize, ShaderImplNvn8::GetBinaryCodeAlignment(pDevice));

            const void* pData =
                nn::util::ConstBytePtr(m_pOutput, rGpuCodeHeader.common.dataOffset).Get();

            stageDataSize[m_StageCount] = rGpuCodeHeader.dataSize;
            pStageData[m_StageCount] =
                nn::util::ConstBytePtr(pData, rGpuCodeHeader.dataOffset).Get();
            m_NvnShaderData[m_StageCount].control =
                nn::util::ConstBytePtr(pData, rGpuCodeHeader.controlOffset).Get();

            ++m_StageCount;

            m_ScratchMemoryPerWarp =
                std::max(m_ScratchMemoryPerWarp,
                         static_cast<size_t>(rGpuCodeHeader.scratchMemBytesPerWarp));
        } else if (type == GLSLC_SECTION_TYPE_REFLECTION) {
            m_pReflectionHeader = &m_pOutput->headers[idxSection].programReflectionHeader;
        }
    }

    // Extra padding after the last stage, since the GPU may prefetch past the end of the code.
    memoryPoolSize = nn::util::align_up(memoryPoolSize + 0x400, 0x1000);
    m_pMemoryBase = malloc(memoryPoolSize + 0x1000);
    if (m_pMemoryBase == nullptr) {
        return false;
    }

    m_pMemory = nn::util::BytePtr(m_pMemoryBase).AlignUp(0x1000).Get();

    nn::util::BytePtr pDst(m_pMemory);
    for (int idxStage = 0; idxStage < m_StageCount; ++idxStage) {
        memcpy(pDst.Get(), pStageData[idxStage], stageDataSize[idxStage]);
        pDst.Advance(stageDataSize[idxStage])
            .AlignUp(ShaderImplNvn8::GetBinaryCodeAlignment(pDevice));
    }

    NVNmemoryPoolBuilder memoryPoolBuilder;
    nvnMemoryPoolBuilderSetDevice(&memoryPoolBuilder, GetNvnDevice(pDevice));
    nvnMemoryPoolBuilderSetDefaults(&memoryPoolBuilder);
    nvnMemoryPoolBuilderSetFlags(&memoryPoolBuilder, NVN_MEMORY_POOL_FLAGS_SHADER_CODE |
                                                         NVN_MEMORY_POOL_FLAGS_GPU_CACHED |
                                                         NVN_MEMORY_POOL_FLAGS_CPU_NO_ACCESS);
    nvnMemoryPoolBuilderSetStorage(&memoryPoolBuilder, m_pMemory, memoryPoolSize);
    nvnMemoryPoolInitialize(&m_NvnMemoryPool, &memoryPoolBuilder);

    NVNbufferAddress address = nvnMemoryPoolGetBufferAddress(&m_NvnMemoryPool);

    ptrdiff_t offset = 0;
    for (int idxStage = 0; idxStage < m_StageCount; ++idxStage) {
        m_NvnShaderData[idxStage].data = offset + address;
        offset += stageDataSize[idxStage];
        offset = nn::util::align_up(offset, ShaderImplNvn8::GetBinaryCodeAlignment(pDevice));
    }

    return true;
}

/**
 * Releases the memory pool and the copied GLSLC output.
 */
void OnlineCompiledShader::Finalize() {
    if (m_pMemoryBase != nullptr) {
        nvnMemoryPoolFinalize(&m_NvnMemoryPool);
        free(m_pMemoryBase);
        m_pMemoryBase = nullptr;
        m_pMemory = nullptr;
    }

    if (m_pOutput != nullptr) {
        free(m_pOutput);
        m_pOutput = nullptr;
    }

    m_pReflectionHeader = nullptr;
}

/**
 * Looks up the slot of a shader interface in the GLSLC reflection data.
 *
 * @param stage Shader stage.
 * @param interfaceType Kind of interface.
 * @param pName Interface name.
 * @return Slot of the interface, or -1 if it was not found.
 */
int OnlineCompiledShader::GetInterfaceSlot(ShaderStage stage, ShaderInterfaceType interfaceType,
                                           const char* pName) const {
    const void* pReflectionData =
        nn::util::ConstBytePtr(m_pOutput, m_pReflectionHeader->common.dataOffset).Get();
    const void* pStringPool =
        nn::util::ConstBytePtr(pReflectionData, m_pReflectionHeader->stringPoolOffset).Get();
    NVNshaderStage nvnStage = Nvn::GetShaderStage(stage);
    int ret = -1;

    switch (interfaceType) {
    case ShaderInterfaceType_Input:
        ret = SearchInterfaceSlot(
            nn::util::ConstBytePtr(pReflectionData, m_pReflectionHeader->programInputsOffset)
                .Get<GLSLCProgramInputInfo>(),
            m_pReflectionHeader->numProgramInputs, pStringPool, pName, nvnStage);
        break;

    case ShaderInterfaceType_Output:
        ret = SearchInterfaceSlot(
            nn::util::ConstBytePtr(pReflectionData, m_pReflectionHeader->programOutputsOffset)
                .Get<GLSLCProgramOutputInfo>(),
            m_pReflectionHeader->numProgramOutputs, pStringPool, pName, nvnStage);
        break;

    case ShaderInterfaceType_ConstantBuffer:
        ret = SearchInterfaceSlot(
            nn::util::ConstBytePtr(pReflectionData, m_pReflectionHeader->uniformBlockOffset)
                .Get<GLSLCuniformBlockInfo>(),
            m_pReflectionHeader->numUniformBlocks, pStringPool, pName, nvnStage);
        break;

    case ShaderInterfaceType_UnorderedAccessBuffer:
        ret = SearchInterfaceSlot(
            nn::util::ConstBytePtr(pReflectionData, m_pReflectionHeader->ssboOffset)
                .Get<GLSLCssboInfo>(),
            m_pReflectionHeader->numSsbo, pStringPool, pName, nvnStage);
        break;

    case ShaderInterfaceType_Sampler:
        ret = SearchInterfaceSlot(
            nn::util::ConstBytePtr(pReflectionData, m_pReflectionHeader->uniformOffset)
                .Get<GLSLCuniformInfo>(),
            m_pReflectionHeader->numUniforms, pStringPool, pName, nvnStage);
        break;

    case ShaderInterfaceType_Image:
        ret = SearchInterfaceSlot(
            nn::util::ConstBytePtr(pReflectionData, m_pReflectionHeader->uniformOffset)
                .Get<GLSLCuniformInfo>(),
            m_pReflectionHeader->numUniforms, pStringPool, pName, nvnStage);
        break;

    default:
        break;
    }

    return ret;
}

/**
 * Retrieves the compute work group size from the GLSLC reflection data.
 *
 * @param pOutWorkGroupSizeX Receives the X size.
 * @param pOutWorkGroupSizeY Receives the Y size.
 * @param pOutWorkGroupSizeZ Receives the Z size.
 */
void OnlineCompiledShader::GetWorkGroupSize(int* pOutWorkGroupSizeX, int* pOutWorkGroupSizeY,
                                            int* pOutWorkGroupSizeZ) const {
    const void* pReflectionData =
        nn::util::ConstBytePtr(m_pOutput, m_pReflectionHeader->common.dataOffset).Get();
    const GLSLCperStageShaderInfo* pPerStageShaderInfo =
        nn::util::ConstBytePtr(pReflectionData, m_pReflectionHeader->shaderInfoOffset)
            .Get<GLSLCperStageShaderInfo>();
    const GLSLCshaderInfoCompute& rShaderInfoCompute = pPerStageShaderInfo->shaderInfo[5].compute;

    *pOutWorkGroupSizeX = rShaderInfoCompute.workGroupSize[0];
    *pOutWorkGroupSizeY = rShaderInfoCompute.workGroupSize[1];
    *pOutWorkGroupSizeZ = rShaderInfoCompute.workGroupSize[2];
}

/// Finalizes a GLSLC compile object when it goes out of scope.
class CompileObjectFinalizer {
public:
    /**
     * Takes ownership of an initialized compile object.
     *
     * @param pCompileObject Compile object to finalize.
     */
    explicit CompileObjectFinalizer(GLSLCcompileObject* pCompileObject) {
        m_pCompileObject = pCompileObject;
    }

    /**
     * Finalizes the compile object.
     */
    ~CompileObjectFinalizer() { GlslcDll::GetInstance().GlslcFinalize(m_pCompileObject); }

private:
    GLSLCcompileObject* m_pCompileObject;
};

/**
 * Compiles GLSL source shaders with GLSLC and creates the online compiled shader.
 *
 * @param pThis Shader being initialized.
 * @param rInfo Shader description.
 * @param pDevice Device the shader is created on.
 * @return Initialization result.
 */
ShaderInitializeResult NvnGlslcCompile(ShaderImplNvn8* pThis, const ShaderInfo& rInfo,
                                       DeviceImplNvn8* pDevice) {
    GLSLCcompileObject compileObject;
    if (GlslcDll::GetInstance().GlslcInitialize(&compileObject) == 0 ||
        compileObject.initStatus != GLSLC_INIT_SUCCESS) {
        return ShaderInitializeResult_SetupFailed;
    }

    CompileObjectFinalizer compileObjectFinalizer(&compileObject);
    GLSLCoptions& rOptions = compileObject.options;
    rOptions.optionFlags.glslSeparable = rInfo.IsSeparationEnabled();
    rOptions.optionFlags.outputAssembly = 0;
    rOptions.optionFlags.outputGpuBinaries = 1;
    rOptions.optionFlags.outputPerfStats = 0;
    rOptions.optionFlags.outputShaderReflection = 1;
    rOptions.optionFlags.language = GLSLC_LANGUAGE_GLSL;
    rOptions.optionFlags.outputDebugInfo = GLSLC_DEBUG_LEVEL_G0;
    rOptions.optionFlags.spillControl = NO_SPILL;
    rOptions.optionFlags.outputThinGpuBinaries = IsThinBinaryAvailable();
    rOptions.optionFlags.tessellationAndPassthroughGS = 0;
    rOptions.includeInfo.numPaths = 0;
    rOptions.includeInfo.paths = nullptr;
    rOptions.xfbVaryingInfo.numVaryings = 0;
    rOptions.xfbVaryingInfo.varyings = nullptr;
    rOptions.forceIncludeStdHeader = nullptr;

    std::string sources[6];
    const char* ppSources[6];
    NVNshaderStage stages[6];

    compileObject.input.sources = ppSources;
    compileObject.input.stages = stages;
    compileObject.input.count = 0;

    for (int idxStage = 0; idxStage < 6; ++idxStage) {
        ShaderStage stage = static_cast<ShaderStage>(idxStage);
        const ShaderCode* pShaderCode =
            static_cast<const ShaderCode*>(rInfo.GetShaderCodePtr(stage));
        if (pShaderCode != nullptr) {
            sources[compileObject.input.count].assign(static_cast<const char*>(pShaderCode->pCode),
                                                      pShaderCode->codeSize);
            ppSources[compileObject.input.count] = sources[compileObject.input.count].c_str();
            stages[compileObject.input.count] = Nvn::GetShaderStage(stage);
            ++compileObject.input.count;
        }
    }

    uint8_t compileResult = GlslcDll::GetInstance().GlslcCompile(&compileObject);
    const GLSLCcompilationStatus& rCompilationStatus =
        *compileObject.lastCompiledResults->compilationStatus;

    if (rCompilationStatus.allocError) {
        return ShaderInitializeResult_SetupFailed;
    }

    if (compileResult == 0 || !rCompilationStatus.success) {
        return ShaderInitializeResult_SetupFailed;
    }

    pThis->ToData()->pOnlineCompiledShader = malloc(sizeof(OnlineCompiledShader));
    OnlineCompiledShader* pOnlineCompiledShader =
        new (pThis->ToData()->pOnlineCompiledShader.ptr) OnlineCompiledShader();

    if (!pOnlineCompiledShader->Initialize(pDevice,
                                           compileObject.lastCompiledResults->glslcOutput)) {
        return ShaderInitializeResult_SetupFailed;
    }

    return ShaderInitializeResult_Success;
}

/// Locks a mutex for the lifetime of the object.
class MutexLocker {
public:
    /**
     * Locks the mutex.
     *
     * @param pMutex Mutex to lock.
     */
    explicit MutexLocker(os::Mutex* pMutex) {
        m_pMutex = pMutex;
        m_pMutex->Lock();
    }

    /**
     * Unlocks the mutex.
     */
    ~MutexLocker() { m_pMutex->Unlock(); }

private:
    os::Mutex* m_pMutex;
};

/**
 * Initializes a shader from GLSL source code, compiling it at runtime.
 *
 * @param pThis Shader being initialized.
 * @param rInfo Shader description.
 * @param pDevice Device the shader is created on.
 * @return Initialization result.
 */
ShaderInitializeResult InitializeSourceShader(ShaderImplNvn8* pThis, const ShaderInfo& rInfo,
                                              DeviceImplNvn8* pDevice) {
    static os::Mutex s_Mutex(false);
    MutexLocker locker(&s_Mutex);

    if (!GlslcDll::GetInstance().IsInitialized()) {
        GlslcDll::GetInstance().Initialize();
    }

    GLSLCversion version = GlslcDll::GetInstance().GlslcGetVersion();
    int majorVersionMinimum;
    int majorVersionMaximum;
    int minorVersionMinimum;
    int minorVersionMaximum;
    nvnDeviceGetInteger(GetNvnDevice(pDevice),
                        NVN_DEVICE_INFO_GLSLC_MIN_SUPPORTED_GPU_CODE_MAJOR_VERSION,
                        &majorVersionMinimum);
    nvnDeviceGetInteger(GetNvnDevice(pDevice),
                        NVN_DEVICE_INFO_GLSLC_MAX_SUPPORTED_GPU_CODE_MAJOR_VERSION,
                        &majorVersionMaximum);
    nvnDeviceGetInteger(GetNvnDevice(pDevice),
                        NVN_DEVICE_INFO_GLSLC_MIN_SUPPORTED_GPU_CODE_MINOR_VERSION,
                        &minorVersionMinimum);
    nvnDeviceGetInteger(GetNvnDevice(pDevice),
                        NVN_DEVICE_INFO_GLSLC_MAX_SUPPORTED_GPU_CODE_MINOR_VERSION,
                        &minorVersionMaximum);

    if (static_cast<int>(version.gpuCodeVersionMajor) < majorVersionMinimum ||
        static_cast<int>(version.gpuCodeVersionMajor) > majorVersionMaximum ||
        static_cast<int>(version.gpuCodeVersionMinor) < minorVersionMinimum ||
        static_cast<int>(version.gpuCodeVersionMinor) > minorVersionMaximum) {
        return ShaderInitializeResult_SetupFailed;
    }

    ShaderInitializeResult initializeResult = NvnGlslcCompile(pThis, rInfo, pDevice);
    if (initializeResult != ShaderInitializeResult_Success) {
        return initializeResult;
    }

    OnlineCompiledShader* pOnlineCompiledShader =
        static_cast<OnlineCompiledShader*>(pThis->ToData()->pOnlineCompiledShader.ptr);
    if (pOnlineCompiledShader->SetShader(GetNvnProgram(pThis)) != true) {
        return ShaderInitializeResult_SetupFailed;
    }

    return ShaderInitializeResult_Success;
}

/**
 * Rebuilds an NVN control section from its decomposed parts.
 *
 * @param pDestination Buffer receiving the control section.
 * @param pDecomposedControlSection Decomposed control section.
 */
void ReassembleControlSection(void* pDestination,
                              const NvnDecomposedControlSection* pDecomposedControlSection) {
    struct ReassembleControlSectionInfo {
        int offsetDataOffset;
        int offsetDataSize;
        nn::util::BinTPtr<const void> NvnDecomposedControlSection::*pSrcData;
    };

    const ReassembleControlSectionInfo ReassembleControlSectionInfoArray[5] = {
        {20, 24, &NvnDecomposedControlSection::pAssemblyData},
        {32, 28, &NvnDecomposedControlSection::pAssemblyLocalsData},
        {1776, 1780, &NvnDecomposedControlSection::pSpecializationData},
        {1992, 1996, &NvnDecomposedControlSection::pPragmaData},
        {2024, 2028, &NvnDecomposedControlSection::pUniform64InfoData},
    };

    const void* const pMetaData = pDecomposedControlSection->pMetaData.Get();
    const int offsetAssemblyDataOffset = 20;
    ptrdiff_t assemblyDataOffset =
        *nn::util::ConstBytePtr(pMetaData, offsetAssemblyDataOffset).Get<int>();

    memcpy(pDestination, pMetaData, assemblyDataOffset);

    for (int idxSection = 0, sectionCount = 5; idxSection < sectionCount; ++idxSection) {
        const ReassembleControlSectionInfo& rTarget = ReassembleControlSectionInfoArray[idxSection];
        const void* pSrcData = (pDecomposedControlSection->*rTarget.pSrcData).Get();
        if (pSrcData != nullptr) {
            ptrdiff_t dataOffset =
                *nn::util::ConstBytePtr(pMetaData, rTarget.offsetDataOffset).Get<int>();
            void* pDstData = nn::util::BytePtr(pDestination, dataOffset).Get();
            size_t dataSize = *nn::util::ConstBytePtr(pMetaData, rTarget.offsetDataSize).Get<int>();

            memcpy(pDstData, pSrcData, dataSize);
        }
    }
}

/**
 * Initializes a shader from precompiled binaries.
 *
 * @param pThis Shader being initialized.
 * @param rInfo Shader description.
 * @return Initialization result.
 */
ShaderInitializeResult InitializeBinaryShader(ShaderImplNvn8* pThis, const ShaderInfo& rInfo) {
    if (!rInfo.ToData()->flags.GetBit(ShaderInfoData::Flag_ResShader)) {
        int sourceCount = 0;
        NVNshaderData sources[6];

        for (int idxStage = 0; idxStage < 6; ++idxStage) {
            ShaderStage stage = static_cast<ShaderStage>(idxStage);
            const NVNshaderData* pShaderData =
                static_cast<const NVNshaderData*>(rInfo.GetShaderCodePtr(stage));

            if (pShaderData != nullptr) {
                sources[sourceCount++] = *pShaderData;
            }
        }

        if (!nvnProgramSetShaders(GetNvnProgram(pThis), sourceCount, sources)) {
            return ShaderInitializeResult_SetupFailed;
        }
    } else {
        int sourceCount = 0;
        NVNshaderDataExt sources[6];

        for (int idxStage = 0; idxStage < 6; ++idxStage) {
            ShaderStage stage = static_cast<ShaderStage>(idxStage);
            const NvnShaderCode* pNvnShaderCode =
                static_cast<const NvnShaderCode*>(rInfo.GetShaderCodePtr(stage));

            if (pNvnShaderCode != nullptr) {
                NVNshaderDataExt& rSource = sources[sourceCount++];
                rSource.data = pNvnShaderCode->dataAddress;
                rSource.control = pNvnShaderCode->pControl.Get();
                rSource.debugDataHash = pNvnShaderCode->pDebugDataHash.Get();

                const NvnDecomposedControlSection* pDecomposedControlSection =
                    pNvnShaderCode->pDecomposedControlSection.Get();
                if (pDecomposedControlSection != nullptr) {
                    void* pControl = const_cast<void*>(pNvnShaderCode->pControl.Get());
                    ReassembleControlSection(pControl, pDecomposedControlSection);
                }
            }
        }

        if (!nvnProgramSetShadersExt(GetNvnProgram(pThis), sourceCount, sources)) {
            return ShaderInitializeResult_SetupFailed;
        }

        if (rInfo.ToData()->flags.GetBit(ShaderInfoData::Flag_ResShader)) {
            const ResShaderProgramData* pResShaderProgram =
                nn::util::ConstBytePtr(&rInfo, 0).Get<ResShaderProgramData>();
            pThis->ToData()->pReflection = pResShaderProgram->pShaderReflection.Get();
        }
    }

    return ShaderInitializeResult_Success;
}

}  // namespace

/**
 * Returns the required alignment of shader binary code.
 *
 * @param pDevice Device (unused).
 * @return Alignment in bytes.
 */
size_t ShaderImplNvn8::GetBinaryCodeAlignment(DeviceImpl<ApiVariationNvn8>* pDevice) {
    return 256;
}

/**
 * Constructs an uninitialized shader.
 */
ShaderImplNvn8::ShaderImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the shader. Finalize must be called beforehand.
 */
ShaderImplNvn8::~ShaderImpl() {}

/**
 * Initializes the shader program from source or binary shader code.
 *
 * @param pDevice Device the shader is created on.
 * @param rInfo Shader description.
 * @return Initialization result.
 */
ShaderInitializeResult ShaderImplNvn8::Initialize(DeviceImplNvn8* pDevice, const InfoType& rInfo) {
    pNvnProgram = &nvnProgram;
    flags = rInfo.ToData()->flags;
    nvnProgramInitialize(GetNvnProgram(this), GetNvnDevice(pDevice));
    ShaderInitializeResult result = ShaderInitializeResult_Success;

    switch (rInfo.GetCodeType()) {
    case ShaderCodeType_Source:
        result = InitializeSourceShader(this, rInfo, pDevice);
        break;

    case ShaderCodeType_Binary:
        result = InitializeBinaryShader(this, rInfo);
        break;

    default:
        result = ShaderInitializeResult_InvalidType;
        break;
    }

    if (result == ShaderInitializeResult_Success) {
        if (!rInfo.IsSeparationEnabled()) {
            nvnShaderStageBits = rInfo.GetShaderCodePtr(ShaderStage_Compute) != nullptr ?
                                     NVN_SHADER_STAGE_COMPUTE_BIT :
                                     NVN_SHADER_STAGE_ALL_GRAPHICS_BITS;
        }

        flags.SetBit(Flag_Shared, false);
        state = State_Initialized;
    }

    return result;
}

/**
 * Finalizes the shader program and releases any online compiled shader.
 *
 * @param pDevice Device the shader was created on (unused).
 */
void ShaderImplNvn8::Finalize(DeviceImplNvn8* pDevice) {
    nvnProgramFinalize(static_cast<NVNprogram*>(pNvnProgram.ptr));
    pNvnProgram = nullptr;

    if (pOnlineCompiledShader.ptr != nullptr) {
        static_cast<OnlineCompiledShader*>(pOnlineCompiledShader.ptr)->~OnlineCompiledShader();
        free(pOnlineCompiledShader.ptr);
        pOnlineCompiledShader = nullptr;
    }

    state = State_NotInitialized;
}

/**
 * Looks up the binding slot of a shader interface.
 *
 * @param stage Shader stage.
 * @param shaderInterfaceType Kind of interface.
 * @param pName Interface name.
 * @return Slot of the interface, or -1 if it was not found.
 */
int ShaderImplNvn8::GetInterfaceSlot(ShaderStage stage, ShaderInterfaceType shaderInterfaceType,
                                     const char* pName) const {
    if (pOnlineCompiledShader.ptr != nullptr) {
        return static_cast<const OnlineCompiledShader*>(pOnlineCompiledShader.ptr)
            ->GetInterfaceSlot(stage, shaderInterfaceType, pName);
    }

    static nn::util::BinTPtr<ResShaderReflectionStageData> const ResShaderReflectionData::*
        s_pResShaderReflectionStages[6] = {
            &ResShaderReflectionData::pVertexReflection,
            &ResShaderReflectionData::pHullReflection,
            &ResShaderReflectionData::pDomainReflection,
            &ResShaderReflectionData::pGeometryReflection,
            &ResShaderReflectionData::pPixelReflection,
            &ResShaderReflectionData::pComputeReflection,
        };

    static nn::util::BinTPtr<nn::util::ResDic> const ResShaderReflectionStageData::*
        s_pInterfaceDics[6] = {
            &ResShaderReflectionStageData::pShaderInputDic,
            &ResShaderReflectionStageData::pShaderOutputDic,
            &ResShaderReflectionStageData::pSamplerDic,
            &ResShaderReflectionStageData::pConstantBufferDic,
            &ResShaderReflectionStageData::pUnorderedAccessBufferDic,
            &ResShaderReflectionStageData::pImageDic,
        };

    static int32_t const ResShaderReflectionStageData::*s_pOffsets[6] = {
        nullptr,
        &ResShaderReflectionStageData::offsetShaderOutput,
        &ResShaderReflectionStageData::offsetSampler,
        &ResShaderReflectionStageData::offsetConstantBuffer,
        &ResShaderReflectionStageData::offsetUnorderedAccessBuffer,
        &ResShaderReflectionStageData::offsetImage,
    };

    static nn::util::BinTPtr<nn::util::ResDic> const ResShaderReflectionStageData2::*
        s_pInterfaceDics2[2] = {
            &ResShaderReflectionStageData2::pSeparateTextureDic,
            &ResShaderReflectionStageData2::pSeparateSamplerDic,
        };

    static int32_t const ResShaderReflectionStageData2::*s_pOffsets2[2] = {
        &ResShaderReflectionStageData2::offsetSeparateTexture,
        &ResShaderReflectionStageData2::offsetSeparateSampler,
    };

    const ResShaderReflectionData* pResShaderReflection =
        static_cast<const ResShaderReflectionData*>(pReflection.ptr);
    const ResShaderReflectionStageData* pResShaderReflectionStage =
        (pResShaderReflection->*s_pResShaderReflectionStages[stage]).Get();
    if (pResShaderReflectionStage != nullptr) {
        const nn::util::ResDic* pResDic = nullptr;
        int offset = 0;

        if (shaderInterfaceType <= ShaderInterfaceType_Image) {
            pResDic = (pResShaderReflectionStage->*s_pInterfaceDics[shaderInterfaceType]).Get();
            if (s_pOffsets[shaderInterfaceType] != nullptr) {
                offset = pResShaderReflectionStage->*s_pOffsets[shaderInterfaceType];
            }
        } else {
            const ResShaderReflectionStageData2* pResShaderReflectionStage2 =
                pResShaderReflectionStage->pReflectionStageData2.Get();
            if (pResShaderReflectionStage2 != nullptr) {
                int shaderInterfaceType2 =
                    shaderInterfaceType - ShaderInterfaceType_SeparateTexture;
                pResDic =
                    (pResShaderReflectionStage2->*s_pInterfaceDics2[shaderInterfaceType2]).Get();
                offset = pResShaderReflectionStage2->*s_pOffsets2[shaderInterfaceType2];
            }
        }

        if (pResDic != nullptr) {
            int idxFound = pResDic->FindIndex(pName);
            if (idxFound >= 0) {
                return pResShaderReflectionStage->pShaderSlotArray.Get()[idxFound + offset];
            }
        }
    }

    return -1;
}

/**
 * Retrieves the work group size of a compute shader.
 *
 * @param pOutWorkGroupSizeX Receives the X size.
 * @param pOutWorkGroupSizeY Receives the Y size.
 * @param pOutWorkGroupSizeZ Receives the Z size.
 */
void ShaderImplNvn8::GetWorkGroupSize(int* pOutWorkGroupSizeX, int* pOutWorkGroupSizeY,
                                      int* pOutWorkGroupSizeZ) const {
    if (pOnlineCompiledShader.ptr != nullptr) {
        static_cast<const OnlineCompiledShader*>(pOnlineCompiledShader.ptr)
            ->GetWorkGroupSize(pOutWorkGroupSizeX, pOutWorkGroupSizeY, pOutWorkGroupSizeZ);
        return;
    }

    const ResShaderReflectionData* pResShaderReflection =
        static_cast<const ResShaderReflectionData*>(pReflection.ptr);
    const ResShaderReflectionStageData* pPerStageShaderInfo =
        pResShaderReflection->pComputeReflection.Get();

    *pOutWorkGroupSizeX = pPerStageShaderInfo->computeWorkGroupSizeX;
    *pOutWorkGroupSizeY = pPerStageShaderInfo->computeWorkGroupSizeY;
    *pOutWorkGroupSizeZ = pPerStageShaderInfo->computeWorkGroupSizeZ;
}

}  // namespace nn::gfx::detail
