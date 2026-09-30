#pragma once

#include "Project/Model/ModelDrawer.hpp"

namespace al {

class ModelDrawerDepthShadow : public ModelDrawer {
public:
    ModelDrawerDepthShadow(const char* pName);

    void createTable() override;
    void draw() const override;
    bool isDepthShadowDrawer() const override { return true; }
};

static_assert(sizeof(ModelDrawerDepthShadow) == 0x38);

}  // namespace al
