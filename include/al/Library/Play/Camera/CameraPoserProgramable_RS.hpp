#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {

class CameraPoserProgramable_RS : public CameraPoser_RS {
public:
    CameraPoserProgramable_RS(const sead::Vector3f* pPos, const sead::Vector3f* pAt,
                              const sead::Vector3f* pUp);
    CameraPoserProgramable_RS();

    const sead::Vector3f* mPosPtr;
    const sead::Vector3f* mAtPtr;
    const sead::Vector3f* mUpPtr;
};

class CameraPoserProgramableAngle : public CameraPoser_RS {
public:
    CameraPoserProgramableAngle(const sead::Vector3f* pAt, const f32* pAngleH, const f32* pAngleV,
                                const f32* pDistance);

    u8 _142[0x26];
};

class CameraPoserProgramableKeepColliderPreCamera : public CameraPoserProgramable_RS {
public:
    CameraPoserProgramableKeepColliderPreCamera(const sead::Vector3f* pPos,
                                                const sead::Vector3f* pAt,
                                                const sead::Vector3f* pUp);
};

}  // namespace al
