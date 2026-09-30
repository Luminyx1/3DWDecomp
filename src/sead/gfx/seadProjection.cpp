#include <gfx/seadProjection.h>

#include <gfx/seadCamera.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrixCalcCommon.h>

namespace sead
{
/**
 * Constructs a projection with the default device posture and depth transform.
 */
Projection::Projection()
{
    mDevicePosture = Graphics::sDefaultDevicePosture;
    mDeviceZScale = Graphics::sDefaultDeviceZScale;
    mDeviceZOffset = Graphics::sDefaultDeviceZOffset;
}

Projection::~Projection() = default;

/**
 * Returns the projection matrix, updating it first if needed.
 * @return The projection matrix.
 */
const Matrix44f& Projection::getProjectionMatrix() const
{
    updateMatrixImpl_();
    return mMatrix;
}

/**
 * Recomputes the projection and device matrices if they are dirty.
 */
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

/**
 * Returns a mutable pointer to the projection matrix, updating it first if needed.
 * @return The projection matrix.
 */
Matrix44f* Projection::getProjectionMatrixMutable()
{
    updateMatrixImpl_();
    return &mMatrix;
}

/**
 * Returns the device projection matrix, updating it first if needed.
 * @return The device projection matrix.
 */
const Matrix44f& Projection::getDeviceProjectionMatrix() const
{
    updateMatrixImpl_();
    return mDeviceMatrix;
}

/**
 * Transforms a camera-space position into screen space.
 * @param pScreenPos Receives the screen position.
 * @param rCameraPos Camera-space position.
 */
void Projection::cameraPosToScreenPos(Vector3f* pScreenPos, const Vector3f& rCameraPos) const
{
    pScreenPos->setMul(getProjectionMatrix(), rCameraPos);
}

/**
 * Transforms a screen position into camera space.
 * @param pCameraPos Receives the camera-space position.
 * @param rScreenPos Screen position.
 */
void Projection::screenPosToCameraPos(Vector3f* pCameraPos, const Vector3f& rScreenPos) const
{
    doScreenPosToCameraPosTo(pCameraPos, rScreenPos);
}

/**
 * Transforms a 2D screen position on the near plane into camera space.
 * @param pCameraPos Receives the camera-space position.
 * @param rScreenPos Screen position.
 */
void Projection::screenPosToCameraPos(Vector3f* pCameraPos, const Vector2f& rScreenPos) const
{
    screenPosToCameraPos(pCameraPos, {rScreenPos.x, rScreenPos.y, 0.0f});
}

/**
 * Updates attributes derived from a direct matrix; does nothing by default.
 */
void Projection::updateAttributesForDirectProjection() {}

/**
 * Projects a camera-space position onto a viewport.
 * @param pDst Receives the viewport position.
 * @param rCameraPos Camera-space position.
 * @param rViewport Viewport to project onto.
 */
void Projection::project(Vector2f* pDst, const Vector3f& rCameraPos,
                         const Viewport& rViewport) const
{
    Vector3f screen_pos;
    cameraPosToScreenPos(&screen_pos, rCameraPos);
    rViewport.project(pDst, screen_pos);
}

/**
 * Converts a screen position into a world position.
 * @param pWorldPos Receives the world position.
 * @param rScreenPos Screen position.
 * @param rCamera Camera to use.
 */
void Projection::unproject(Vector3f* pWorldPos, const Vector3f& rScreenPos,
                           const Camera& rCamera) const
{
    Vector3f camera_pos;
    screenPosToCameraPos(&camera_pos, rScreenPos);
    rCamera.cameraPosToWorldPosByMatrix(pWorldPos, camera_pos);
}

/**
 * Converts a screen position into a world-space ray.
 * @param pDst Receives the ray.
 * @param rScreenPos Screen position.
 * @param rCamera Camera to use.
 */
void Projection::unprojectRay(Ray<Vector3f>* pDst, const Vector3f& rScreenPos,
                              const Camera& rCamera) const
{
    Vector3f camera_pos;
    screenPosToCameraPos(&camera_pos, rScreenPos);
    rCamera.unprojectRayByMatrix(pDst, camera_pos);
}

void Projection::doUpdateDeviceMatrix(Matrix44f* pDst, const Matrix44f& rSrc,
                                      Graphics::DevicePosture posture) const
{
    *pDst = rSrc;

    switch (posture)
    {
    case Graphics::cDevicePosture_Same:
        break;
    case Graphics::cDevicePosture_RotateHalfAround:
        pDst->m[0][0] = -pDst->m[0][0];
        pDst->m[0][1] = -pDst->m[0][1];
        pDst->m[0][2] = -pDst->m[0][2];
        pDst->m[0][3] = -pDst->m[0][3];
        pDst->m[1][0] = -pDst->m[1][0];
        pDst->m[1][1] = -pDst->m[1][1];
        pDst->m[1][2] = -pDst->m[1][2];
        pDst->m[1][3] = -pDst->m[1][3];
        break;
    case Graphics::cDevicePosture_FlipX:
        pDst->m[0][0] = -pDst->m[0][0];
        pDst->m[0][1] = -pDst->m[0][1];
        pDst->m[0][2] = -pDst->m[0][2];
        pDst->m[0][3] = -pDst->m[0][3];
        break;
    case Graphics::cDevicePosture_FlipY:
        pDst->m[1][0] = -pDst->m[1][0];
        pDst->m[1][1] = -pDst->m[1][1];
        pDst->m[1][2] = -pDst->m[1][2];
        pDst->m[1][3] = -pDst->m[1][3];
        break;
    case Graphics::cDevicePosture_RotateLeft:
    {
        f32 t0 = pDst->m[0][0];
        f32 t1 = pDst->m[0][1];
        f32 t2 = pDst->m[0][2];
        f32 t3 = pDst->m[0][3];
        pDst->m[0][0] = -pDst->m[1][0];
        pDst->m[0][1] = -pDst->m[1][1];
        pDst->m[0][2] = -pDst->m[1][2];
        pDst->m[0][3] = -pDst->m[1][3];
        pDst->m[1][0] = t0;
        pDst->m[1][1] = t1;
        pDst->m[1][2] = t2;
        pDst->m[1][3] = t3;
        break;
    }
    case Graphics::cDevicePosture_RotateRight:
    {
        f32 t0 = pDst->m[0][0];
        f32 t1 = pDst->m[0][1];
        f32 t2 = pDst->m[0][2];
        f32 t3 = pDst->m[0][3];
        pDst->m[0][0] = pDst->m[1][0];
        pDst->m[0][1] = pDst->m[1][1];
        pDst->m[0][2] = pDst->m[1][2];
        pDst->m[0][3] = pDst->m[1][3];
        pDst->m[1][0] = -t0;
        pDst->m[1][1] = -t1;
        pDst->m[1][2] = -t2;
        pDst->m[1][3] = -t3;
        break;
    }
    default:
        break;
    }

    pDst->m[2][0] = mDeviceZScale * pDst->m[2][0];
    pDst->m[2][1] = pDst->m[2][1] * mDeviceZScale;
    pDst->m[2][2] = mDeviceZScale * (pDst->m[2][2] + pDst->m[3][2] * mDeviceZOffset);
    pDst->m[2][3] = pDst->m[2][3] * mDeviceZScale + pDst->m[3][3] * mDeviceZOffset;
}

/**
 * Constructs a 45 degree 4:3 perspective projection.
 */
PerspectiveProjection::PerspectiveProjection()
    : mNear(1.0f), mFar(10000.0f), mAspect(4.0f / 3.0f), mOffset(Vector2f::zero)
{
    setFovy_(numbers::pi_v<f32> / 4);
}

/**
 * Sets the vertical field of view and caches its half-angle sine, cosine and tangent.
 * @param fovy Vertical field of view in radians.
 */
void PerspectiveProjection::setFovy_(f32 fovy)
{
    mFovyRad = fovy;
    f32 half = fovy * 0.5f;
    mFovySin = Mathf::sin(half);
    mFovyCos = Mathf::cos(half);
    mFovyTan = Mathf::tan(half);
    setDirty();
}

/**
 * Constructs a perspective projection.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param fovy Vertical field of view in radians.
 * @param aspect Aspect ratio.
 */
PerspectiveProjection::PerspectiveProjection(f32 near, f32 far, f32 fovy, f32 aspect)
    : mNear(near), mFar(far), mAspect(aspect), mOffset(Vector2f::zero)
{
    setFovy_(fovy);
}

PerspectiveProjection::~PerspectiveProjection() = default;

/**
 * Sets all perspective parameters.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param fovy Vertical field of view in radians.
 * @param aspect Aspect ratio.
 */
void PerspectiveProjection::set(f32 near, f32 far, f32 fovy, f32 aspect)
{
    setNear(near);
    setFar(far);
    setFovy_(fovy);
    setAspect(aspect);
}

/**
 * Returns the near clip distance.
 * @return The near clip distance.
 */
f32 PerspectiveProjection::getNear() const
{
    return mNear;
}

/**
 * Returns the far clip distance.
 * @return The far clip distance.
 */
f32 PerspectiveProjection::getFar() const
{
    return mFar;
}

/**
 * Returns the vertical field of view.
 * @return The vertical field of view in radians.
 */
f32 PerspectiveProjection::getFovy() const
{
    return mFovyRad;
}

/**
 * Returns the aspect ratio.
 * @return The aspect ratio.
 */
f32 PerspectiveProjection::getAspect() const
{
    return mAspect;
}

/**
 * Gets the projection center offset.
 * @param pOffset Receives the offset.
 */
void PerspectiveProjection::getOffset(Vector2f* pOffset) const
{
    pOffset->x = mOffset.x;
    pOffset->y = mOffset.y;
}

/**
 * Builds the perspective projection matrix.
 * @param pDst Receives the matrix.
 */
void PerspectiveProjection::doUpdateMatrix(Matrix44f* pDst) const
{
    f32 h = 2 * mNear * mFovyTan;
    f32 w = h * mAspect;
    f32 offset_x = w * mOffset.x;
    f32 left = -(w * 0.5f) + offset_x;
    f32 right = w * 0.5f + offset_x;
    f32 offset_y = h * mOffset.y;
    f32 top = h * 0.5f + offset_y;
    f32 bottom = -(h * 0.5f) + offset_y;

    f32 inv_w = 1.0f / (right - left);
    pDst->m[0][0] = 2 * mNear * inv_w;
    pDst->m[0][1] = 0.0f;
    pDst->m[0][2] = (right + left) * inv_w;
    pDst->m[0][3] = 0.0f;

    f32 inv_h = 1.0f / (top - bottom);
    pDst->m[1][0] = 0.0f;
    pDst->m[1][1] = 2 * mNear * inv_h;
    pDst->m[1][2] = (top + bottom) * inv_h;
    pDst->m[1][3] = 0.0f;

    f32 inv_d = 1.0f / (mFar - mNear);
    pDst->m[2][0] = 0.0f;
    pDst->m[2][1] = 0.0f;
    pDst->m[2][2] = -(mFar + mNear) * inv_d;
    pDst->m[2][3] = -(2 * mFar * mNear) * inv_d;

    pDst->m[3][0] = 0.0f;
    pDst->m[3][1] = 0.0f;
    pDst->m[3][2] = -1.0f;
    pDst->m[3][3] = 0.0f;
}

/**
 * Transforms a screen position onto the near plane in camera space.
 * @param pCameraPos Receives the camera-space position.
 * @param rScreenPos Screen position.
 */
void PerspectiveProjection::doScreenPosToCameraPosTo(Vector3f* pCameraPos,
                                                     const Vector3f& rScreenPos) const
{
    pCameraPos->set(0.0f, 0.0f, -mNear);
    f32 h = 2 * mNear * mFovyTan * 0.5f;
    pCameraPos->y = h * (rScreenPos.y + 2 * mOffset.y);
    f32 w = 2 * mNear * mFovyTan * mAspect * 0.5f;
    pCameraPos->x = w * (rScreenPos.x + 2 * mOffset.x);
}

/**
 * Sets the aspect ratio from a horizontal field of view.
 * @param fovx Horizontal field of view in radians.
 */
void PerspectiveProjection::setFovx(f32 fovx)
{
    setAspect(Mathf::tan(fovx * 0.5f) / mFovyTan);
}

void PerspectiveProjection::createDividedProjection(PerspectiveProjection* pDst, s32 partno_x,
                                                    s32 partno_y, s32 divnum_x,
                                                    s32 divnum_y) const
{
    f32 div_y = divnum_y;
    pDst->mFovyTan = mFovyTan / div_y;
    f32 half = Mathf::atan2(pDst->mFovyTan, 1.0f);
    pDst->mFovySin = Mathf::sin(half);
    pDst->mFovyCos = Mathf::cos(half);
    pDst->mFovyRad = half * 2;
    f32 div_x = divnum_x;
    pDst->mAspect = mAspect * div_y / div_x;
    f32 offset_x = (partno_x + 0.5f) - div_x / 2;
    f32 offset_y = (partno_y + 0.5f) - div_y / 2;
    pDst->setOffset({offset_x + mOffset.x / div_x, mOffset.y / div_y - offset_y});
    pDst->mNear = mNear;
    pDst->mFar = mFar;
    pDst->setDirty();
}

/**
 * Returns the top edge of the near plane.
 * @return The top edge.
 */
f32 PerspectiveProjection::getTop() const
{
    f32 h = 2 * mNear * mFovyTan;
    f32 offset = mOffset.y * h;
    return h * 0.5f + offset;
}

/**
 * Returns the bottom edge of the near plane.
 * @return The bottom edge.
 */
f32 PerspectiveProjection::getBottom() const
{
    f32 h = 2 * mNear * mFovyTan;
    return mOffset.y * h - h * 0.5f;
}

/**
 * Returns the left edge of the near plane.
 * @return The left edge.
 */
f32 PerspectiveProjection::getLeft() const
{
    f32 w = 2 * mNear * mFovyTan * mAspect;
    f32 offset = mOffset.x * w;
    return -(w * 0.5f) + offset;
}

/**
 * Returns the right edge of the near plane.
 * @return The right edge.
 */
f32 PerspectiveProjection::getRight() const
{
    f32 w = 2 * mNear * mFovyTan * mAspect;
    f32 offset = mOffset.x * w;
    return w * 0.5f + offset;
}

/**
 * Sets the near plane edges, deriving field of view, aspect and offset.
 * @param top Top edge.
 * @param bottom Bottom edge.
 * @param left Left edge.
 * @param right Right edge.
 */
void PerspectiveProjection::setTBLR(f32 top, f32 bottom, f32 left, f32 right)
{
    f32 h = top - bottom;
    f32 w = right - left;
    setAspect(w / h);
    setFovy_(Mathf::atan2(h * 0.5f, getNear()) * 2);
    setOffset({(right + left) * 0.5f / w, (top + bottom) * 0.5f / h});
}

/**
 * Constructs a unit orthographic projection.
 */
OrthoProjection::OrthoProjection() : mNear(0.0f), mFar(1.0f)
{
    setTBLR(0.5f, -0.5f, -0.5f, 0.5f);
}

/**
 * Constructs an orthographic projection.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param top Top edge.
 * @param bottom Bottom edge.
 * @param left Left edge.
 * @param right Right edge.
 */
OrthoProjection::OrthoProjection(f32 near, f32 far, f32 top, f32 bottom, f32 left, f32 right)
    : mNear(near), mFar(far)
{
    setTBLR(top, bottom, left, right);
}

/**
 * Constructs an orthographic projection from a bounding box.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param rBoundBox Visible area.
 */
OrthoProjection::OrthoProjection(f32 near, f32 far, const BoundBox2f& rBoundBox)
    : mNear(near), mFar(far)
{
    mTop = rBoundBox.getMax().y;
    mBottom = rBoundBox.getMin().y;
    mLeft = rBoundBox.getMin().x;
    mRight = rBoundBox.getMax().x;
    setDirty();
}

/**
 * Constructs an orthographic projection centered on a viewport.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param rViewport Viewport to cover.
 */
OrthoProjection::OrthoProjection(f32 near, f32 far, const Viewport& rViewport)
    : mNear(near), mFar(far)
{
    mTop = rViewport.getSizeY() * 0.5f;
    mBottom = -rViewport.getSizeY() * 0.5f;
    mLeft = -rViewport.getSizeX() * 0.5f;
    mRight = rViewport.getSizeX() * 0.5f;
    setDevicePosture(rViewport.getDevicePosture());
    setDirty();
}

OrthoProjection::~OrthoProjection() = default;

/**
 * Returns the near clip distance.
 * @return The near clip distance.
 */
f32 OrthoProjection::getNear() const
{
    return mNear;
}

/**
 * Returns the far clip distance.
 * @return The far clip distance.
 */
f32 OrthoProjection::getFar() const
{
    return mFar;
}

/**
 * Returns the vertical field of view, which is zero.
 * @return Zero.
 */
f32 OrthoProjection::getFovy() const
{
    return 0.0f;
}

/**
 * Returns the aspect ratio.
 * @return The aspect ratio.
 */
f32 OrthoProjection::getAspect() const
{
    return (mRight - mLeft) / (mTop - mBottom);
}

/**
 * Gets the projection center offset.
 * @param pOffset Receives the offset.
 */
void OrthoProjection::getOffset(Vector2f* pOffset) const
{
    pOffset->x = (mLeft + mRight) * 0.5f / (mRight - mLeft);
    pOffset->y = (mTop + mBottom) * 0.5f / (mTop - mBottom);
}

void OrthoProjection::setByViewport(const Viewport& rViewport)
{
    f32 w = rViewport.getSizeX();
    f32 h = rViewport.getSizeY();
    setTBLR(h * 0.5f, -h * 0.5f, -w * 0.5f, w * 0.5f);
}

/**
 * Sets the edges of the visible area.
 * @param top Top edge.
 * @param bottom Bottom edge.
 * @param left Left edge.
 * @param right Right edge.
 */
void OrthoProjection::setTBLR(f32 top, f32 bottom, f32 left, f32 right)
{
    mTop = top;
    mBottom = bottom;
    mLeft = left;
    mRight = right;
    setDirty();
}

/**
 * Sets the visible area from a bounding box.
 * @param rBoundBox Visible area.
 */
void OrthoProjection::setBoundBox(const BoundBox2f& rBoundBox)
{
    setTBLR(rBoundBox.getMax().y, rBoundBox.getMin().y, rBoundBox.getMin().x,
            rBoundBox.getMax().x);
}

/**
 * Builds the orthographic projection matrix.
 * @param pDst Receives the matrix.
 */
void OrthoProjection::doUpdateMatrix(Matrix44f* pDst) const
{
    f32 cx = (mLeft + mRight) * 0.5f;
    f32 cy = (mTop + mBottom) * 0.5f;
    f32 w = (mRight - mLeft) * 0.5f;
    pDst->m[0][0] = 1.0f / w;
    pDst->m[0][1] = 0.0f;
    pDst->m[0][2] = 0.0f;
    pDst->m[0][3] = -cx / w;

    f32 h = (mTop - mBottom) * 0.5f;
    pDst->m[1][0] = 0.0f;
    pDst->m[1][1] = 1.0f / h;
    pDst->m[1][2] = 0.0f;
    pDst->m[1][3] = -cy / h;

    f32 inv_d = 1.0f / (mFar - mNear);
    pDst->m[2][0] = 0.0f;
    pDst->m[2][1] = 0.0f;
    pDst->m[2][2] = inv_d * -2.0f;
    pDst->m[2][3] = -(inv_d * (mNear + mFar));

    pDst->m[3][0] = 0.0f;
    pDst->m[3][1] = 0.0f;
    pDst->m[3][2] = 0.0f;
    pDst->m[3][3] = 1.0f;
}

/**
 * Transforms a screen position onto the near plane in camera space.
 * @param pCameraPos Receives the camera-space position.
 * @param rScreenPos Screen position.
 */
void OrthoProjection::doScreenPosToCameraPosTo(Vector3f* pCameraPos,
                                               const Vector3f& rScreenPos) const
{
    f32 w = rScreenPos.x * (mRight - mLeft) * 0.5f;
    pCameraPos->x = (mRight + mLeft) * 0.5f + w;
    f32 h = rScreenPos.y * (mTop - mBottom) * 0.5f;
    pCameraPos->y = (mTop + mBottom) * 0.5f + h;
    pCameraPos->z = -mNear;
}

/**
 * Sets up a projection covering one cell of a grid division of this one.
 * @param pDst Receives the divided projection.
 * @param partno_x Cell column.
 * @param partno_y Cell row.
 * @param divnum_x Number of columns.
 * @param divnum_y Number of rows.
 */
void OrthoProjection::createDividedProjection(OrthoProjection* pDst, s32 partno_x,
                                              s32 partno_y, s32 divnum_x, s32 divnum_y) const
{
    pDst->setLeft(mLeft + (mRight - mLeft) * partno_x / divnum_x);
    pDst->setRight(pDst->mLeft + (mRight - mLeft) / divnum_x);
    pDst->setTop(mTop + (mBottom - mTop) * partno_y / divnum_y);
    pDst->setBottom(pDst->mTop + (mBottom - mTop) / divnum_y);
    pDst->setNear(getNear());
    pDst->setFar(getFar());
}

/**
 * Constructs a default 4:3 frustum projection.
 */
FrustumProjection::FrustumProjection() : mNear(1.0f), mFar(10000.0f)
{
    setTBLR(0.5f, -0.5f, -2.0f / 3.0f, 2.0f / 3.0f);
}

/**
 * Constructs a frustum projection.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param top Top edge.
 * @param bottom Bottom edge.
 * @param left Left edge.
 * @param right Right edge.
 */
FrustumProjection::FrustumProjection(f32 near, f32 far, f32 top, f32 bottom, f32 left,
                                     f32 right)
    : mNear(near), mFar(far)
{
    setTBLR(top, bottom, left, right);
}

/**
 * Constructs a frustum projection from a bounding box on the near plane.
 * @param near Near clip distance.
 * @param far Far clip distance.
 * @param rBoundBox Near plane area.
 */
FrustumProjection::FrustumProjection(f32 near, f32 far, const BoundBox2f& rBoundBox)
    : mNear(near), mFar(far)
{
    mTop = rBoundBox.getMax().y;
    mBottom = rBoundBox.getMin().y;
    mLeft = rBoundBox.getMin().x;
    mRight = rBoundBox.getMax().x;
    setDirty();
}

FrustumProjection::~FrustumProjection() = default;

/**
 * Returns the near clip distance.
 * @return The near clip distance.
 */
f32 FrustumProjection::getNear() const
{
    return mNear;
}

/**
 * Returns the far clip distance.
 * @return The far clip distance.
 */
f32 FrustumProjection::getFar() const
{
    return mFar;
}

/**
 * Builds the frustum projection matrix.
 * @param pDst Receives the matrix.
 */
void FrustumProjection::doUpdateMatrix(Matrix44f* pDst) const
{
    f32 inv_w = 1.0f / (mRight - mLeft);
    pDst->m[0][0] = 2 * mNear * inv_w;
    pDst->m[0][1] = 0.0f;
    pDst->m[0][2] = inv_w * (mLeft + mRight);
    pDst->m[0][3] = 0.0f;

    f32 inv_h = 1.0f / (mTop - mBottom);
    pDst->m[1][0] = 0.0f;
    pDst->m[1][1] = inv_h * (2 * mNear);
    pDst->m[1][2] = inv_h * (mTop + mBottom);
    pDst->m[1][3] = 0.0f;

    f32 inv_d = 1.0f / (mFar - mNear);
    pDst->m[2][0] = 0.0f;
    pDst->m[2][1] = 0.0f;
    pDst->m[2][2] = -(inv_d * (mFar + mNear));
    pDst->m[2][3] = -(inv_d * (2 * mFar * mNear));

    pDst->m[3][0] = 0.0f;
    pDst->m[3][1] = 0.0f;
    pDst->m[3][2] = -1.0f;
    pDst->m[3][3] = 0.0f;
}

/**
 * Transforms a screen position onto the near plane in camera space.
 * @param pCameraPos Receives the camera-space position.
 * @param rScreenPos Screen position.
 */
void FrustumProjection::doScreenPosToCameraPosTo(Vector3f* pCameraPos,
                                                 const Vector3f& rScreenPos) const
{
    pCameraPos->z = -mNear;
    f32 w = (mRight - mLeft) * rScreenPos.x * 0.5f;
    pCameraPos->x = (mRight + mLeft) * 0.5f + w;
    f32 h = (mTop - mBottom) * rScreenPos.y * 0.5f;
    pCameraPos->y = (mTop + mBottom) * 0.5f + h;
}

/**
 * Sets the edges of the near plane.
 * @param top Top edge.
 * @param bottom Bottom edge.
 * @param left Left edge.
 * @param right Right edge.
 */
void FrustumProjection::setTBLR(f32 top, f32 bottom, f32 left, f32 right)
{
    mTop = top;
    mBottom = bottom;
    mLeft = left;
    mRight = right;
    setDirty();
}

/**
 * Sets the near plane edges from a bounding box.
 * @param rBoundBox Near plane area.
 */
void FrustumProjection::setBoundBox(const BoundBox2f& rBoundBox)
{
    setTBLR(rBoundBox.getMax().y, rBoundBox.getMin().y, rBoundBox.getMin().x,
            rBoundBox.getMax().x);
}

/**
 * Sets up a projection covering one cell of a grid division of this one.
 * @param pDst Receives the divided projection.
 * @param partno_x Cell column.
 * @param partno_y Cell row.
 * @param divnum_x Number of columns.
 * @param divnum_y Number of rows.
 */
void FrustumProjection::createDividedProjection(FrustumProjection* pDst, s32 partno_x,
                                                s32 partno_y, s32 divnum_x,
                                                s32 divnum_y) const
{
    f32 w = mRight - mLeft;
    f32 h = mTop - mBottom;
    f32 left = mLeft + w * partno_x / divnum_x;
    f32 right = w / divnum_x + left;
    pDst->mLeft = left;
    pDst->mRight = right;
    pDst->setDirty();
    f32 top = mTop - h * partno_y / divnum_y;
    f32 bottom = top - h / divnum_y;
    pDst->mTop = top;
    pDst->mBottom = bottom;
    pDst->setDirty();
}

/**
 * Returns the vertical field of view.
 * @return The vertical field of view in radians.
 */
f32 FrustumProjection::getFovy() const
{
    return Mathf::atan2((mTop - mBottom) * 0.5f, getNear()) * 2;
}

/**
 * Returns the aspect ratio.
 * @return The aspect ratio.
 */
f32 FrustumProjection::getAspect() const
{
    return (mRight - mLeft) / (mTop - mBottom);
}

/**
 * Gets the projection center offset.
 * @param pOffset Receives the offset.
 */
void FrustumProjection::getOffset(Vector2f* pOffset) const
{
    pOffset->x = getOffsetX();
    pOffset->y = getOffsetY();
}

/**
 * Returns the horizontal center offset.
 * @return The horizontal offset.
 */
f32 FrustumProjection::getOffsetX() const
{
    f32 w = mRight - mLeft;
    return (mRight + mLeft) * 0.5f / w;
}

/**
 * Returns the vertical center offset.
 * @return The vertical offset.
 */
f32 FrustumProjection::getOffsetY() const
{
    f32 h = mTop - mBottom;
    return (mTop + mBottom) * 0.5f / h;
}

/**
 * Sets the near plane edges from a field of view, aspect ratio and offset.
 * @param fovy Vertical field of view in radians.
 * @param aspect Aspect ratio.
 * @param rOffset Center offset.
 */
void FrustumProjection::setFovyAspectOffset(f32 fovy, f32 aspect, const Vector2f& rOffset)
{
    f32 h = 2 * Mathf::tan(fovy * 0.5f) * getNear();
    f32 w = h * aspect;
    setTop(h * 0.5f + rOffset.y * h);
    setBottom(h * rOffset.y - h * 0.5f);
    setLeft(-(w * 0.5f) + w * rOffset.x);
    setRight(w * 0.5f + w * rOffset.x);
}

/**
 * Constructs a direct projection with an identity matrix.
 */
DirectProjection::DirectProjection()
    : mProjectionMatrix(Matrix44f::ident), mNear(0.0f), mFar(0.0f), mFovy(0.0f), mAspect(0.0f),
      mOffset(0.0f, 0.0f), _f0(true)
{
    setDirty();
}

DirectProjection::DirectProjection(const Matrix44f& rMtx, Graphics::DevicePosture posture)
    : mNear(0.0f), mFar(0.0f), mFovy(0.0f), mAspect(0.0f), mOffset(0.0f, 0.0f), _f0(true)
{
    setProjectionMatrix(rMtx, posture);
}

void DirectProjection::setProjectionMatrix(const Matrix44f& rMtx,
                                           Graphics::DevicePosture posture)
{
    mProjectionMatrix = rMtx;
    Matrix44f& m = mProjectionMatrix;

    switch (posture)
    {
    case Graphics::cDevicePosture_Same:
        break;
    case Graphics::cDevicePosture_RotateLeft:
    {
        f32 m00 = m.m[0][0];
        m.m[0][0] = m.m[1][0];
        f32 m01 = m.m[0][1];
        m.m[0][1] = m.m[1][1];
        f32 m02 = m.m[0][2];
        m.m[0][2] = m.m[1][2];
        f32 m03 = m.m[0][3];
        m.m[0][3] = m.m[1][3];
        m.m[1][0] = -m00;
        m.m[1][1] = -m01;
        m.m[1][2] = -m02;
        m.m[1][3] = -m03;
        break;
    }
    case Graphics::cDevicePosture_RotateRight:
    {
        f32 m10 = m.m[1][0];
        f32 m11 = m.m[1][1];
        f32 m12 = m.m[1][2];
        f32 m13 = m.m[1][3];
        m.m[1][0] = m.m[0][0];
        m.m[1][1] = m.m[0][1];
        m.m[1][2] = m.m[0][2];
        m.m[1][3] = m.m[0][3];
        m.m[0][0] = -m10;
        m.m[0][1] = -m11;
        m.m[0][2] = -m12;
        m.m[0][3] = -m13;
        break;
    }
    case Graphics::cDevicePosture_RotateHalfAround:
        m.m[0][0] = -m.m[0][0];
        m.m[0][1] = -m.m[0][1];
        m.m[0][2] = -m.m[0][2];
        m.m[0][3] = -m.m[0][3];
        m.m[1][0] = -m.m[1][0];
        m.m[1][1] = -m.m[1][1];
        m.m[1][2] = -m.m[1][2];
        m.m[1][3] = -m.m[1][3];
        break;
    case Graphics::cDevicePosture_FlipX:
        m.m[0][0] = -m.m[0][0];
        m.m[0][1] = -m.m[0][1];
        m.m[0][2] = -m.m[0][2];
        m.m[0][3] = -m.m[0][3];
        break;
    case Graphics::cDevicePosture_FlipY:
        m.m[1][0] = -m.m[1][0];
        m.m[1][1] = -m.m[1][1];
        m.m[1][2] = -m.m[1][2];
        m.m[1][3] = -m.m[1][3];
        break;
    default:
        break;
    }

    setDirty();
    _f0 = true;
}

DirectProjection::~DirectProjection() = default;

/**
 * Returns the near clip distance.
 * @return The near clip distance.
 */
f32 DirectProjection::getNear() const
{
    return mNear;
}

/**
 * Returns the far clip distance.
 * @return The far clip distance.
 */
f32 DirectProjection::getFar() const
{
    return mFar;
}

/**
 * Returns the vertical field of view.
 * @return The vertical field of view in radians.
 */
f32 DirectProjection::getFovy() const
{
    return mFovy;
}

/**
 * Returns the aspect ratio.
 * @return The aspect ratio.
 */
f32 DirectProjection::getAspect() const
{
    return mAspect;
}

/**
 * Gets the projection center offset.
 * @param pOffset Receives the offset.
 */
void DirectProjection::getOffset(Vector2f* pOffset) const
{
    pOffset->x = mOffset.x;
    pOffset->y = mOffset.y;
}

void DirectProjection::updateAttributesForDirectProjection()
{
    if (!_f0)
    {
        return;
    }

    Matrix44f inv;
    Matrix44CalcCommon<f32>::inverse(inv, mProjectionMatrix);

    const Vector4f cCorners[8] = {{-1.0f, -1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f, 1.0f},
                                  {1.0f, 1.0f, -1.0f, 1.0f},   {1.0f, -1.0f, -1.0f, 1.0f},
                                  {-1.0f, -1.0f, 1.0f, 1.0f},  {-1.0f, 1.0f, 1.0f, 1.0f},
                                  {1.0f, 1.0f, 1.0f, 1.0f},    {1.0f, -1.0f, 1.0f, 1.0f}};
    Vector3f corners[8];

    for (s32 i = 0; i < 8; i++)
    {
        const Vector4f& c = cCorners[i];
        f32 x = c.x * inv(0, 0) + c.y * inv(0, 1) + c.z * inv(0, 2) + c.w * inv(0, 3);
        f32 y = c.x * inv(1, 0) + c.y * inv(1, 1) + c.z * inv(1, 2) + c.w * inv(1, 3);
        f32 z = c.x * inv(2, 0) + c.y * inv(2, 1) + c.z * inv(2, 2) + c.w * inv(2, 3);
        f32 w = c.x * inv(3, 0) + c.y * inv(3, 1) + c.z * inv(3, 2) + c.w * inv(3, 3);
        f32 inv_w = 1.0f / w;
        corners[i].set(x * inv_w, y * inv_w, z * inv_w);
    }

    mNear = -corners[0].z;
    mFar = -corners[4].z;

    f32 h = corners[1].y - corners[0].y;
    f32 w = corners[2].x - corners[0].x;
    mAspect = w / h;
    mOffset.x = (corners[2].x + corners[0].x) * 0.5f / w;
    mOffset.y = (corners[1].y + corners[0].y) * 0.5f / h;

    auto abs = [](f32 x) { return x > 0 ? x : -x; };

    if (abs(corners[0].x - corners[4].x) > 0.0001f || abs(corners[1].x - corners[5].x) > 0.0001f ||
        abs(corners[2].x - corners[6].x) > 0.0001f || abs(corners[3].x - corners[7].x) > 0.0001f)
    {
        mFovy = Mathf::atan2(h * 0.5f, mNear) * 2;
    }
    else
    {
        mFovy = 0.0f;
    }

    _f0 = false;
}

/**
 * Copies the stored projection matrix.
 * @param pDst Receives the matrix.
 */
void DirectProjection::doUpdateMatrix(Matrix44f* pDst) const
{
    *pDst = mProjectionMatrix;
}

/**
 * Transforms a screen position into camera space with the inverse matrix.
 * @param pCameraPos Receives the camera-space position.
 * @param rScreenPos Screen position.
 */
void DirectProjection::doScreenPosToCameraPosTo(Vector3f* pCameraPos,
                                                const Vector3f& rScreenPos) const
{
    Matrix44f inv;
    Matrix44CalcCommon<f32>::inverse(inv, mProjectionMatrix);
    pCameraPos->setMul(inv, rScreenPos);
}

}  // namespace sead
