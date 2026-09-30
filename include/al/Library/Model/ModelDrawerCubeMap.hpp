#pragma once

#include "Project/Model/ModelDrawer.hpp"

namespace al {

class ModelDrawerCubeMap : public ModelDrawer {
public:
    ModelDrawerCubeMap(const char* pName);

    void createTable() override;
    void draw() const override;
    void removeModel(alModelCafe* pModel) override;
};

static_assert(sizeof(ModelDrawerCubeMap) == 0x38);

}  // namespace al
