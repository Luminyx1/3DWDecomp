#include "gfx/seadCamera.h"
#include "basis/seadRawPrint.h"

namespace sead
{
Camera::~Camera() = default;

LookAtCamera::~LookAtCamera() = default;

LookAtCamera::LookAtCamera(const Vector3f& rPos, const Vector3f& rAt, const Vector3f& rUp)
    : mPos(rPos), mAt(rAt), mUp(rUp)
{
    SEAD_ASSERT(mPos != mAt);
    mUp.normalize();
}

OrthoCamera::~OrthoCamera() = default;

DirectCamera::~DirectCamera() = default;

}  // namespace sead
