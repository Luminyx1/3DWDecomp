#pragma once

#include <container/seadPtrArray.h>

#include "Project/Model/ModelDrawer.hpp"

namespace agl {
class DisplayList;
}

namespace al {

class ModelDrawerDeferredExt : public ModelDrawer {
public:
    using DrawCallback = void (*)(void*, void*);

    ModelDrawerDeferredExt(const char* pName, bool isSecondCategory, bool);

    void createTable() override;
    void draw() const override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;
    virtual void setDrawInfo(GraphicsSystemInfo* pInfo, alModelCafe* pModel);

    void setDrawCallback(DrawCallback callback, void* pUserData);

private:
    bool mIsSecondCategory;
    bool _39;
    sead::PtrArray<agl::DisplayList> mDisplayLists;
    DrawCallback mDrawCallback = nullptr;
    void* mDrawCallbackUserData = nullptr;
};

static_assert(sizeof(ModelDrawerDeferredExt) == 0x60);

}  // namespace al
