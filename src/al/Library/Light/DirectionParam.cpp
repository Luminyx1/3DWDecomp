#include "Library/Light/DirectionParam.hpp"

#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>
#include "utility/aglParameter.h"
#include "utility/aglParameterObj.h"

#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Initializes the direction from the placement arguments.
 * @param rInfo Actor init info.
 */
void DirectionParam::initByArg(const ActorInitInfo& rInfo) {
    f32 longitudeDegree = 0.0f;
    f32 latitudeDegree = 0.0f;
    tryGetArg(&longitudeDegree, rInfo, "DirectionParamLongitudeDegree");
    tryGetArg(&latitudeDegree, rInfo, "DirectionParamLatitudeDegree");
    sead::Vector2f coordinate = {sead::Mathf::deg2rad(longitudeDegree),
                                 sead::Mathf::deg2rad(latitudeDegree)};
    mCoordinate = new agl::utl::Parameter<sead::Vector2f>(
        coordinate, "PlacementArg", "配置情報からのパラメータ", new agl::utl::ParameterObj);
    syncToDirection();
}

/**
 * Updates the direction from the longitude and latitude.
 */
void DirectionParam::syncToDirection() {
    mDirection.y = -sead::Mathf::sin((*mCoordinate)->y);
    f32 rho = -sead::Mathf::cos((*mCoordinate)->y);
    mDirection.x = sead::Mathf::sin((*mCoordinate)->x) * rho;
    mDirection.z = sead::Mathf::cos((*mCoordinate)->x) * rho;
    rotateVectorDegreeY(&mDirection, mRotateDegreeY);
}

/**
 * Registers the longitude and latitude parameter.
 * @param pParamObj Parameter object to register to.
 * @param pName Parameter name.
 * @param pLabel Parameter label.
 */
void DirectionParam::initializeDir(agl::utl::ParameterObj* pParamObj, const char* pName,
                                   const char* pLabel) {
    mCoordinate =
        new agl::utl::Parameter<sead::Vector2f>(sead::Vector2f::zero, pName, pLabel, pParamObj);
    syncToDirection();
}

/**
 * Registers the longitude and latitude parameter with an initial direction.
 * @param rDir Initial direction.
 * @param pParamObj Parameter object to register to.
 * @param pName Parameter name.
 * @param pLabel Parameter label.
 */
void DirectionParam::initializeDir(const sead::Vector3f& rDir, agl::utl::ParameterObj* pParamObj,
                                   const char* pName, const char* pLabel) {
    mCoordinate =
        new agl::utl::Parameter<sead::Vector2f>(sead::Vector2f::zero, pName, pLabel, pParamObj);
    syncFromDirection(rDir);
}

/**
 * Sets the direction and updates the longitude and latitude.
 * @param rDir New direction.
 */
void DirectionParam::syncFromDirection(const sead::Vector3f& rDir) {
    mDirection = rDir;
    syncFromDirection();
}

/**
 * Updates the longitude and latitude from the direction.
 */
void DirectionParam::syncFromDirection() {
    sead::Vector3f dir = mDirection;
    rotateVectorDegreeY(&dir, mRotateDegreeY);
    f32 length = dir.length();

    if (!(length > 0.0f)) {
        return;
    }

    f32 invLength = 1.0f / length;
    f32 x = invLength * dir.x;
    f32 y = invLength * dir.y;
    f32 z = invLength * dir.z;
    f32 lengthXZ = sead::Mathf::sqrt(z * z + x * x);

    if (lengthXZ > 0.0f) {
        f32 inv = 1.0f / lengthXZ;
        f32 cosLongitude = -z * inv;
        f32 sinLongitude = -x * inv;
        (*mCoordinate)->x = sead::Mathf::atan2(sinLongitude, cosLongitude);
    }

    (*mCoordinate)->y = sead::Mathf::asin(sead::Mathf::clamp(-y, -1.0f, 1.0f));
}

/**
 * Sets the direction from roll, pitch and yaw angles.
 * @param rDegreeRPY Roll, pitch and yaw in degrees.
 */
void DirectionParam::syncFromRPYDegree(const sead::Vector3f& rDegreeRPY) {
    sead::Quatf quat;
    quat.setRPY(sead::Mathf::deg2rad(rDegreeRPY.x), sead::Mathf::deg2rad(rDegreeRPY.y),
                sead::Mathf::deg2rad(rDegreeRPY.z));
    sead::Vector3f up;
    calcQuatUp(&up, quat);
    syncFromDirection(-up);
}

/**
 * Interpolates between two directions.
 * @param rStart Start direction.
 * @param rEnd End direction.
 * @param rate Interpolation rate.
 */
void DirectionParam::lerp(const DirectionParam& rStart, const DirectionParam& rEnd, f32 rate) {
    lerpVec(&mDirection, rStart.mDirection, rEnd.mDirection, rate);

    if (normalizeOrZero(&mDirection)) {
        mDirection = rEnd.mDirection;
    }

    syncFromDirection();
}

/**
 * Registers the plane normal and distance parameters.
 * @param rDir Initial plane normal.
 * @param pParamObj Parameter object to register to.
 * @param pPlaneName Name prefix of the parameters.
 */
void PlaneParam::initialize(const sead::Vector3f& rDir, agl::utl::ParameterObj* pParamObj,
                            const char* pPlaneName) {
    StringTmp<256> normalName("%sNormal", pPlaneName);
    StringTmp<256> distanceName("%sDistance", pPlaneName);
    initializeDir(rDir, pParamObj, normalName.cstr(), "平面法線");
    mDistanceFromOrigin.init(0.0f, distanceName, "原点からの距離", "Min=-100000, Max=100000",
                             pParamObj);
}

}  // namespace al
