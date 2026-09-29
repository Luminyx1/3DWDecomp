#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace sead {
class Projection;
}

namespace agl::detail {

class SeadUtil {
public:
    static bool getNearFarAspectFovy(const sead::Projection& rProjection, f32* pNear, f32* pFar,
                                     f32* pAspect, f32* pFovy, sead::Vector2f* pOffset);
    static bool setNearFarAspectFovy(sead::Projection* pProjection, f32 near, f32 far, f32 aspect,
                                     f32 fovy, const sead::Vector2f& rOffset, bool keepWidth);
};

}  // namespace agl::detail
