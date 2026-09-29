#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace agl::sdw
{

class LightMatrix
{
public:
    LightMatrix();
    explicit LightMatrix(const sead::Vector3f& rDir);
    LightMatrix(const sead::Vector3f& rPos, const sead::Vector3f& rDir, const sead::Vector3f& rUp,
                f32 near, f32 far, f32 fovy, f32 aspect);
    LightMatrix(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx);

    void update(const sead::Vector3f& rDir);
    void update(const sead::Vector3f& rPos, const sead::Vector3f& rDir, const sead::Vector3f& rUp,
                f32 near, f32 far, f32 fovy, f32 aspect);
    void update(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx);

    void calcLightSpace(sead::Matrix34f* pViewMtx, sead::Matrix44f* pProjMtx,
                        const sead::Matrix34f& rCameraViewMtx) const;
    void calcLightSpace(sead::Matrix34f* pViewMtx, sead::Matrix44f* pProjMtx,
                        const sead::Vector3f& rUp) const;

    const sead::Matrix34f& getViewMatrix() const { return mViewMtx; }
    const sead::Matrix44f& getProjectionMatrix() const { return mProjMtx; }
    bool isDirectional() const { return mIsDirectional; }

private:
    sead::Matrix34f mViewMtx;
    sead::Matrix44f mProjMtx;
    bool mIsDirectional;
};
static_assert(sizeof(LightMatrix) == 0x74);

}  // namespace agl::sdw
