#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace nn::g3d {
class ModelObj;
class ResSkeleton;
}  // namespace nn::g3d

namespace al {
class ByamlIter;

class SklAnimRetargettingInfo {
public:
    /// Per-bone retargetting data.
    struct Entry {
        f32 length;
        s32 targetIndex;
        f32 targetLength;
    };

    static_assert(sizeof(Entry) == 0xC);

    SklAnimRetargettingInfo(const nn::g3d::ModelObj* pModel, const nn::g3d::ModelObj* pTargetModel,
                            const sead::Vector3f& rScale);
    SklAnimRetargettingInfo(const nn::g3d::ResSkeleton* pSkeleton,
                            const nn::g3d::ResSkeleton* pTargetSkeleton,
                            const sead::Vector3f& rScale);
    SklAnimRetargettingInfo(const nn::g3d::ResSkeleton* pSkeleton, const ByamlIter& rIter,
                            const char* pTargetKey, const sead::Vector3f& rScale);
    SklAnimRetargettingInfo(const ByamlIter& rIter, const char* pKey, const char* pTargetKey,
                            const sead::Vector3f& rScale);

    const Entry& getEntry(s32 index) const { return mEntries[index]; }
    const sead::Vector3f& getScale() const { return mScale; }

private:
    Entry* mEntries;
    sead::Vector3f mScale;
};

static_assert(sizeof(SklAnimRetargettingInfo) == 0x18);

}  // namespace al
