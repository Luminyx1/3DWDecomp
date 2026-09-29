#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace sead {
class Projection;
}

namespace agl::cull {

class ViewFrustumCulling {
public:
    ViewFrustumCulling();
    ViewFrustumCulling& operator=(const ViewFrustumCulling& rOther);

    void update(const sead::Matrix34f& rViewMtx, const sead::Projection& rProjection);
    void update(const sead::Matrix34f& rViewMtx);
    void update(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx, f32 near,
                f32 far, f32 fovy, f32 aspect, const sead::Vector2f& rOffset);
    void update(const sead::Matrix44f& rProjMtx, f32 near, f32 far, f32 fovy, f32 aspect,
                const sead::Vector2f& rOffset);
    void update(const sead::Projection& rProjection);
    bool isInside(const sead::Vector3f& rPos, f32 radius) const;
    bool isInside(const sead::Vector3f& rMin, const sead::Vector3f& rMax) const;

    const sead::Matrix34f& getViewMtx() const { return mViewMtx; }
    const sead::Matrix34f& getViewInvMtx() const { return mViewInvMtx; }
    const sead::Matrix44f& getProjMtx() const { return mProjMtx; }
    f32 getNear() const { return mNear; }
    f32 getFar() const { return mFar; }

    sead::Matrix34f mViewMtx;
    sead::Matrix34f mViewInvMtx;
    sead::Matrix44f mProjMtx;
    sead::Matrix44f mProjInvMtx;
    f32 mNear;
    f32 mFar;
    f32 mFovy;
    f32 mAspect;
    f32 _f0;
    f32 mTanHalfFovy;
    f32 _f8;
    sead::Vector2f mOffset;
    u8 _104[0x238 - 0x104];

private:
    void update_();
};
static_assert(sizeof(ViewFrustumCulling) == 0x238);

}  // namespace agl::cull
