#pragma once
#include "Library/Scene/ISceneObj.hpp"
#include "Library/Scene/IUseSceneObjHolder.hpp"
#include "System/ScenarioList.hpp"
#include <container/seadPtrArray.h>
class OceanScenarioList : public al::ISceneObj {
  public:
    static ScenarioList* tryGetOceanScenarioList(const al::IUseSceneObjHolder* pUser, s32 quadrant);
    ScenarioList* getScenarioListByQuadrant(s32 quadrant);
    explicit OceanScenarioList(s32 capacity);
    void addList(ScenarioList* pList);
    s32 getScenarioNum() const;
    s32 getQuadrantIndexFromScenarioId(s32 scenarioId) const;

  private:
    sead::PtrArray<ScenarioList> mLists;
};
