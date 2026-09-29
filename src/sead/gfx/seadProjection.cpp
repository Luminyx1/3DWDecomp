#include <gfx/seadProjection.h>

#include <gfx/seadCamera.h>
#include <gfx/seadViewport.h>

namespace sead
{
Projection::Projection()
{
    mDevicePosture = Graphics::sDefaultDevicePosture;
    mDeviceZScale = Graphics::sDefaultDeviceZScale;
    mDeviceZOffset = Graphics::sDefaultDeviceZOffset;
}

void Projection::updateAttributesForDirectProjection() {}

const Matrix44f& Projection::getProjectionMatrix() const
{
    updateMatrixImpl_();
    return mMatrix;
}

void Projection::updateMatrixImpl_() const
{
    if (mDirty)
    {
        doUpdateMatrix(const_cast<Matrix44f*>(&mMatrix));
        mDirty = false;
        mDeviceDirty = true;
    }

    if (mDeviceDirty)
    {
        doUpdateDeviceMatrix(const_cast<Matrix44f*>(&mDeviceMatrix), mMatrix, mDevicePosture);
        mDeviceDirty = false;
    }
}

Matrix44f* Projection::getProjectionMatrixMutable()
{
    updateMatrixImpl_();
    return &mMatrix;
}

const Matrix44f& Projection::getDeviceProjectionMatrix() const
{
    updateMatrixImpl_();
    return mDeviceMatrix;
}

void Projection::cameraPosToScreenPos(Vector3f* pScreenPos, const Vector3f& rCameraPos) const
{
    pScreenPos->setMul(getProjectionMatrix(), rCameraPos);
}

void Projection::screenPosToCameraPos(Vector3f* pCameraPos, const Vector3f& rScreenPos) const
{
    doScreenPosToCameraPosTo(pCameraPos, rScreenPos);
}

void Projection::screenPosToCameraPos(Vector3f* pCameraPos, const Vector2f& rScreenPos) const
{
    screenPosToCameraPos(pCameraPos, {rScreenPos.x, rScreenPos.y, 0.0f});
}

void Projection::project(Vector2f* pDst, const Vector3f& rCameraPos,
                         const Viewport& rViewport) const
{
    Vector3f screen_pos;
    cameraPosToScreenPos(&screen_pos, rCameraPos);
    rViewport.project(pDst, screen_pos);
}

void Projection::unproject(Vector3f* pWorldPos, const Vector3f& rScreenPos,
                           const Camera& rCamera) const
{
    Vector3f camera_pos;
    screenPosToCameraPos(&camera_pos, rScreenPos);
    rCamera.cameraPosToWorldPosByMatrix(pWorldPos, camera_pos);
}

void Projection::unprojectRay(Ray<Vector3f>* pDst, const Vector3f& rScreenPos,
                              const Camera& rCamera) const
{
    Vector3f camera_pos;
    screenPosToCameraPos(&camera_pos, rScreenPos);
    rCamera.unprojectRayByMatrix(pDst, camera_pos);
}

}  // namespace sead
