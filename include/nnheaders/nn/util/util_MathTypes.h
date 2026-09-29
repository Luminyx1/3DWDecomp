#pragma once

#include <nn/types.h>
#include "nn/util/detail/util_MathTypes.neon.h"

namespace nn::util {

typedef uint32_t AngleIndex;

struct Float2 {
    union {
        float v[2];
        struct {
            float x;
            float y;
        };
    };
};

struct Float3 {
    union {
        float v[3];
        struct {
            float x;
            float y;
            float z;
        };
    };
};

struct Float4 {
    union {
        float v[4];
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };
};

struct FloatColumnMajor4x3 {
    float m[3][4];
};

struct Unorm8x4 {
    union {
        uint8_t v[4];
    };
};

typedef Unorm8x4 Color4u8Type;

typedef neon::Vector3fType Vector3fType;
typedef neon::Vector4fType Vector4fType;
typedef neon::MatrixRowMajor4x3fType Matrix4x3fType;
typedef neon::MatrixColumnMajor4x3fType MatrixT4x3fType;
typedef neon::MatrixColumnMajor4x4fType MatrixT4x4fType;

}  // namespace nn::util
