#pragma once

#include "Project/AreaObj/IUseAreaObj.hpp"
#include "Project/Framework/MultiCoreQueueThread.hpp"

namespace al {
    class IScenarioCompleteChecker;
    class PlayerHolder;
    class SwitchKeepOnAreaGroup;
    class SwitchOnAreaGroup;

    /// Updates the scene's SwitchOnArea and SwitchKeepOnArea groups with the players' positions.
    class SwitchAreaDirector : public IUseAreaObj, public MultiCoreQueueExecutor {
    public:
        static SwitchAreaDirector* tryCreate(AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder,
                                             MultiCoreQueueThread* pQueueThread);

        SwitchAreaDirector(AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder,
                           MultiCoreQueueThread* pQueueThread);
        ~SwitchAreaDirector();

        void waitDone();
        void internalUpdate();
        void update();
        void endInit(IScenarioCompleteChecker* pChecker);

        virtual AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

        virtual const char* executorName() const override { return "SwitchAreaDirector"; }

        virtual void executeOnThread() override;

        const PlayerHolder* mPlayerHolder;              // _10
        AreaObjDirector* mAreaObjDirector;              // _18
        SwitchOnAreaGroup* mSwitchOnAreaGroup;          // _20
        SwitchKeepOnAreaGroup* mSwitchKeepOnAreaGroup;  // _28
        MultiCoreQueueThread* mQueueThread;             // _30
    };
};
