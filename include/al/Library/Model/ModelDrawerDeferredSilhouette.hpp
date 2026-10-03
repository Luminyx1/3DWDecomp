#pragma once

#include <gfx/seadGraphicsContextMRT.h>

#include "Project/Model/ModelDrawer.hpp"

namespace al {

/// Draw category index, passed by value as a small struct (in a 64-bit register).
struct SilhouetteDrawCategory {
    s32 value;
};

class ModelDrawerDeferredSilhouette : public ModelDrawer {
public:
    ModelDrawerDeferredSilhouette(const char* pName, SilhouetteDrawCategory category);

    void createTable() override;
    void draw() const override;

private:
    SilhouetteDrawCategory mCategory;
    sead::GraphicsContextMRT mGraphicsContext;
};

}  // namespace al
