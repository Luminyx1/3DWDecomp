#pragma once

#include <cstring>

#include <math/seadMatrix.h>
#include <nn/util/util_MatrixApi.h>
#include <nn/util/util_VectorApi.h>
#include <nn/vfx/Handle.h>
#include <nn/vfx/System.h>
#include <heap/seadHeapMgr.h>
#include <mc/seadCoreInfo.h>
#include <ptcl/seadPtclConfig.h>
#include <ptcl/seadPtclEditorInterface.h>
#include <thread/seadCriticalSection.h>
#include <time/seadTickTime.h>

namespace sead {
namespace ptcl {

class Handle : public nn::vfx::Handle {
public:
    Handle() {
        m_EmitterSet = nullptr;
        m_CreateId = -1;
    }
};

class PtclSystem : public nn::vfx::System {
public:
    explicit PtclSystem(const Config& rConfig);
    ~PtclSystem() override {}

    void entryResource(sead::Heap* pHeap, void* pData, s32 resourceId) {
        mResourceHeaps[resourceId] = pHeap;
        Heap heap(pHeap);
        ScopedCurrentHeapSetter setter(pHeap);
        EntryResource(&heap, pData, resourceId, false, nullptr);
        GetResource(resourceId)->RegisterTextureViewToDescriptorPool(mRegisterTextureViewSlot,
                                                                     nullptr);
    }

    void calc(u8 groupId, f32 frameRate) {
        if (groupId == cEditorGroupId) {
            if (mViewerSystem == nullptr) {
                return;
            }

            if (mIsViewerPause) {
                Calculate(cEditorGroupId, 0.0f, nn::vfx::BufferSwapMode_None);
            } else {
                Calculate(cEditorGroupId, frameRate, nn::vfx::BufferSwapMode_Swap);
            }
        } else {
            Calculate(groupId, frameRate, nn::vfx::BufferSwapMode_Swap);
        }
    }

    nn::vfx::RegisterTextureViewSlot getRegisterTextureViewSlot() const {
        return mRegisterTextureViewSlot;
    }

    nn::vfx::RegisterSamplerSlot getRegisterSamplerSlot() const { return mRegisterSamplerSlot; }

    void setRegisterTextureViewSlot(nn::vfx::RegisterTextureViewSlot pFunc) {
        mRegisterTextureViewSlot = pFunc;
    }

    void setUnregisterTextureViewSlot(nn::vfx::UnregisterTextureViewSlot pFunc) {
        mUnregisterTextureViewSlot = pFunc;
    }

    void setRegisterSamplerSlot(nn::vfx::RegisterSamplerSlot pFunc) {
        mRegisterSamplerSlot = pFunc;
    }

    void setUnregisterSamplerSlot(nn::vfx::UnregisterSamplerSlot pFunc) {
        mUnregisterSamplerSlot = pFunc;
    }

    void beginRender(nn::gfx::CommandBuffer* pCommandBuffer, const Matrix44f& rProjMtx,
                     const Matrix34f& rViewMtx, const Vector3f& rCamPos, f32 near, f32 far,
                     f32 fovy);

    static const u8 cEditorGroupId = 63;

    bool createEmitterSetID(Handle* pHandle, const Matrix34f& rMtx, s32 emitterSetId, s32 resourceId,
                            u8 groupId) {
        mCriticalSection.lock();
        nn::util::Matrix4x3fType mtx;
        nn::util::MatrixLoad(&mtx, reinterpret_cast<const nn::util::FloatColumnMajor4x3&>(rMtx));

        bool isSuccess = CreateEmitterSetId(pHandle, emitterSetId, resourceId, groupId, nullptr, false) &&
                         pHandle->IsValid();

        if (isSuccess) {
            pHandle->GetEmitterSet()->SetMatrix(mtx);
        }

        mCriticalSection.unlock();
        return isSuccess;
    }

    bool createEmitterSetID(Handle* pHandle, const Vector3f& rTrans, s32 emitterSetId, s32 resourceId,
                            u8 groupId) {
        mCriticalSection.lock();
        nn::util::Vector3fType trans;
        nn::util::VectorLoad(&trans, reinterpret_cast<const nn::util::Float3&>(rTrans));

        bool isSuccess = CreateEmitterSetId(pHandle, emitterSetId, resourceId, groupId, nullptr, false) &&
                         pHandle->IsValid();

        if (isSuccess) {
            nn::util::Matrix4x3fType mtx;
            nn::util::MatrixIdentity(&mtx);
            nn::util::MatrixSetAxisW(&mtx, trans);
            pHandle->GetEmitterSet()->SetMatrix(mtx);
        }

        mCriticalSection.unlock();
        return isSuccess;
    }

private:
    TickTime mCreateTick;
    Config mConfig;
    Heap mSystemHeap;
    Heap mGpuHeap;
    Heap mResourceHeap;
    Heap mEditorHeap;
    Heap mEditorGpuHeap;
    sead::Heap** mResourceHeaps;
    void* mViewerSystem;
    PtclEditorInterface mEditorInterface;
    void* _2868;
    void* _2870;
    void* _2878;
    void* _2880;
    void* _2888;
    void* _2890;
    s32 _2898;
    s32 _289c;
    s32 _28a0;
    s32 _28a4;
    s32 _28a8;
    s32 _28ac;
    bool mIsViewerPause;
    CriticalSection mCriticalSection;
    nn::vfx::RegisterTextureViewSlot mRegisterTextureViewSlot;
    nn::vfx::UnregisterTextureViewSlot mUnregisterTextureViewSlot;
    nn::vfx::RegisterSamplerSlot mRegisterSamplerSlot;
    nn::vfx::UnregisterSamplerSlot mUnregisterSamplerSlot;
};

static_assert(sizeof(PtclSystem) == 0x2918);

inline PtclSystem::PtclSystem(const Config& rConfig) : nn::vfx::System(rConfig) {
    mSystemHeap.setHeap(rConfig.getSystemHeap());
    mGpuHeap.setHeap(rConfig.getGpuHeap());
    mResourceHeap.setHeap(rConfig.getResourceHeap());

    if (rConfig.getEditorHeap() != nullptr && rConfig.getEditorGpuHeap() != nullptr) {
        mEditorHeap.setHeap(rConfig.getEditorHeap());
        mEditorHeap.getHeap()->enableLock(true);
        mEditorGpuHeap.setHeap(rConfig.getEditorGpuHeap());
        mEditorGpuHeap.getHeap()->enableLock(true);
    } else {
        mViewerSystem = nullptr;
    }

    mResourceHeaps = static_cast<sead::Heap**>(
        rConfig.getSystemHeap()->tryAlloc(sizeof(sead::Heap*) * rConfig.GetResourceNum(), 8));
    memset(mResourceHeaps, 0, sizeof(sead::Heap*) * rConfig.GetResourceNum());

    _2868 = nullptr;
    _2870 = nullptr;
    _2878 = nullptr;
    _2880 = nullptr;
    _2888 = nullptr;
    _2890 = nullptr;
    _2898 = 0;
    _28a4 = 0;
    _28a8 = 0;
    _28ac = 0;
    memcpy(&mConfig, &rConfig, sizeof(Config));
    mRegisterTextureViewSlot = nullptr;
    mUnregisterTextureViewSlot = nullptr;
    mRegisterSamplerSlot = nullptr;
    mUnregisterSamplerSlot = nullptr;
}
}  // namespace ptcl
}  // namespace sead
