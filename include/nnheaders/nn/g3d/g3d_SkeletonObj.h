#pragma once

#include <nn/gfx/gfx_Buffer.h>
#include <nn/gfx/gfx_Types.h>
#include <nn/g3d/g3d_World.h>
#include <nn/types.h>
#include "nn/g3d/g3d_ResSkeleton.h"
#include "nn/util/util_MathTypes.h"

namespace nn::g3d {

struct LocalMtx {
    u8 _0[0x60];
};

// TODO
class SkeletonObj {
public:
    // index selects a bone in the skeleton resource array.
    const ResBone* GetBone(int index) const { return &m_Bones[index]; }
    const ResSkeleton* GetRes() const { return m_Res; }

    const nn::util::Matrix4x3fType* GetWorldMtxArray() const { return m_WorldMtxArray; }
    int GetBoneCount() const { return m_BoneCount; }
    const LocalMtx* GetLocalMtx(int index) const { return &m_pLocalMtxArray[index]; }

    const gfx::Buffer* GetMtxBlock(int bufferIndex) const {
        return m_pMtxBlockArray ? &m_pMtxBlockArray[bufferIndex] : nullptr;
    }

    size_t GetMtxBlockSize() const { return m_MtxBlockSize; }

    void SetCalculateWorldCallback(ICalculateWorldCallback* pCallback) {
        m_Callback = pCallback;
        m_CallbackBoneIndex = m_BoneCount == 0 ? -1 : 0;
    }

private:
    const ResSkeleton* m_Res;
    void* _8;
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
    // TODO: the rest of the members
};

}  // namespace nn::g3d
