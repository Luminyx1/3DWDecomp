#pragma once

#include <math/seadVector.h>

#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
class AreaObj;
class AreaObjGroup;

/// Requests a vertical camera angle while the camera target is inside a CameraAngleVerticalRequestArea.
class CameraAngleVerticalRequester : public IUseAreaObj {
public:
    CameraAngleVerticalRequester();

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

    void init(AreaObjDirector* pAreaObjDirector);
    void initAfterPlacement();
    void update(const sead::Vector3f& rPos);

    s32 mFramesUnchanged = 0;                  // _8
    f32 mAngleVertical = 0.0f;                 // _C
    const AreaObj* mRequestArea = nullptr;     // _10
    AreaObjGroup* mRequestAreaGroup = nullptr; // _18
    AreaObjDirector* mAreaObjDirector = nullptr;  // _20
};
}  // namespace al
