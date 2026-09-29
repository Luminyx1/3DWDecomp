#pragma once

#include <nn/util/util_MathTypes.h>

namespace nn::util {
namespace detail {
inline float32x4x4_t Matrix4x4fTranspose(float32x4x4_t matrix) {
    float32x4x2_t tmp0 = vzipq_f32(matrix.val[0], matrix.val[2]);
    float32x4x2_t tmp1 = vzipq_f32(matrix.val[1], matrix.val[3]);
    float32x4x2_t transpose0 = vzipq_f32(tmp0.val[0], tmp1.val[0]);
    float32x4x2_t transpose1 = vzipq_f32(tmp0.val[1], tmp1.val[1]);

    float32x4x4_t result;
    result.val[0] = transpose0.val[0];
    result.val[1] = transpose0.val[1];
    result.val[2] = transpose1.val[0];
    result.val[3] = transpose1.val[1];
    return result;
}
}  // namespace detail

inline void MatrixLoad(Matrix4x3fType* pOutValue, const FloatColumnMajor4x3& rSource) {
    float32x4x4_t tmp;
    tmp.val[0] = vld1q_f32(rSource.m[0]);
    tmp.val[1] = vld1q_f32(rSource.m[1]);
    tmp.val[2] = vld1q_f32(rSource.m[2]);
    tmp.val[3] = vdupq_n_f32(0);
    pOutValue->_m = detail::Matrix4x4fTranspose(tmp);
}

inline void MatrixStore(FloatColumnMajor4x3* pOutValue, const Matrix4x3fType& rSource) {
    float32x4x4_t transposed = detail::Matrix4x4fTranspose(rSource._m);
    vst1q_f32(pOutValue->m[0], transposed.val[0]);
    vst1q_f32(pOutValue->m[1], transposed.val[1]);
    vst1q_f32(pOutValue->m[2], transposed.val[2]);
}
}  // namespace nn::util
