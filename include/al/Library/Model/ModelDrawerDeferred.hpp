#pragma once

#include <container/seadPtrArray.h>
#include <gfx/seadGraphicsContextMRT.h>

#include "Project/Model/ModelDrawer.hpp"

namespace agl {
class DisplayList;
}

namespace al {

class ModelDrawerDeferred : public ModelDrawer {
public:
    ModelDrawerDeferred(const char* pName, bool isSecondCategory, bool);

    void createTable() override;
    void draw() const override;

protected:
    bool mIsSecondCategory;
    bool _39;
    sead::PtrArray<agl::DisplayList> mDisplayLists;
};

static_assert(sizeof(ModelDrawerDeferred) == 0x50);

class ModelDrawerDeferredPlayer : public ModelDrawerDeferred {
public:
    ModelDrawerDeferredPlayer(const char* pName, bool isSecondCategory, bool);

    void draw() const override;
};

}  // namespace al
