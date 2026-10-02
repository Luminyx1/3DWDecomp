#include <math/seadMathCalcCommon.h>

#include "Library/Play/Camera/CameraPoserProgramable_RS.hpp"

namespace al {

/**
 * Constructs a poser placed around a look-at position by distance and angles.
 * @param pAt Look-at position source.
 * @param pDistance Camera distance source.
 * @param pAngleDegreeH Horizontal angle source, in degrees.
 * @param pAngleDegreeV Vertical angle source, in degrees.
 */
CameraPoserProgramableAngle::CameraPoserProgramableAngle(const sead::Vector3f* pAt,
                                                         const f32* pDistance,
                                                         const f32* pAngleDegreeH,
                                                         const f32* pAngleDegreeV)
    : CameraPoser_RS("角度指定プログラマブル"), mAtPtr(pAt), mDistancePtr(pDistance),
      mAngleDegreeHPtr(pAngleDegreeH), mAngleDegreeVPtr(pAngleDegreeV) {}

/**
 * Places the camera at the configured distance and angles from the look-at position.
 */
void CameraPoserProgramableAngle::update() {
    mAt.set(*mAtPtr);

    sead::Vector3f dir(sead::Mathf::sin(sead::Mathf::deg2rad(*mAngleDegreeHPtr)), 0.0f,
                       sead::Mathf::cos(sead::Mathf::deg2rad(*mAngleDegreeHPtr)));
    f32 lengthH = sead::Mathf::cos(sead::Mathf::deg2rad(*mAngleDegreeVPtr));
    f32 length = dir.length();

    if (length > 0.0f) {
        dir *= lengthH / length;
    }

    dir.y = sead::Mathf::sin(sead::Mathf::deg2rad(*mAngleDegreeVPtr));
    f32 distance = *mDistancePtr;
    length = dir.length();

    if (length > 0.0f) {
        dir *= distance / length;
    }

    mEye.set(mAt + dir);
}

}  // namespace al
