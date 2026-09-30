#pragma once

#include <gfx/seadGraphicsContextMRT.h>

#include "Project/Model/ModelDrawer.hpp"

namespace al {

enum SilhouetteDrawCategory : s32 {};

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
