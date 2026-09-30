#pragma once

#include <gfx/seadGraphicsContextMRT.h>

#include "Project/Model/ModelDrawer.hpp"

namespace al {

enum EchoDrawCategory : s32 {};

class ModelDrawerDeferredEcho : public ModelDrawer {
public:
    ModelDrawerDeferredEcho(const char* pName, EchoDrawCategory category);

    void createTable() override;
    void draw() const override;

private:
    EchoDrawCategory mCategory;
    sead::GraphicsContextMRT mGraphicsContext;
};

}  // namespace al
