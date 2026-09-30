#pragma once

#include <gfx/seadGraphicsContextMRT.h>

#include "Project/Model/ModelDrawer.hpp"

namespace al {

class ModelDrawerDeferredSky : public ModelDrawer {
public:
    ModelDrawerDeferredSky(const char* pName);

    void createTable() override;
    void draw() const override;

private:
    sead::GraphicsContextMRT mGraphicsContext;
};

static_assert(sizeof(ModelDrawerDeferredSky) == 0xb0);

}  // namespace al
