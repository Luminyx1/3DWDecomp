#pragma once

#include <nn/util/util_MathTypes.h>

namespace nn::util {

class MatrixRowMajor4x3f : public Matrix4x3fType {
public:
    static const MatrixRowMajor4x3f ConstantIdentity;
};

}  // namespace nn::util
