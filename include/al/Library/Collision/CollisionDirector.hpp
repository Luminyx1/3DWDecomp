#pragma once

#include <basis/seadTypes.h>

#include "Library/Execute/IUseExecutor.hpp"
#include "Library/HostIO/IUseHioNode.hpp"

namespace al {
class ExecuteDirector;
class ICollisionPartsKeeper;

class CollisionDirector : public HioNode, public IUseExecutor {
public:
    CollisionDirector(ExecuteDirector* pExecuteDirector, s32 threadNum);

    void execute() override;
    void endInit();

    ICollisionPartsKeeper* getActivePartsKeeper() const { return mActivePartsKeeper; }

private:
    ICollisionPartsKeeper* mActivePartsKeeper;
    u8 _10[0x58 - 0x10];
};
}  // namespace al
