#pragma once

#include <math/seadVector.h>

#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
class AreaObjGroup;
class CameraStartParamArea;
struct CameraFlagCtrl;
struct CameraStartInfo;

class CameraStartParamCtrl : public IUseAreaObj {
public:
    CameraStartParamCtrl();

    void init(AreaObjDirector* pDirector, const CameraFlagCtrl* pFlagCtrl);
    void initAfterPlacement();
    void update(const sead::Vector3f& rPos);
    void tryApplyParam(CameraStartInfo* pInfo);

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

private:
    AreaObjDirector* mAreaObjDirector = nullptr;
    AreaObjGroup* mAreaGroup = nullptr;
    AreaObjGroup* mAreaGroupKids = nullptr;
    CameraStartParamArea* mCurrentArea = nullptr;
    const CameraFlagCtrl* mFlagCtrl = nullptr;
};

}  // namespace al
