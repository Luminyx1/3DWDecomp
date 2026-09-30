#pragma once

#include <container/seadPtrArray.h>

#include "Library/Scene/ISceneObj.hpp"

namespace al {
class ActorInitInfo;
class FootPrint;

class FootPrintServer : public ISceneObj {
public:
    FootPrintServer(const ActorInitInfo& rInfo, const char* pArchiveName, s32 num);

    const char* getSceneObjName() const override { return "足跡サーバー"; }

    FootPrint* findDeadFootPrint();

    sead::PtrArray<FootPrint>* mFootPrints;
};
}  // namespace al
