#pragma once

#include <nn/util/util_MathTypes.h>

namespace nn {
namespace util {
inline Float2 MakeFloat2(float x, float y) {
    Float2 vec = {x, y};
    return vec;
}

inline Float3 MakeFloat3(float x, float y, float z) {
    Float3 vec = {x, y, z};
    return vec;
}

inline Float4 MakeFloat4(float x, float y, float z, float w) {
    Float4 vec = {x, y, z, w};
    return vec;
}

inline void VectorSet(Vector3fType* pOutValue, float x, float y, float z) {
    float w = 0.0f;
    float32x2_t low = vcreate_f32(static_cast<uint64_t>(*reinterpret_cast<uint32_t*>(&x)) |
                                  static_cast<uint64_t>(*reinterpret_cast<uint32_t*>(&y)) << 32);
    float32x2_t high = vcreate_f32(static_cast<uint64_t>(*reinterpret_cast<uint32_t*>(&z)) |
                                   static_cast<uint64_t>(*reinterpret_cast<uint32_t*>(&w)) << 32);
    pOutValue->_v = vcombine_f32(low, high);
}

inline void VectorLoad(Vector3fType* pOutValue, const Float3& rSource) {
    float32x2_t low = vld1_f32(rSource.v);
    float32x2_t high =
        vcreate_f32(static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(&rSource.v[2])));
    pOutValue->_v = vcombine_f32(low, high);
}

inline float VectorGetX(const Vector3fType& vector) {
    return vgetq_lane_f32(vector._v, 0);
}

inline float VectorGetY(const Vector3fType& vector) {
    return vgetq_lane_f32(vector._v, 1);
}

inline float VectorGetZ(const Vector3fType& vector) {
    return vgetq_lane_f32(vector._v, 2);
}

};  // namespace util
};  // namespace nn