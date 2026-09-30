#pragma once

#include "Library/Scene/ISceneObj.hpp"
#include "Library/Scene/IUseSceneObjHolder.hpp"

typedef al::ISceneObj* (*CreateFunc)(int);

namespace al {
class SceneObjHolder {
public:
    SceneObjHolder(CreateFunc pCreator, int numObjs);
    ISceneObj* create(int objID);
    ISceneObj* tryGetObj(int objID) const;
    ISceneObj* getObj(int objID) const;
    bool isExist(int objID) const;
    void setSceneObj(ISceneObj* pObj, int objID);
    void initAfterPlacementSceneObj(const ActorInitInfo& rInfo);

    CreateFunc mFunc;   // 0x00
    ISceneObj** mObjs;  // 0x08
    int mNumObjs;       // 0x10
};
};  // namespace al
