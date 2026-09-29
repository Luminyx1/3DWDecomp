#include "Project/AreaObj/SwitchAreaDirector.hpp"
#include <math/seadVector.h>
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/SwitchKeepOnAreaGroup.hpp"
#include "Project/AreaObj/SwitchOnAreaGroup.hpp"

namespace al {
    class LiveActor;

    s32 getPlayerNumMax(const PlayerHolder* pHolder);
    bool isPlayerDead(const PlayerHolder* pHolder, s32 index);
    bool isPlayerAreaTarget(const PlayerHolder* pHolder, s32 index);
    const sead::Vector3f& getPlayerPos(const PlayerHolder* pHolder, s32 index);
    LiveActor* getPlayerActor(const PlayerHolder* pHolder, s32 index);
    bool isDisasterMode(LiveActor* pActor);

    /**
     * @brief Creates the director if the scene has any switch areas.
     * @param pAreaObjDirector The scene's area director.
     * @param pPlayerHolder The scene's player holder.
     * @param pQueueThread The thread to update on, or nullptr to update on the calling thread.
     * @return The created director, or nullptr if the scene has no switch areas.
     */
    SwitchAreaDirector* SwitchAreaDirector::tryCreate(AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder,
                                                      MultiCoreQueueThread* pQueueThread) {
        if (pAreaObjDirector->getAreaObjGroup("SwitchOnArea") == nullptr &&
            pAreaObjDirector->getAreaObjGroup("SwitchKeepOnArea") == nullptr) {
            return nullptr;
        }

        return new SwitchAreaDirector(pAreaObjDirector, pPlayerHolder, pQueueThread);
    }

    /** @brief Waits for a pending update on the queue thread to finish. */
    void SwitchAreaDirector::waitDone() {
        if (mQueueThread != nullptr) {
            mQueueThread->waitDone();
        }
    }

    /** @brief Updates the switch area groups with the positions of the players. */
    void SwitchAreaDirector::internalUpdate() {
        sead::Vector3f playerPos[64];
        s32 playerPosNum = 0;

        s32 playerNumMax = getPlayerNumMax(mPlayerHolder);
        for (s32 i = 0; i < playerNumMax; i++) {
            if (isPlayerDead(mPlayerHolder, i)) {
                continue;
            }

            if (!isPlayerAreaTarget(mPlayerHolder, i)) {
                continue;
            }

            playerPos[playerPosNum].set(getPlayerPos(mPlayerHolder, i));
            playerPosNum++;
        }

        LiveActor* player = getPlayerActor(mPlayerHolder, 0);
        bool isDisaster = player != nullptr ? isDisasterMode(player) : false;

        if (mSwitchOnAreaGroup != nullptr) {
            mSwitchOnAreaGroup->update(playerPos, playerPosNum, isDisaster);
        }

        if (mSwitchKeepOnAreaGroup != nullptr) {
            mSwitchKeepOnAreaGroup->update(playerPos, playerPosNum, isDisaster);
        }
    }

    /** @brief Requests an update on the queue thread, or updates directly if there is none. */
    void SwitchAreaDirector::update() {
        if (mQueueThread != nullptr) {
            mQueueThread->requestExecute(this);
            return;
        }

        internalUpdate();
    }

    /** @brief Updates the switch area groups on the queue thread. */
    void SwitchAreaDirector::executeOnThread() {
        internalUpdate();
    }

    /**
     * @brief Finishes initialization once all areas are registered.
     * @param pChecker The checker for completed scenarios.
     */
    void SwitchAreaDirector::endInit(IScenarioCompleteChecker* pChecker) {
        if (mSwitchOnAreaGroup != nullptr) {
            mSwitchOnAreaGroup->endInit(pChecker);
        }
    }

    /**
     * @brief Constructs the director and creates the switch area groups.
     * @param pAreaObjDirector The scene's area director.
     * @param pPlayerHolder The scene's player holder.
     * @param pQueueThread The thread to update on, or nullptr to update on the calling thread.
     */
    SwitchAreaDirector::SwitchAreaDirector(AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder,
                                           MultiCoreQueueThread* pQueueThread)
        : mPlayerHolder(pPlayerHolder), mAreaObjDirector(pAreaObjDirector), mSwitchOnAreaGroup(nullptr),
          mSwitchKeepOnAreaGroup(nullptr), mQueueThread(pQueueThread) {
        AreaObjGroup* switchOnGroup = pAreaObjDirector->getAreaObjGroup("SwitchOnArea");
        if (switchOnGroup != nullptr) {
            mSwitchOnAreaGroup = new SwitchOnAreaGroup(switchOnGroup);
        }

        AreaObjGroup* switchKeepOnGroup = pAreaObjDirector->getAreaObjGroup("SwitchKeepOnArea");
        if (switchKeepOnGroup != nullptr) {
            mSwitchKeepOnAreaGroup = new SwitchKeepOnAreaGroup(switchKeepOnGroup);
        }
    }

    /** @brief Waits for a pending update before the director is destroyed. */
    SwitchAreaDirector::~SwitchAreaDirector() {
        waitDone();
    }
};
