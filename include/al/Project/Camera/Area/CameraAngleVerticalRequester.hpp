#pragma once

#include <math/seadVector.h>

#include "Library/HostIO/IUseHioNode.hpp"
#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
class AreaObj;
class AreaObjGroup;

class CameraAngleVerticalRequester : public HioNode, public IUseAreaObj {
public:
    CameraAngleVerticalRequester();

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

    void init(AreaObjDirector* pAreaObjDirector);
    void initAfterPlacement();
    void update(const sead::Vector3f& rPos);

private:
    s32 mFramesUnchanged = 0;
    f32 mAngleVertical = 0.0f;
    const AreaObj* mRequestArea = nullptr;
    AreaObjGroup* mRequestAreaGroup = nullptr;
    AreaObjDirector* mAreaObjDirector = nullptr;
};

}  // namespace al
