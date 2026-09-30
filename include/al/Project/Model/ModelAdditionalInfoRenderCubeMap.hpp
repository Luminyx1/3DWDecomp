#pragma once

#include "Project/Model/ModelAdditionalInfo.hpp"

namespace al {

class ModelAdditionalInfoRenderCubeMap : public ModelAdditionalInfo {
public:
    ModelAdditionalInfoRenderCubeMap(const GraphicsSystemInfo* pInfo);
};

static_assert(sizeof(ModelAdditionalInfoRenderCubeMap) == 0x40);

}  // namespace al
