#pragma once

#include <nn/font/font_Util.h>
#include <nn/ui2d/ui2d_Pane.h>
#include <nn/util/util_MathTypes.h>

namespace nn {
namespace font { class GpuBuffer; }
namespace ui2d {
class DrawInfo {
public:
    DrawInfo();
    NN_RUNTIME_TYPEINFO_BASE();
    virtual ~DrawInfo();
    void ResetDrawState();

    nn::util::MatrixT4x4fType m_ProjMtx;
    nn::util::MatrixT4x3fType m_ViewMtx;
    nn::util::MatrixT4x3fType m_ModelViewMtx;
    nn::util::Float2 m_LocationAdjustScale;
    unsigned char _B8[8];
    const Pane::CalculateContext::LayoutInformation* m_pLayoutInformation;
    nn::font::GpuBuffer* m_pConstantBuffer;
    nn::font::GpuBuffer* m_pFontConstantBuffer;
    unsigned char _D8[0xc8];

};
}  // namespace ui2d
}  // namespace nn
