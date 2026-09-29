#pragma once

#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
class AreaObjGroup;
class SceneCameraInfo;

/// Turns on the switch of every CameraInSwitchOnArea that contains the look-at point of a camera.
class CameraInSwitchOnAreaDirector : public IUseAreaObj {
public:
    CameraInSwitchOnAreaDirector();

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

    void init(const SceneCameraInfo* pSceneCameraInfo, AreaObjDirector* pAreaObjDirector);
    void initAfterPlacement();
    void update();

    const SceneCameraInfo* mSceneCameraInfo = nullptr;  // _8
    AreaObjDirector* mAreaObjDirector = nullptr;        // _10
    AreaObjGroup* mAreaGroup = nullptr;                 // _18
};
}  // namespace al
