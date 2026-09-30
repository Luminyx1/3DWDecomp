#pragma once

#include <math/seadVector.h>

namespace nn::g3d {
class ModelObj;
class ResSkeleton;
}  // namespace nn::g3d

namespace al {
class SklAnimRetargettingInfo {
public:
    SklAnimRetargettingInfo(const nn::g3d::ModelObj* pModel, const nn::g3d::ModelObj* pTargetModel,
                            const sead::Vector3f& rScale);
    SklAnimRetargettingInfo(const nn::g3d::ResSkeleton* pSkeleton,
                            const nn::g3d::ResSkeleton* pTargetSkeleton,
                            const sead::Vector3f& rScale);

private:
    u8 _0[0x18];
};
}  // namespace al
