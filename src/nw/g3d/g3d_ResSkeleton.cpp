#include <nn/g3d/g3d_ResSkeleton.h>

namespace nn::g3d {
static_assert(sizeof(ResBoneData) == 0x60, "Bone resource size");
static_assert(sizeof(ResSkeletonData) == 0x40, "Skeleton resource size");
void ResSkeleton::Reset() { pUserPtr.Clear(); }
// guard selects resource state to preserve; bit zero retains the user pointer.
void ResSkeleton::Reset(nn::Bit32 guard) {
    if (!(guard & ResetGuardFlag_UserPtr)) pUserPtr.Clear();
}

// index selects a bone in depth-first order; return the first bone outside its branch.
int ResSkeleton::GetBranchEndIndex(int index) const {
    int count = boneCount;
    const ResBone* bones = pBoneArray.Get();
    int parent = bones[index].parentIndex;

    if (index == 0) parent = -1;
    int end = index + 1;

    while (end < count && bones[end].parentIndex > parent) ++end;
    return end;
}

void ResSkeleton::UpdateBillboardMode() {
    int count = boneCount;

    for (int i = 0; i < count; ++i) {
        ResBone* bone = GetBone(i);

        if (bone->flag & (0x6 << ResBone::Shift_Billboard)) {
            if (i + 1 < count && GetBone(i + 1)->parentIndex == i) bone->billboardIndex = i;
            else bone->billboardIndex = ResBone::Flag_BillboardIndexNone;
        } else {
            bone->flag &= ~ResBone::Mask_Billboard;

            if (i != 0) {
                ResBone* parent = GetBone(bone->parentIndex);
                bone->billboardIndex = parent->billboardIndex;

                if (static_cast<s16>(parent->billboardIndex) != -1) bone->flag |= ResBone::Flag_BillboardChild;
            } else bone->billboardIndex = ResBone::Flag_BillboardIndexNone;
        }
    }
}
}
