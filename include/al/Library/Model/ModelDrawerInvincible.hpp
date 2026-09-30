#pragma once

#include <gfx/seadGraphicsContextMRT.h>

#include "Project/Model/ModelDrawer.hpp"

namespace al {

class ModelDrawerInvincible : public ModelDrawer {
public:
    ModelDrawerInvincible(const char* pName);

    void createTable() override;
    void draw() const override;

private:
    sead::GraphicsContextMRT mGraphicsContext;
};

}  // namespace al
