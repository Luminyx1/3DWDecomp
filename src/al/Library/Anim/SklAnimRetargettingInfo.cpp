#include "Library/Anim/SklAnimRetargettingInfo.hpp"

#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResSkeleton.h>
#include <nn/g3d/g3d_SkeletonObj.h>
#include <nn/util/util_VectorApi.h>

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
namespace {

/**
 * Computes the length of a bone's local translation.
 * @param pBone The bone.
 * @return The translation length.
 */
inline f32 calcBoneTransLength(const nn::g3d::ResBone* pBone) {
    nn::util::Vector3fType trans;
    nn::util::VectorLoad(&trans, pBone->GetTranslate());
    float32x4_t square = vmulq_f32(trans._v, trans._v);
    float32x2_t sum = vadd_f32(vget_high_f32(square), vget_low_f32(square));
    sum = vpadd_f32(sum, sum);
    return vgetq_lane_f32(vsqrtq_f32(vcombine_f32(sum, sum)), 0);
}

}  // namespace

/**
 * Builds retargetting info between the skeletons of two model objects.
 * @param pModel The source model.
 * @param pTargetModel The target model.
 * @param rScale The retargetting scale.
 */
SklAnimRetargettingInfo::SklAnimRetargettingInfo(const nn::g3d::ModelObj* pModel,
                                                 const nn::g3d::ModelObj* pTargetModel,
                                                 const sead::Vector3f& rScale) {
    const nn::g3d::SkeletonObj* targetSkeleton = pTargetModel->GetSkeleton();
    const nn::g3d::SkeletonObj* skeleton = pModel->GetSkeleton();
    mEntries = new Entry[skeleton->GetRes()->GetBoneCount()];
    mScale = rScale;
    s32 boneCount = skeleton->GetRes()->GetBoneCount();

    for (s32 i = 0; i < boneCount; i++) {
        Entry& entry = mEntries[i];
        entry.length = calcBoneTransLength(skeleton->GetBone(i));
        const char* boneName = skeleton->GetRes()->GetBoneName(i);
        s32 targetIndex = targetSkeleton->GetRes()->FindBoneIndex(boneName);
        entry.targetIndex = targetIndex;

        if (targetIndex != -1) {
            entry.targetLength = calcBoneTransLength(targetSkeleton->GetBone(targetIndex));
        }
    }
}

/**
 * Builds retargetting info between two skeleton resources.
 * @param pSkeleton The source skeleton.
 * @param pTargetSkeleton The target skeleton.
 * @param rScale The retargetting scale.
 */
SklAnimRetargettingInfo::SklAnimRetargettingInfo(const nn::g3d::ResSkeleton* pSkeleton,
                                                 const nn::g3d::ResSkeleton* pTargetSkeleton,
                                                 const sead::Vector3f& rScale)
    : mEntries(new Entry[pSkeleton->GetBoneCount()]) {
    mScale = rScale;
    s32 boneCount = pSkeleton->GetBoneCount();

    for (s32 i = 0; i < boneCount; i++) {
        Entry& entry = mEntries[i];
        entry.length = calcBoneTransLength(pSkeleton->GetBone(i));
        const char* boneName = pSkeleton->GetBoneName(i);
        s32 targetIndex = pTargetSkeleton->FindBoneIndex(boneName);
        entry.targetIndex = targetIndex;

        if (targetIndex != -1) {
            entry.targetLength = calcBoneTransLength(pTargetSkeleton->GetBone(targetIndex));
        }
    }
}

/**
 * Builds retargetting info between a skeleton resource and target bone lengths stored in BYAML.
 * @param pSkeleton The source skeleton.
 * @param rIter The BYAML iterator holding the target bone length tables.
 * @param pTargetKey The key of the target bone length table.
 * @param rScale The retargetting scale.
 */
SklAnimRetargettingInfo::SklAnimRetargettingInfo(const nn::g3d::ResSkeleton* pSkeleton,
                                                 const ByamlIter& rIter, const char* pTargetKey,
                                                 const sead::Vector3f& rScale)
    : mEntries(nullptr) {
    ByamlIter targetIter;

    if (!rIter.tryGetIterByKey(&targetIter, pTargetKey)) {
        return;
    }

    mEntries = new Entry[pSkeleton->GetBoneCount()];
    mScale = rScale;
    s32 boneCount = pSkeleton->GetBoneCount();

    for (s32 i = 0; i < boneCount; i++) {
        Entry& entry = mEntries[i];
        entry.length = calcBoneTransLength(pSkeleton->GetBone(i));
        bool isFound =
            targetIter.tryGetFloatByKey(&entry.targetLength, pSkeleton->GetBoneName(i));
        entry.targetIndex = isFound ? 1 : -1;
    }
}

/**
 * Builds retargetting info from source and target bone length tables stored in BYAML.
 * @param rIter The BYAML iterator holding the bone length tables.
 * @param pKey The key of the source bone length table.
 * @param pTargetKey The key of the target bone length table.
 * @param rScale The retargetting scale.
 */
SklAnimRetargettingInfo::SklAnimRetargettingInfo(const ByamlIter& rIter, const char* pKey,
                                                 const char* pTargetKey,
                                                 const sead::Vector3f& rScale)
    : mEntries(nullptr) {
    ByamlIter iter;
    ByamlIter targetIter;

    if (!rIter.tryGetIterByKey(&targetIter, pTargetKey) || !rIter.tryGetIterByKey(&iter, pKey)) {
        return;
    }

    if (targetIter.getSize() != iter.getSize()) {
        return;
    }

    u32 boneCount = iter.getSize();
    mEntries = new Entry[boneCount];
    mScale = rScale;

    for (u32 i = 0; i < boneCount; i++) {
        Entry& entry = mEntries[i];
        const char* boneName = nullptr;
        iter.getKeyName(&boneName, i);
        iter.tryGetFloatByIndex(&entry.length, i);
        bool isFound = targetIter.tryGetFloatByKey(&entry.targetLength, boneName);
        entry.targetIndex = isFound ? 1 : -1;
    }
}

}  // namespace al
