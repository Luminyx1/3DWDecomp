#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace sead {
class LookAtCamera;
}

namespace al {
class CameraPoser_RS;
class Projection;
}  // namespace al

namespace alCameraPoserFunction {
s32 getViewIndex(const al::CameraPoser_RS* pPoser);
const sead::LookAtCamera& getLookAtCamera(const al::CameraPoser_RS* pPoser);
const al::Projection& getProjection(const al::CameraPoser_RS* pPoser);
const sead::Matrix44f& getProjectionMtx(const al::CameraPoser_RS* pPoser);
f32 getNear(const al::CameraPoser_RS* pPoser);
f32 getFar(const al::CameraPoser_RS* pPoser);
f32 getAspect(const al::CameraPoser_RS* pPoser);
const sead::Vector3f& getPreCameraPos(const al::CameraPoser_RS* pPoser);
const sead::Vector3f& getPreLookAtPos(const al::CameraPoser_RS* pPoser);
const sead::Vector3f& getPreUpDir(const al::CameraPoser_RS* pPoser);
f32 getPreFovyDegree(const al::CameraPoser_RS* pPoser);
f32 getPreFovyRadian(const al::CameraPoser_RS* pPoser);
}  // namespace alCameraPoserFunction
