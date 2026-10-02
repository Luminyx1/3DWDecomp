#pragma once

#include <nn/gfx/gfx_Buffer.h>
#include <nn/gfx/gfx_Types.h>
#include <nn/g3d/g3d_World.h>
#include <nn/g3d/g3d_Resources.h>
#include <nn/types.h>
#include "nn/g3d/g3d_ResSkeleton.h"
#include "nn/util/util_MathTypes.h"

namespace nn::g3d {

struct LocalMtx {
    nn::Bit32 flag;
    nn::util::Vector3fType scale;
    nn::util::Matrix4x3fType mtx;
};

static_assert(sizeof(LocalMtx) == 0x60);

// TODO
class SkeletonObj {
  public:
    SkeletonObj()
        : m_Res(nullptr), m_Flag(0), m_BufferingCount(0), m_Bones(nullptr), m_pLocalMtxArray(nullptr),
          m_WorldMtxArray(nullptr), _28(nullptr), _30(nullptr), _38(nullptr), m_pMtxBlockArray(nullptr),
          m_MtxBlockSize(0), m_BoneCount(0), m_CallbackBoneIndex(0), m_Callback(nullptr), m_UserData(nullptr),
          m_MemoryPool(nullptr), m_MemoryPoolOffset(0), m_WorkMemory(nullptr) {}
    struct InitializeArgument {
        const ResSkeleton* resource;
        int bufferCount;
        size_t memorySize;
        size_t memoryAlignment;
        detail::WorkMemoryBlock blocks[4];
        void CalculateMemorySize();
    };
    // argument selects the skeleton; buffer supplies bufferSize bytes of working memory.
    bool Initialize(const InitializeArgument& argument, void* buffer, size_t bufferSize);
    // index selects a bone in the skeleton resource array.
    const ResBone* GetBone(int index) const { return &m_Bones[index]; }
    const ResSkeleton* GetRes() const { return m_Res; }
    // name selects a bone; returns -1 when the skeleton has no bone with that name.
    int FindBoneIndex(const char* name) const;

    const nn::util::Matrix4x3fType* GetWorldMtxArray() const { return m_WorldMtxArray; }
    int GetBoneCount() const { return m_BoneCount; }
    const LocalMtx* GetLocalMtx(int index) const { return &m_pLocalMtxArray[index]; }
    LocalMtx* GetLocalMtxArray() { return m_pLocalMtxArray; }

    const gfx::Buffer* GetMtxBlock(int bufferIndex) const {
        return (m_pMtxBlockArray != nullptr) ? &m_pMtxBlockArray[bufferIndex] : nullptr;
    }

    bool IsBlockBufferValid() const { return (m_Flag & 1) != 0; }
    // device supplies the GPU block requirements and owns the initialized buffers.
    size_t GetBlockBufferAlignment(gfx::Device* device) const;
    size_t CalculateBlockBufferSize(gfx::Device* device) const;
    // pool supplies size bytes at offset for the skeleton blocks.
    bool SetupBlockBuffer(gfx::Device* device, gfx::MemoryPool* pool, ptrdiff_t offset, size_t size);
    void CleanupBlockBuffer(gfx::Device* device);
    // world is the root model-to-world transform.
    void CalculateWorldMtx(const nn::util::Matrix4x3fType& world);
    // bufferIndex selects the buffered matrix block.
    void CalculateSkeleton(int bufferIndex);
    // output receives the billboard transform for boneIndex and view; world includes its world transform.
    void CalculateBillboardMtx(nn::util::Matrix4x3fType* output, const nn::util::Matrix4x3fType& view,
                               int boneIndex, bool world) const;

    size_t GetMtxBlockSize() const { return m_MtxBlockSize; }

    void SetCalculateWorldCallback(ICalculateWorldCallback* pCallback) {
        m_Callback = pCallback;
        m_CallbackBoneIndex = m_BoneCount == 0 ? -1 : 0;
    }

  private:
    const ResSkeleton* m_Res;
    u16 m_Flag;
    u8 m_BufferingCount;
    u8 _b[5];
    const ResBone* m_Bones;
    LocalMtx* m_pLocalMtxArray;
    nn::util::Matrix4x3fType* m_WorldMtxArray;
    void* _28;
    void* _30;
    void* _38;
    gfx::Buffer* m_pMtxBlockArray;
    size_t m_MtxBlockSize;
    u16 m_BoneCount;
    s16 m_CallbackBoneIndex;
    ICalculateWorldCallback* m_Callback;
    void* m_UserData;
    gfx::MemoryPool* m_MemoryPool;
    ptrdiff_t m_MemoryPoolOffset;
    void* m_WorkMemory;
};

} // namespace nn::g3d
