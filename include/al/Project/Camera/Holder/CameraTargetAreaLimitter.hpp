#pragma once

#include <math/seadVector.h>

namespace al {
class AreaShape;
struct PlacementInfo;

class CameraTargetAreaLimitter {
public:
    static CameraTargetAreaLimitter* tryCreate(const PlacementInfo& rInfo);

    CameraTargetAreaLimitter(const AreaShape* pShape);

    bool applyAreaLimit(sead::Vector3f* pOut, const sead::Vector3f& rPos) const;

private:
    const AreaShape* mAreaShape;
};

static_assert(sizeof(CameraTargetAreaLimitter) == 0x8);

}  // namespace al
