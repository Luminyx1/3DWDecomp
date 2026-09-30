#include "Project/AreaObj/SwitchAreaDirector.hpp"

#include <new>

#include "Library/LiveActor/Util/ActorSceneUtil.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/SwitchKeepOnAreaGroup.hpp"
#include "Project/AreaObj/SwitchOnAreaGroup.hpp"

namespace al {
/**
 * Creates the switch area director if the stage has switch areas.
 * @param pAreaObjDirector area director
 * @param pPlayerHolder player holder
 * @param pThread thread to update on, or nullptr
 * @return the director, or nullptr if there are no switch areas
 */
SwitchAreaDirector* SwitchAreaDirector::tryCreate(AreaObjDirector* pAreaObjDirector,
                                                  const PlayerHolder* pPlayerHolder,
                                                  MultiCoreQueueThread* pThread) {
    if (!pAreaObjDirector->getAreaObjGroup("SwitchOnArea") &&
        !pAreaObjDirector->getAreaObjGroup("SwitchKeepOnArea")) {
        return nullptr;
    }

    return new SwitchAreaDirector(pAreaObjDirector, pPlayerHolder, pThread);
}

/**
 * Waits until the threaded update is done.
 */
void SwitchAreaDirector::waitDone() {
    if (mThread) {
        mThread->waitDone();
    }
}

/**
 * Updates the switch areas with the positions of the living players.
 */
void SwitchAreaDirector::internalUpdate() {
    sead::Vector3f positions[64];
    s32 numPositions = 0;
    s32 numPlayers = getPlayerNumMax(mPlayerHolder);
    for (s32 i = 0; i < numPlayers; i++) {
        if (isPlayerDead(mPlayerHolder, i) || !isPlayerAreaTarget(mPlayerHolder, i)) {
            continue;
        }

        new (&positions[numPositions]) sead::Vector3f(getPlayerPos(mPlayerHolder, i));
        numPositions++;
    }

    LiveActor* player = getPlayerActor(mPlayerHolder, 0);
    bool isDisaster = player ? isDisasterMode(player) : false;
    if (mSwitchOnAreaGroup) {
        mSwitchOnAreaGroup->update(positions, numPositions, isDisaster);
    }

    if (mSwitchKeepOnAreaGroup) {
        mSwitchKeepOnAreaGroup->update(positions, numPositions, isDisaster);
    }
}

/**
 * Updates the switch areas, on the thread if one is set.
 */
void SwitchAreaDirector::update() {
    if (mThread) {
        mThread->requestExecute(this);
        return;
    }

    internalUpdate();
}

/**
 * Updates the switch areas on the thread.
 */
void SwitchAreaDirector::executeOnThread() {
    internalUpdate();
}

/**
 * Finishes initialization.
 * @param pChecker scenario completion checker
 */
void SwitchAreaDirector::endInit(IScenarioCompleteChecker* pChecker) {
    if (mSwitchOnAreaGroup) {
        mSwitchOnAreaGroup->endInit(pChecker);
    }
}

/**
 * Constructs the director and its switch area groups.
 * @param pAreaObjDirector area director
 * @param pPlayerHolder player holder
 * @param pThread thread to update on, or nullptr
 */
SwitchAreaDirector::SwitchAreaDirector(AreaObjDirector* pAreaObjDirector,
                                       const PlayerHolder* pPlayerHolder,
                                       MultiCoreQueueThread* pThread)
    : mPlayerHolder(pPlayerHolder), mAreaObjDirector(pAreaObjDirector), mThread(pThread) {
    AreaObjGroup* switchOnGroup = pAreaObjDirector->getAreaObjGroup("SwitchOnArea");
    if (switchOnGroup) {
        mSwitchOnAreaGroup = new SwitchOnAreaGroup(switchOnGroup);
    }

    AreaObjGroup* keepOnGroup = pAreaObjDirector->getAreaObjGroup("SwitchKeepOnArea");
    if (keepOnGroup) {
        mSwitchKeepOnAreaGroup = new SwitchKeepOnAreaGroup(keepOnGroup);
    }
}

/**
 * Waits until the threaded update is done.
 */
SwitchAreaDirector::~SwitchAreaDirector() {
    if (mThread) {
        mThread->waitDone();
    }
}
}  // namespace al
