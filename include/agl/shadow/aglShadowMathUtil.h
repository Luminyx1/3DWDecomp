#pragma once

#include <arm_neon.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace agl::sdw::detail
{

inline void multiplyMtx44(sead::Matrix44f& rOut, const sead::Matrix44f& rA,
                          const sead::Matrix44f& rB)
{
    const float32x4_t a0 = vld1q_f32(rA.m[0]);
    const float32x4_t a1 = vld1q_f32(rA.m[1]);
    const float32x4_t a2 = vld1q_f32(rA.m[2]);
    const float32x4_t a3 = vld1q_f32(rA.m[3]);

    const float32x4_t b0 = vld1q_f32(rB.m[0]);
    const float32x4_t b1 = vld1q_f32(rB.m[1]);
    const float32x4_t b2 = vld1q_f32(rB.m[2]);
    const float32x4_t b3 = vld1q_f32(rB.m[3]);

    float32x4_t c0 = vmulq_laneq_f32(b0, a0, 0);
    c0 = vfmaq_laneq_f32(c0, b1, a0, 1);
    c0 = vfmaq_laneq_f32(c0, b2, a0, 2);
    c0 = vfmaq_laneq_f32(c0, b3, a0, 3);

    float32x4_t c1 = vmulq_laneq_f32(b0, a1, 0);
    c1 = vfmaq_laneq_f32(c1, b1, a1, 1);
    c1 = vfmaq_laneq_f32(c1, b2, a1, 2);
    c1 = vfmaq_laneq_f32(c1, b3, a1, 3);

    float32x4_t c2 = vmulq_laneq_f32(b0, a2, 0);
    c2 = vfmaq_laneq_f32(c2, b1, a2, 1);
    c2 = vfmaq_laneq_f32(c2, b2, a2, 2);
    c2 = vfmaq_laneq_f32(c2, b3, a2, 3);

    float32x4_t c3 = vmulq_laneq_f32(b0, a3, 0);
    c3 = vfmaq_laneq_f32(c3, b1, a3, 1);
    c3 = vfmaq_laneq_f32(c3, b2, a3, 2);
    c3 = vfmaq_laneq_f32(c3, b3, a3, 3);

    vst1q_f32(rOut.m[0], c0);
    vst1q_f32(rOut.m[1], c1);
    vst1q_f32(rOut.m[2], c2);
    vst1q_f32(rOut.m[3], c3);
}

inline void transformProj(sead::Vector3f* pOut, const sead::Matrix44f& rMtx,
                          const sead::Vector3f& rIn)
{
    const sead::Vector3f p = rIn;
    const f32 w = rMtx(3, 0) * p.x + rMtx(3, 1) * p.y + rMtx(3, 2) * p.z + rMtx(3, 3);
    pOut->x = rMtx(0, 0) / w * p.x + rMtx(0, 1) / w * p.y + rMtx(0, 2) / w * p.z + rMtx(0, 3) / w;
    pOut->y = rMtx(1, 0) / w * p.x + rMtx(1, 1) / w * p.y + rMtx(1, 2) / w * p.z + rMtx(1, 3) / w;
    pOut->z = rMtx(2, 0) / w * p.x + rMtx(2, 1) / w * p.y + rMtx(2, 2) / w * p.z + rMtx(2, 3) / w;
}

inline void transformProj(sead::Vector3f* pPoint, const sead::Matrix44f& rMtx)
{
    transformProj(pPoint, rMtx, *pPoint);
}

inline void transform(sead::Vector3f* pPoint, const sead::Matrix34f& rMtx)
{
    const sead::Vector3f p = *pPoint;
    pPoint->x = rMtx(0, 0) * p.x + rMtx(0, 1) * p.y + rMtx(0, 2) * p.z + rMtx(0, 3);
    pPoint->y = rMtx(1, 0) * p.x + rMtx(1, 1) * p.y + rMtx(1, 2) * p.z + rMtx(1, 3);
    pPoint->z = rMtx(2, 0) * p.x + rMtx(2, 1) * p.y + rMtx(2, 2) * p.z + rMtx(2, 3);
}

template <typename Box>
inline void setBoxCorners(sead::Vector3f* pPoints, const Box& rBox)
{
    const sead::Vector3f& min = rBox.getMin();
    const sead::Vector3f& max = rBox.getMax();
    pPoints[0].set(min.x, min.y, min.z);
    pPoints[1].set(max.x, min.y, min.z);
    pPoints[2].set(max.x, min.y, max.z);
    pPoints[3].set(min.x, min.y, max.z);
    pPoints[4].set(min.x, max.y, min.z);
    pPoints[5].set(max.x, max.y, min.z);
    pPoints[6].set(max.x, max.y, max.z);
    pPoints[7].set(min.x, max.y, max.z);
}

}  // namespace agl::sdw::detail
