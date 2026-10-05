#pragma once

#include "MapObj/RenderMaterialIndirectParam.hpp"

namespace ChameleonStateUtil {
void updateIndirectParam(RenderMaterialIndirectParam*, float, const RenderMaterialIndirectParam*,
                         const RenderMaterialIndirectParam*);
void updateIndirectParam(RenderMaterialIndirectParam*, const RenderMaterialIndirectParam*);
bool isVisible(const RenderMaterialIndirectParam*, const RenderMaterialIndirectParam*,
               const RenderMaterialIndirectParam*);
void updateIndirectParam(RenderMaterialIndirectParam* pParam, float blendRate, float intensity,
                         const sead::Vector4f& rVector0, const sead::Vector4f& rVector1,
                         const sead::Vector4f& rVector2);
}
