/**
 * @file Config.h
 * @brief VFX configuration.
 */

#pragma once

#include <nn/types.h>

namespace nn {
namespace gfx {
template <class TTarget>
class TDevice;
}  // namespace gfx

namespace vfx {
class Heap;

class Config {
public:
    Config() {
        m_GfxDevice = nullptr;
        m_SystemHeap = nullptr;
        m_DynamicHeap = nullptr;
        m_EmitterSetNum = 0x40;
        m_EmitterNum = 0x20;
        m_StripeNum = 0x20;
        m_SuperStripeNum = 0x20;
        m_ResourceNum = 8;
        m_GpuBufferSize = 0x10000;
        m_ParticleSortBufferNum = 0x400;
        m_EnableDoubleBuffer = false;
        m_MultiBufferNum = 1;
        m_EnableDebugMode = false;
        m_EnableComputeShaderBatch = true;
        m_EnableDrawPathCheck = false;
        m_EnableGpuBufferFixed = false;
        m_TemporaryBufferSize = 0x100000;
    }

    virtual ~Config() {}

    void* GetGfxDevice() const { return m_GfxDevice; }
    Heap* GetSystemHeap() const { return m_SystemHeap; }
    Heap* GetDynamicHeap() const { return m_DynamicHeap; }
    s32 GetResourceNum() const { return m_ResourceNum; }

    void SetGfxDevice(void* pDevice) { m_GfxDevice = pDevice; }
    void SetSystemHeap(Heap* pHeap) { m_SystemHeap = pHeap; }
    void SetDynamicHeap(Heap* pHeap) { m_DynamicHeap = pHeap; }
    void SetEmitterSetNum(s32 num) { m_EmitterSetNum = num; }
    void SetEmitterNum(s32 num) { m_EmitterNum = num; }
    void SetStripeNum(s32 num) { m_StripeNum = num; }
    void SetResourceNum(s32 num) { m_ResourceNum = num; }
    void SetGpuBufferSize(size_t size) { m_GpuBufferSize = size; }
    void SetParticleSortBufferNum(s32 num) { m_ParticleSortBufferNum = num; }
    void SetMultiBufferNum(s32 num) { m_MultiBufferNum = num; }
    void SetEnableComputeShaderBatchProcess(bool isEnable) { m_EnableComputeShaderBatch = isEnable; }
    void SetEnableDrawPathCheck(bool isEnable) { m_EnableDrawPathCheck = isEnable; }
    void SetEnableGpuBufferFixed(bool isEnable) { m_EnableGpuBufferFixed = isEnable; }
    void SetTemporaryBufferSize(size_t size) { m_TemporaryBufferSize = size; }

    void* m_GfxDevice;
    Heap* m_SystemHeap;
    Heap* m_DynamicHeap;
    s32 m_EmitterSetNum;
    s32 m_EmitterNum;
    s32 m_StripeNum;
    s32 m_SuperStripeNum;
    s32 m_ResourceNum;
    size_t m_GpuBufferSize;
    s32 m_ParticleSortBufferNum;
    bool m_EnableDoubleBuffer;
    s32 m_MultiBufferNum;
    bool m_EnableDebugMode;
    bool m_EnableComputeShaderBatch;
    bool m_EnableDrawPathCheck;
    bool m_EnableGpuBufferFixed;
    size_t m_TemporaryBufferSize;
};
}  // namespace vfx
}  // namespace nn
