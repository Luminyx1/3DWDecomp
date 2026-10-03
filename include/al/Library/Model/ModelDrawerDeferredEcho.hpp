#pragma once

#include <gfx/seadGraphicsContextMRT.h>

#include "Project/Model/ModelDrawer.hpp"

namespace al {

/// Draw category index, passed by value as a small struct (in a 64-bit register).
struct EchoDrawCategory {
    s32 value;
};

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
