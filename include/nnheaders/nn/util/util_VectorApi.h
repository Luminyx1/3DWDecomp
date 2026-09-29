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

};  // namespace util
};  // namespace nn