#pragma once

#include "Project/Model/ModelDrawer.hpp"

namespace al {

class ModelDrawerForward : public ModelDrawer {
public:
    ModelDrawerForward(const char* pName, bool isSecondCategory, bool isAssignShader,
                       bool isRenderStateInvalidate, bool isRemoveable);

    void createTable() override;
    void draw() const override;
    bool isRemoveable() const override { return mIsRemoveable; }

private:
    bool mIsSecondCategory;
    bool mIsAssignShader;
    bool mIsRenderStateInvalidate;
    bool mIsRemoveable;
};

static_assert(sizeof(ModelDrawerForward) == 0x40);

}  // namespace al
