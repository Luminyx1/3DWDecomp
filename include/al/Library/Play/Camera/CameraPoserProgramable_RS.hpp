#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {

class CameraPoserProgramable_RS : public CameraPoser_RS {
public:
    CameraPoserProgramable_RS(const sead::Vector3f* pPos, const sead::Vector3f* pAt,
                              const sead::Vector3f* pUp);
    CameraPoserProgramable_RS();

    void update() override;

    void setPose(const sead::Vector3f& rPos, const sead::Vector3f& rAt, const sead::Vector3f& rUp);

    const sead::Vector3f* mPosPtr;
    const sead::Vector3f* mAtPtr;
    const sead::Vector3f* mUpPtr;
};

static_assert(sizeof(CameraPoserProgramable_RS) == 0x160);

class CameraPoserProgramableAngle : public CameraPoser_RS {
public:
    CameraPoserProgramableAngle(const sead::Vector3f* pAt, const f32* pDistance,
                                const f32* pAngleDegreeH, const f32* pAngleDegreeV);

    void update() override;

private:
    const sead::Vector3f* mAtPtr;
    const f32* mDistancePtr;
    const f32* mAngleDegreeHPtr;
    const f32* mAngleDegreeVPtr;
};

static_assert(sizeof(CameraPoserProgramableAngle) == 0x168);

class CameraPoserProgramableKeepColliderPreCamera : public CameraPoserProgramable_RS {
public:
    CameraPoserProgramableKeepColliderPreCamera(const sead::Vector3f* pPos,
                                                const sead::Vector3f* pAt,
                                                const sead::Vector3f* pUp);

    void init() override;
    void start(const CameraStartInfo& rInfo) override;
};

}  // namespace al
