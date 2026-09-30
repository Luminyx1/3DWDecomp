#pragma once

#include <nn/font/font_DispStringBuffer.h>
#include <nn/font/font_GpuBuffer.h>
#include <nn/gfx/gfx_MemoryPool.h>

namespace sead { class Heap; }
namespace nn::ui2d { class DrawInfo; }

namespace eui {

class ConstantBuffer {
public:
    struct InitConfig {
        InitConfig();
        size_t paneBufferSize;
        size_t fontBufferSize;
        bool isAtomicAllocation;
        bool isPaneBufferUnallocated;
    };

    ConstantBuffer();
    ~ConstantBuffer();
    void initialize(sead::Heap* pHeap, const InitConfig& rConfig);
    void finalize();
    void map();
    void unmap();
    void setToDrawInfo(nn::ui2d::DrawInfo* pDrawInfo);
    void setToDispStringBufferInitializeArg(nn::font::DispStringBuffer::InitializeArg* pArg);
    void setToDispStringBuffer(nn::font::DispStringBuffer* pBuffer);

private:
    nn::gfx::MemoryPool m_MemoryPool;
    nn::font::GpuBuffer m_PaneBuffer;
    nn::font::GpuBuffer m_FontBuffer;
    void* m_pPoolMemory;
    s8 mBufferIndex;
    bool mIsPaneBufferInitialized;
    bool mIsFontBufferInitialized;
};

static_assert(sizeof(ConstantBuffer) == 0x1d0, "ConstantBuffer size");

}  // namespace eui
