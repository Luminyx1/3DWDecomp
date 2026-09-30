#pragma once

#include <gfx/seadGraphicsContextMRT.h>

#include "Project/Model/ModelDrawer.hpp"

namespace al {

class ModelDrawerDepthOnly : public ModelDrawer {
public:
    ModelDrawerDepthOnly(const char* pName, bool isForceFarDepth, bool isRemoveable);
    ~ModelDrawerDepthOnly() override;

    void createTable() override;
    void draw() const override;
    bool isRemoveable() const override { return mIsRemoveable; }

private:
    sead::GraphicsContextMRT mGraphicsContext;
    bool mIsRemoveable;
    bool mIsForceFarDepth;
};

static_assert(sizeof(ModelDrawerDepthOnly) == 0xb0);

}  // namespace al
