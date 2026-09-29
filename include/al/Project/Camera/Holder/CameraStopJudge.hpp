#pragma once

#include <math/seadVector.h>

#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
/// Decides whether the camera should stop following its target.
class CameraStopJudge : public IUseAreaObj {
public:
    CameraStopJudge();

    bool isStop() const;
    void update(const sead::Vector3f& rPos);

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

    bool mIsInCameraStopArea = false;        // _8
    bool _9 = false;                         // _9
    bool mIsInvalidStopJudgeByDemo = false;  // _A
    AreaObjDirector* mAreaObjDirector = nullptr;  // _10
};
}  // namespace al
