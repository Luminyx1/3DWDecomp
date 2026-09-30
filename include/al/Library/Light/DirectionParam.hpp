#pragma once

#include <math/seadVector.h>
#include "utility/aglParameter.h"

namespace agl::utl {
template <typename T>
class Parameter;
class ParameterObj;
}  // namespace agl::utl

namespace al {
class ActorInitInfo;

class DirectionParam {
public:
    DirectionParam() = default;

    void initByArg(const ActorInitInfo& rInfo);
    void syncToDirection();
    void initializeDir(agl::utl::ParameterObj* pParamObj, const char* pName, const char* pLabel);
    void initializeDir(const sead::Vector3f& rDir, agl::utl::ParameterObj* pParamObj,
                       const char* pName, const char* pLabel);
    void syncFromDirection(const sead::Vector3f& rDir);
    void syncFromDirection();
    void syncFromRPYDegree(const sead::Vector3f& rDegreeRPY);
    void lerp(const DirectionParam& rStart, const DirectionParam& rEnd, f32 rate);

    const sead::Vector3f& getDirection() const { return mDirection; }

protected:
    agl::utl::Parameter<sead::Vector2f>* mCoordinate = nullptr;
    sead::Vector3f mDirection = {0.0f, -1.0f, 0.0f};
    f32 mRotateDegreeY = 0.0f;
};

static_assert(sizeof(DirectionParam) == 0x18);
class PlaneParam : public DirectionParam {
public:
    PlaneParam() = default;

    void initialize(const sead::Vector3f& rDir, agl::utl::ParameterObj* pParamObj,
                    const char* pPlaneName);

private:
    agl::utl::Parameter<f32> mDistanceFromOrigin;
};

static_assert(sizeof(PlaneParam) == 0x38);

}  // namespace al
