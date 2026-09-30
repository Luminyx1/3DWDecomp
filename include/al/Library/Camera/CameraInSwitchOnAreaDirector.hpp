#pragma once

#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
class AreaObjGroup;
class SceneCameraInfo;

class CameraInSwitchOnAreaDirector : public IUseAreaObj {
public:
    CameraInSwitchOnAreaDirector();

    void init(const SceneCameraInfo* pSceneCameraInfo, AreaObjDirector* pAreaObjDirector);
    void initAfterPlacement();
    void update();

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

private:
    const SceneCameraInfo* mSceneCameraInfo = nullptr;
    AreaObjDirector* mAreaObjDirector = nullptr;
    AreaObjGroup* mAreaObjGroup = nullptr;
};

}  // namespace al
