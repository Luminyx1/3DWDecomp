#include "gfx/seadCamera.h"
#include "basis/seadRawPrint.h"
#include "gfx/seadProjection.h"
#include "gfx/seadViewport.h"
#include "math/seadGeometry.h"
#include "math/seadMathCalcCommon.h"

namespace sead
{
Camera::~Camera() = default;

/**
 * Computes the camera position in world space from the view matrix.
 * @param pDst Receives the world position.
 */
void Camera::getWorldPosByMatrix(Vector3f* pDst) const
{
    const Matrix34f& m = mMatrix;
    f32 x = -m(0, 0) * m(0, 3) - m(1, 0) * m(1, 3) - m(2, 0) * m(2, 3);
    f32 y = -m(0, 3) * m(0, 1) - m(1, 3) * m(1, 1) - m(2, 3) * m(2, 1);
    f32 z = -m(0, 3) * m(0, 2) - m(1, 3) * m(1, 2) - m(2, 3) * m(2, 2);
    pDst->set(x, y, z);
}

/**
 * Gets the look vector (third matrix row).
 * @param pDst Receives the look vector.
 */
void Camera::getLookVectorByMatrix(Vector3f* pDst) const
{
    pDst->set(mMatrix(2, 0), mMatrix(2, 1), mMatrix(2, 2));
}

/**
 * Gets the right vector (first matrix row).
 * @param pDst Receives the right vector.
 */
void Camera::getRightVectorByMatrix(Vector3f* pDst) const
{
    pDst->set(mMatrix(0, 0), mMatrix(0, 1), mMatrix(0, 2));
}

/**
 * Gets the up vector (second matrix row).
 * @param pDst Receives the up vector.
 */
void Camera::getUpVectorByMatrix(Vector3f* pDst) const
{
    pDst->set(mMatrix(1, 0), mMatrix(1, 1), mMatrix(1, 2));
}

/**
 * Transforms a world position into camera space.
 * @param pDst Receives the camera-space position.
 * @param rWorldPos World-space position.
 */
void Camera::worldPosToCameraPosByMatrix(Vector3f* pDst, const Vector3f& rWorldPos) const
{
    pDst->setMul(mMatrix, rWorldPos);
}

void Camera::cameraPosToWorldPosByMatrix(Vector3f* pDst, const Vector3f& rCameraPos) const
{
    Vector3f right;
    getRightVectorByMatrix(&right);
    Vector3f look;
    getLookVectorByMatrix(&look);
    Vector3f up;
    getUpVectorByMatrix(&up);
    up *= rCameraPos.y;
    look *= rCameraPos.z;
    right *= rCameraPos.x;
    Vector3f pos;
    getWorldPosByMatrix(&pos);
    *pDst = right + (look + (up + pos));
}

/**
 * Projects a world position onto the viewport.
 * @param pDst Receives the viewport position.
 * @param rWorldPos World-space position.
 * @param rProjection Projection to use.
 * @param rViewport Viewport to project onto.
 */
void Camera::projectByMatrix(Vector2f* pDst, const Vector3f& rWorldPos,
                             const Projection& rProjection, const Viewport& rViewport) const
{
    Vector3f camera_pos;
    worldPosToCameraPosByMatrix(&camera_pos, rWorldPos);
    rProjection.project(pDst, camera_pos, rViewport);
}

/**
 * Builds a world-space ray from the camera through a camera-space position.
 * @param pDst Receives the ray.
 * @param rCameraPos Camera-space position.
 */
void Camera::unprojectRayByMatrix(Ray<Vector3f>* pDst, const Vector3f& rCameraPos) const
{
    Vector3f up;
    getUpVectorByMatrix(&up);
    Vector3f right;
    getRightVectorByMatrix(&right);
    Vector3f look;
    getLookVectorByMatrix(&look);
    up *= rCameraPos.y;
    look *= rCameraPos.z;
    right *= rCameraPos.x;
    Vector3f dir = up + look + right;
    dir.normalize();

    Vector3f pos;
    getWorldPosByMatrix(&pos);
    pDst->setPosDir(pos, dir);
}

LookAtCamera::~LookAtCamera() = default;

/**
 * Constructs a look-at camera and normalizes the up vector.
 * @param rPos Camera position.
 * @param rAt Look-at target.
 * @param rUp Up vector.
 */
LookAtCamera::LookAtCamera(const Vector3f& rPos, const Vector3f& rAt, const Vector3f& rUp)
    : mPos(rPos), mAt(rAt), mUp(rUp)
{
    SEAD_ASSERT(mPos != mAt);
    mUp.normalize();
}

/**
 * Builds the view matrix from position, target and up vector.
 * @param pDst Receives the view matrix.
 */
void LookAtCamera::doUpdateMatrix(Matrix34f* pDst) const
{
    if (mPos == mAt)
    {
        return;
    }

    Vector3f dir = mPos;
    dir -= mAt;
    dir.normalize();

    Vector3f right;
    right.setCross(mUp, dir);
    right.normalize();

    Vector3f up;
    up.setCross(dir, right);

    f32 tx = -right.dot(mPos);
    f32 ty = -up.dot(mPos);
    f32 tz = -dir.dot(mPos);

    pDst->m[0][0] = right.x;
    pDst->m[0][1] = right.y;
    pDst->m[0][2] = right.z;
    pDst->m[0][3] = tx;

    pDst->m[1][0] = up.x;
    pDst->m[1][1] = up.y;
    pDst->m[1][2] = up.z;
    pDst->m[1][3] = ty;

    pDst->m[2][0] = dir.x;
    pDst->m[2][1] = dir.y;
    pDst->m[2][2] = dir.z;
    pDst->m[2][3] = tz;
}

/**
 * Constructs an ortho camera looking down -Z from (0, 0, 1).
 */
OrthoCamera::OrthoCamera()
    : LookAtCamera(Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0.0f, 0.0f, 0.0f),
                   Vector3f(0.0f, 1.0f, 0.0f))
{
}

/**
 * Constructs an ortho camera at a 2D position and distance.
 * @param rPos XY position.
 * @param distance Z position of the camera.
 */
OrthoCamera::OrthoCamera(const Vector2f& rPos, f32 distance)
    : LookAtCamera(Vector3f(rPos.x, rPos.y, distance), Vector3f(rPos.x, rPos.y, distance - 1.0f),
                   Vector3f(0.0f, 1.0f, 0.0f))
{
}

/**
 * Constructs an ortho camera centered on an ortho projection.
 * @param rProjection Projection to center on.
 */
OrthoCamera::OrthoCamera(const OrthoProjection& rProjection)
    : LookAtCamera(Vector3f((rProjection.getLeft() + rProjection.getRight()) * 0.5f,
                            (rProjection.getTop() + rProjection.getBottom()) * 0.5f,
                            rProjection.getNear()),
                   Vector3f((rProjection.getLeft() + rProjection.getRight()) * 0.5f,
                            (rProjection.getTop() + rProjection.getBottom()) * 0.5f,
                            rProjection.getNear() - 1.0f),
                   Vector3f(0.0f, 1.0f, 0.0f))
{
}

OrthoCamera::~OrthoCamera() = default;

/**
 * Centers the camera on an ortho projection.
 * @param rProjection Projection to center on.
 */
void OrthoCamera::setByOrthoProjection(const OrthoProjection& rProjection)
{
    setPos(Vector3f((rProjection.getLeft() + rProjection.getRight()) * 0.5f,
                    (rProjection.getTop() + rProjection.getBottom()) * 0.5f,
                    rProjection.getNear()));
    setAt(Vector3f((rProjection.getLeft() + rProjection.getRight()) * 0.5f,
                   (rProjection.getTop() + rProjection.getBottom()) * 0.5f,
                   rProjection.getNear() - 1.0f));
    setUp(Vector3f(0.0f, 1.0f, 0.0f));
    normalizeUp();
}

/**
 * Sets the camera roll by rotating the up vector.
 * @param rotation Roll angle in radians.
 */
void OrthoCamera::setRotation(f32 rotation)
{
    f32 angle = rotation - Mathf::piHalf();
    setUp(Vector3f(Mathf::cos(angle), -Mathf::sin(angle), 0.0f));
    normalizeUp();
}

DirectCamera::~DirectCamera() = default;

}  // namespace sead
