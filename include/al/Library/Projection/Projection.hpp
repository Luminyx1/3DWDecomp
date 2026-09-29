#pragma once

#include <gfx/seadProjection.h>
#include <math/seadMatrix.h>

namespace al {
/// A perspective projection with extra cached matrices and an offset.
class Projection {
public:
    Projection();

    f32 getNear() const;
    f32 getFar() const;
    f32 getAspect() const;
    f32 getFovy() const;
    const sead::Matrix44f& getProjMtx() const;

    sead::Projection& getProjectionSead() { return mBase; }
    const sead::Projection& getProjectionSead() const { return mBase; }

    sead::PerspectiveProjection mBase;  // _0
};
}  // namespace al
