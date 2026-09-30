#pragma once

#include <gfx/seadGraphicsContextMRT.h>

#include "Project/Model/ModelDrawer.hpp"

namespace al {

class ModelDrawerHeightMap : public ModelDrawer {
public:
    ModelDrawerHeightMap(const char* pName);

    void createTable() override;
    void draw() const override;
    void addModel(alModelCafe* pModel) override;

private:
    sead::GraphicsContextMRT mGraphicsContext;
};

}  // namespace al
