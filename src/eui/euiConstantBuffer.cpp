#include <eui/euiConstantBuffer.h>

#include <eui/euiNwAllocator.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <heap/seadHeap.h>
#include <nn/gfx/gfx_MemoryPoolInfo.h>
#include <nn/ui2d/ui2d_DrawInfo.h>

namespace eui {

/** @brief Defaults both buffer sizes to one MiB with optional allocation modes disabled. */
ConstantBuffer::InitConfig::InitConfig()
    : paneBufferSize(0x100000), fontBufferSize(0x100000),
      isAtomicAllocation(false), isPaneBufferUnallocated(false) {}

/** @brief Creates uninitialized pane and font constant buffers. */
ConstantBuffer::ConstantBuffer()
    : m_pPoolMemory(nullptr), mBufferIndex(0), mIsPaneBufferInitialized(false),
      mIsFontBufferInitialized(false) {}

/** @brief Destroys the buffer wrapper; GPU resources must first be finalized explicitly. */
ConstantBuffer::~ConstantBuffer() = default;

/**
 * @brief Allocates a shared memory pool and initializes two copies of each requested buffer.
 * @param[in] pHeap Heap providing pool memory and GPU buffer bookkeeping allocations.
 * @param[in] rConfig Per-copy buffer sizes and allocation modes; a zero size disables that buffer.
 */
void ConstantBuffer::initialize(sead::Heap* pHeap, const InitConfig& rConfig) {
    auto* pDevice = reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice());
    nn::gfx::MemoryPoolInfo info;
    info.SetDefault();
    info.SetMemoryPoolProperty(nn::gfx::MemoryPoolProperty_CpuUncached | nn::gfx::MemoryPoolProperty_GpuCached);
    const auto size = rConfig.paneBufferSize + rConfig.fontBufferSize;
    const auto granularity = nn::gfx::MemoryPool::GetPoolMemorySizeGranularity(pDevice, info);
    const auto poolSize = (size + granularity - 1) & -granularity;
    const auto alignment = nn::gfx::MemoryPool::GetPoolMemoryAlignment(pDevice, info);

    if (poolSize) {
        m_pPoolMemory = pHeap->alloc(poolSize * 2, alignment);
        info.SetPoolMemory(m_pPoolMemory, poolSize * 2);
        m_MemoryPool.Initialize(pDevice, info);
    }

    size_t offset = 0;

    if (rConfig.paneBufferSize) {
        nn::font::GpuBuffer::InitializeArg arg;
        arg.gpuAccessFlag = nn::gfx::GpuAccess_ConstantBuffer;
        arg.bufferSize = rConfig.paneBufferSize;
        arg.bufferCount = 2;
        arg.pMemoryPool = &m_MemoryPool;
        arg.memoryPoolOffset = 0;
        arg.pAllocateFunction = NwAllocator::ui2dAllocateFunction;
        arg.pUserData = pHeap;
        arg.isAtomicAllocation = rConfig.isAtomicAllocation;
        arg.isUnallocated = rConfig.isPaneBufferUnallocated;
        m_PaneBuffer.Initialize(pDevice, arg);
        offset = rConfig.paneBufferSize * 2;
        mIsPaneBufferInitialized = true;
    }

    if (rConfig.fontBufferSize) {
        nn::font::GpuBuffer::InitializeArg arg;
        arg.gpuAccessFlag = nn::gfx::GpuAccess_ConstantBuffer;
        arg.bufferSize = rConfig.fontBufferSize;
        arg.bufferCount = 2;
        arg.pMemoryPool = &m_MemoryPool;
        arg.memoryPoolOffset = offset;
        arg.pAllocateFunction = NwAllocator::ui2dAllocateFunction;
        arg.pUserData = pHeap;
        arg.isAtomicAllocation = rConfig.isAtomicAllocation;
        m_FontBuffer.Initialize(pDevice, arg);
        mIsFontBufferInitialized = true;
    }
}

/** @brief Finalizes both GPU buffers and releases their shared memory pool. */
void ConstantBuffer::finalize() {
    auto* pDevice = reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice());
    m_PaneBuffer.Finalize(pDevice, NwAllocator::ui2dDeallocateFunctionWithFindContainHeap, nullptr);
    m_FontBuffer.Finalize(pDevice, NwAllocator::ui2dDeallocateFunctionWithFindContainHeap, nullptr);
    m_MemoryPool.Finalize(pDevice);
    NwAllocator::ui2dDeallocateFunctionWithFindContainHeap(m_pPoolMemory, nullptr);
}

/** @brief Remaps each initialized buffer to the selected CPU and GPU access index. */
void ConstantBuffer::map() {
    if (mIsPaneBufferInitialized) {
        m_PaneBuffer.Unmap();
        m_PaneBuffer.m_GpuAccessBufferIndex = mBufferIndex;
        m_PaneBuffer.Map(mBufferIndex);
    }

    if (mIsFontBufferInitialized) {
        m_FontBuffer.Unmap();
        m_FontBuffer.m_GpuAccessBufferIndex = mBufferIndex;
        m_FontBuffer.Map(mBufferIndex);
    }
}

/** @brief Leaves the mappings intact; the next map call performs the actual unmap. */
void ConstantBuffer::unmap() {}

/**
 * @brief Attaches the pane and font constant buffers to drawing state.
 * @param[out] pDrawInfo Drawing state whose constant-buffer pointers are replaced.
 */
void ConstantBuffer::setToDrawInfo(nn::ui2d::DrawInfo* pDrawInfo) {
    pDrawInfo->m_pConstantBuffer = &m_PaneBuffer;
    pDrawInfo->m_pFontConstantBuffer = &m_FontBuffer;
}

/**
 * @brief Selects the font constant buffer for display-string initialization.
 * @param[in,out] pArg Initialization arguments whose constant-buffer pointer is replaced.
 */
void ConstantBuffer::setToDispStringBufferInitializeArg(nn::font::DispStringBuffer::InitializeArg* pArg) {
    pArg->pConstantBuffer = &m_FontBuffer;
}

/**
 * @brief Attaches the font constant buffer to an existing display-string buffer.
 * @param[in,out] pBuffer Display-string buffer receiving the constant-buffer association.
 */
void ConstantBuffer::setToDispStringBuffer(nn::font::DispStringBuffer* pBuffer) {
    pBuffer->SetConstantBuffer(&m_FontBuffer);
}

}  // namespace eui
