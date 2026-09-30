#pragma once

#include <basis/seadTypes.h>

class alModelCafe;

namespace al {
class GraphicsSystemInfo;

class ModelDrawerBase {
public:
    ModelDrawerBase(const char* pName);

    virtual ~ModelDrawerBase() { ; }

    virtual void createTable() = 0;
    virtual void draw() const = 0;

    virtual bool isDepthShadowDrawer() const { return false; }

    virtual bool isRemoveable() const { return false; }

    virtual void registerModel(alModelCafe* pModel) = 0;
    virtual void addModel(alModelCafe* pModel) = 0;
    virtual void removeModel(alModelCafe* pModel) = 0;

    void setDrawInfo(GraphicsSystemInfo* pInfo, alModelCafe* pModel);

    const char* getName() const { return mName; }

protected:
    const char* mName;
    alModelCafe* mModel;
    GraphicsSystemInfo* mGraphicsSystemInfo;
};

static_assert(sizeof(ModelDrawerBase) == 0x20);

}  // namespace al
