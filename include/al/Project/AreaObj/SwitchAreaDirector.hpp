#pragma once

#include <basis/seadTypes.h>

#include "Project/AreaObj/IUseAreaObj.hpp"
#include "Project/Framework/MultiCoreQueueThread.hpp"

namespace al {
class IScenarioCompleteChecker;
class PlayerHolder;
class SwitchKeepOnAreaGroup;
class SwitchOnAreaGroup;

class SwitchAreaDirector : public IUseAreaObj, public MultiCoreQueueExecutor {
public:
    static SwitchAreaDirector* tryCreate(AreaObjDirector* pAreaObjDirector,
                                         const PlayerHolder* pPlayerHolder,
                                         MultiCoreQueueThread* pThread);

    SwitchAreaDirector(AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder,
                       MultiCoreQueueThread* pThread);
    ~SwitchAreaDirector();

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

    const char* executorName() const override { return "SwitchAreaDirector"; }

    void executeOnThread() override;

    void waitDone();
    void internalUpdate();
    void update();
    void endInit(IScenarioCompleteChecker* pChecker);

    const PlayerHolder* mPlayerHolder;
    AreaObjDirector* mAreaObjDirector;
    SwitchOnAreaGroup* mSwitchOnAreaGroup = nullptr;
    SwitchKeepOnAreaGroup* mSwitchKeepOnAreaGroup = nullptr;
    MultiCoreQueueThread* mThread;
};
}  // namespace al
