#pragma once

#include <nn/font/font_Util.h>
#include <nn/util/util_MathTypes.h>

namespace nn {
namespace ui2d {
class DrawInfo {
public:
    DrawInfo();
    NN_RUNTIME_TYPEINFO_BASE();
    virtual ~DrawInfo();

    nn::util::MatrixT4x4fType m_ProjMtx;
    nn::util::MatrixT4x3fType m_ViewMtx;
    nn::util::MatrixT4x3fType m_ModelViewMtx;
    nn::util::Float2 m_LocationAdjustScale;
    unsigned char _B8[0xe8];

};
}  // namespace ui2d
}  // namespace nn
