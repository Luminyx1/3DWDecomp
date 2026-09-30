#pragma once

#include "nn/g3d/g3d_ResSkeleton.h"

namespace nn::g3d {

// TODO
class SkeletonObj {
public:
    // index selects a bone in the skeleton resource array.
    const ResBone* GetBone(int index) const { return &m_Bones[index]; }
    const ResSkeleton* GetRes() const { return m_Res; }

private:
    const ResSkeleton* m_Res;
    void* _8;
    const ResBone* m_Bones;
    // TODO: the rest of the members
};

}  // namespace nn::g3d
