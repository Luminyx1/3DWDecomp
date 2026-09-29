#pragma once

#include <arm_neon.h>

namespace nn {
namespace util {
namespace neon {

struct MatrixColumnMajor4x3fType {
    float32x4x3_t _m;
};

struct MatrixColumnMajor4x4fType {
    float32x4x4_t _m;
};
};  // namespace neon
};  // namespace util
};  // namespace nn
