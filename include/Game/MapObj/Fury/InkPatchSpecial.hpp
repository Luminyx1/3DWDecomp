#pragma once
#include "Library/Scene/ISceneObj.hpp"
namespace al { class IUseSceneObjHolder; }
class InkPatch;
class InkPatchSpecial : public al::ISceneObj {
public:
    static InkPatch* tryGetSpecialInkPatch(const al::IUseSceneObjHolder*);
    static InkPatchSpecial* tryGetInkPatchSpecial(const al::IUseSceneObjHolder*);
    InkPatchSpecial();
    void addUnlockerInkPatch(InkPatch*);
    bool isUnlockerInkPatch(InkPatch*);
    bool isLastUnlockerInkPatch(InkPatch*);
    const char* getSceneObjName() const override;
private:
    InkPatch* mSpecialPatch = nullptr;
    InkPatch* mUnlockers[2] = {};
    int _20 = 0;
};
