#pragma once

#include <math/seadVector.h>

#include "Library/HostIO/IUseHioNode.hpp"
#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {

class CameraStopJudge : public HioNode, public IUseAreaObj {
public:
    CameraStopJudge();

    bool isStop() const;
    void update(const sead::Vector3f& rPos);

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

    void setAreaObjDirector(AreaObjDirector* pDirector) { mAreaObjDirector = pDirector; }

    void setIsStopByDeathPlayer(bool isStop) { mIsStopByDeathPlayer = isStop; }

    void setIsInvalidStopJudgeByDemo(bool isInvalid) { mIsInvalidStopJudgeByDemo = isInvalid; }

private:
    bool mIsInCameraStopArea = false;
    bool mIsStopByDeathPlayer = false;
    bool mIsInvalidStopJudgeByDemo = false;
    AreaObjDirector* mAreaObjDirector = nullptr;
};

}  // namespace al
