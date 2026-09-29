#pragma once

#include "nn/ui2d/ui2d_Types.h"

namespace nn {
namespace ui2d {
enum BnvgShapePathType {
    BnvgShapePathType_Path,
    BnvgShapePathType_Ellipse,
    BnvgShapePathType_Rect,
    BnvgShapePathType_Star,
    BnvgShapePathType_Max
};

class ResBlendMode {
public:
    ResBlendMode() {}

    void Set(BlendOp aBlendOp, BlendFactor aSrcFactor, BlendFactor aDstFactor, LogicOp aLogicOp) {
        m_BlendOperation = uint8_t(aBlendOp);
        m_SrcBlendFactor = uint8_t(aSrcFactor);
        m_DestBlendFactor = uint8_t(aDstFactor);
        m_LogicOp = uint8_t(aLogicOp);
    }

    uint8_t m_BlendOperation;
    uint8_t m_SrcBlendFactor;
    uint8_t m_DestBlendFactor;
    uint8_t m_LogicOp;
};
};  // namespace ui2d
};  // namespace nn