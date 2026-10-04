#include "MapObj/Dokan.hpp"
#include "Layout/DokanGuideBalloon.hpp"
#include "Layout/IslandMap.hpp"
#include "Layout/SingleModeSceneLayout.hpp"
#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Bgm/BgmLineFunction.hpp"
#include "Library/Camera/CameraPoserFunction.hpp"
#include "Library/Camera/CameraTargetBase.hpp"
#include "Library/Camera/CameraTargetHolder.hpp"
#include "Library/Camera/CameraTicket.hpp"
#include "Library/Camera/CameraUtil.hpp"
#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/SubActorKeeper.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Obj/CollisionObj.hpp"
#include "Library/Obj/PartsFunction.hpp"
#include "Library/Play/Camera/CameraPoserFix.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Library/Scene/SceneObjUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Shadow/Common/ShadowUtil.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "MapObj/ActorStateGiantBlow.hpp"
#include "MapObj/BindPuppeteerGroup.hpp"
#include "MapObj/DisasterModeController.hpp"
#include "MapObj/DokanBindPuppeteer.hpp"
#include "MapObj/DrcAssistDirectorUtil.hpp"
#include "MapObj/WarpObjUtil.hpp"
#include "NPC/GhostPlayerRecorder.hpp"
#include "Player/IUsePlayerKeyConfig.hpp"
#include "Player/Normal/PlayerActor.hpp"
#include "Player/Player.hpp"
#include "Player/PlayerActionFunc.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Camera/Main/CameraDirector_RS.hpp"
#include "Raidon/RaidonSurf.hpp"
#include "Scene/SceneObjID.hpp"
#include "System/Data/SingleModeDataFunction.hpp"
#include "System/GameDataHolderAccessor.hpp"
#include "System/GameDataHolderWriter.hpp"
#include "Util/AreaObjUtil.hpp"
#include "Util/ControlUserUtil.hpp"
#include "Util/DemoUtil.hpp"
#include "Util/PlayerUtil.hpp"
#include "Util/ScoreUtil.hpp"

namespace {
    NERVE_DECL(Dokan, Wait);
    NERVE_DECL(Dokan, GiantBlow);
    NERVE_DECL(Dokan, Deactive);
    NERVE_DECL(Dokan, PlayerIn);
    NERVE_DECL(Dokan, PlayerOutWaitInput);
    NERVE_DECL(Dokan, Appear);

    /// Appearance of the destination pipe of an out-only pair; it plays the normal appearance.
    class DokanNrvAppearOutOnly : public al::Nerve {
    public:
        void execute(al::NerveKeeper* pKeeper) const override {
            pKeeper->getParent<Dokan>()->exeAppear();
        }
    };

    NERVE_DECL(Dokan, Disappear);
    NERVE_DECL(Dokan, WaitAppearDestDokan);
    NERVE_DECL(Dokan, PlayerOutWithCameraWait);
    NERVE_DECL(Dokan, PlayerOutWithCameraTicketWait);
    NERVE_DECL(Dokan, PlayerOut);
    NERVE_DECL(Dokan, PlayerOutWaitForDstAnimDone);
    NERVES_MAKE_NOSTRUCT(Dokan, Wait, GiantBlow, Deactive, PlayerIn, PlayerOutWaitInput, Appear,
                         AppearOutOnly, Disappear, WaitAppearDestDokan, PlayerOutWithCameraWait,
                         PlayerOutWithCameraTicketWait, PlayerOut, PlayerOutWaitForDstAnimDone)
};  // namespace

/**
 * @brief Stop or restart the vertical move of the camera target that follows the player.
 * @param pActor Actor used to reach the camera director.
 * @param isStop True to stop the vertical move, false to restart it.
 */
inline void stopPlayerCameraTargetVerticalMove(const al::LiveActor* pActor, bool isStop) {
    al::CameraTargetBase* target =
        pActor->mActorSceneInfo->cameraDirector->getTargetHolder()->getTarget(0);

    if (al::isEqualString(target->getTargetName(), "プレイヤー")) {
        target->stopVerticalMove(isStop);
    }
}

/**
 * @brief Check whether a pipe waits for players or is bringing players in.
 * @param pDokan Pipe to check.
 * @return True in the wait or player-in state.
 */
inline bool isNerveWaitOrPlayerIn(const Dokan* pDokan) {
    return al::isNerve(pDokan, &NrvDokanWait) || al::isNerve(pDokan, &NrvDokanPlayerIn);
}

/**
 * @brief Get the figure director of a player actor.
 * @param pPlayer Actor of the player.
 * @return The figure director of the player.
 */
inline const PlayerFigureDirector* getPlayerFigureDirector(const al::LiveActor* pPlayer) {
    return static_cast<const PlayerActor*>(pPlayer)->getPlayer()->getFigureDirector();
}

/**
 * @brief Construct a warp pipe.
 * @param pName Name of the actor.
 */
Dokan::Dokan(const char* pName) : al::LiveActor(pName) {
}

/**
 * @brief Initialize the pipe from its placement: model, destination pipe, puppeteers, cameras
 * and collision.
 * @param rInfo Placement and scene information of the actor.
 */
void Dokan::init(const al::ActorInitInfo& rInfo) {
    mIsGold = false;
    mIsSingleMode = rInfo.getActorSceneInfo().isSingleMode;
    bool isUseSingleModeCollision = mIsSingleMode;
    const char* archiveName = "Dokan";

    if (al::isObjectName(rInfo, "DokanSide")) {
        isUseSingleModeCollision = false;
        mIsSide = true;
        archiveName = "DokanSide";
    }

    const char* collisionArchiveName;

    if (al::isObjectName(rInfo, "DokanGold")) {
        archiveName = "DokanGold";
        collisionArchiveName = "DokanGold";
        mIsGold = true;
    } else if (al::isObjectName(rInfo, "DokanGoldSide")) {
        isUseSingleModeCollision = false;
        mIsSide = true;
        archiveName = "DokanGoldSide";
        collisionArchiveName = "DokanGold";
        mIsGold = true;
    } else {
        collisionArchiveName = "Dokan";
    }

    if (mIsSingleMode) {
        al::tryGetArg(&mSaveFlagId, rInfo, "SingleModeSaveFlag");
    }

    mAudioDirector = al::getAudioDirector(rInfo);

    if (al::isObjectName(rInfo, "DokanUpsideDown")) {
        al::initActorWithArchiveName(this, rInfo, archiveName, "UpsideDown");
    } else {
        al::initActorWithArchiveName(this, rInfo, archiveName, nullptr);
    }

    al::tryGetArg(&mIsCameraInterpolate, rInfo, "CameraInterpolate");
    al::initNerve(this, &NrvDokanWait, 1);

    mBreakModel = new al::LiveActor("巨大マリオ吹き飛び壊れモデル");

    if (mIsGold) {
        al::initActorWithArchiveName(mBreakModel, rInfo, "DokanGoldBreak", nullptr);
    } else {
        al::initActorWithArchiveName(mBreakModel, rInfo, "DokanBreak", nullptr);
    }

    mBreakModel->makeActorDead();
    mTraceModel = new al::LiveActor("巨大マリオ吹き飛び残骸");

    if (mIsGold) {
        al::initActorWithArchiveName(mTraceModel, rInfo, "DokanGoldTrace", nullptr);
    } else {
        al::initActorWithArchiveName(mTraceModel, rInfo, "DokanTrace", nullptr);
    }

    mTraceModel->makeActorDead();

    mGiantBlowState = new ActorStateGiantBlow(this, nullptr, mBreakModel, mTraceModel);
    al::initNerveState(this, mGiantBlowState, &NrvDokanGiantBlow, "巨大マリオ吹き飛び");

    mPairDokan = this;
    mIsHideModel = false;
    al::tryGetArg(&mIsHideModel, rInfo, "IsHideModel");

    if (mIsHideModel) {
        al::hideModelIfShow(this);
    }

    bool isUseObjectCamera = false;
    al::tryGetArg(&isUseObjectCamera, rInfo, "IsUseObjectCamera");
    al::calcLinkChildNum(rInfo, "DestDokan");

    if (al::calcLinkChildNum(rInfo, "DestDokan") > 0) {
        al::ActorInitInfo destInfo;
        al::PlacementInfo destPlacementInfo;
        al::getLinksActorInfo(&destInfo, &destPlacementInfo, rInfo, "DestDokan", 0);

        auto* destDokan = new Dokan("土管");
        destDokan->init(destInfo);
        destDokan->mPairDokan = this;
        mPairDokan = destDokan;
        destDokan->setIsHideModel(mIsHideModel);
    }

    if (!al::tryGetPlacementID(mPlacementId, rInfo)) {
        makeActorDead();
        return;
    }

    mPuppeteerGroup = new BindPuppeteerGroup("土管バインド操作グループ", al::getPlayerNumMax(this));

    for (s32 i = 0; i < mPuppeteerGroup->getPuppeteerNumMax(); i++) {
        auto* puppeteer = new DokanBindPuppeteer("土管バインド操作", mIsSide, false, this);
        puppeteer->init(rInfo);
        mPuppeteerGroup->registerPuppeteer(puppeteer);
    }

    mBindOrderGroup = new BindPuppeteerGroup("土管バインド操作グループ(バインド順)",
                                             al::getPlayerNumMax(this));
    al::tryGetArg(&mType, rInfo, "Type");

    // An upside-down pipe is always the out-only end of a pair.
    if (al::isObjectName(rInfo, "DokanUpsideDown")) {
        mIsUpsideDown = true;
        mType = Type::OutOnly;
        al::rotateQuatXDirDegree(this, 180.0f);
        al::tryGetArg(&mIsControlPlayerOut, rInfo, "IsControlPlayerOut");
    } else {
        al::tryGetArg(&mIsControlPlayerOut, rInfo, "IsControlPlayerOut");
    }

    if (isTypeOutOnly()) {
        makeActorDead();
    } else {
        bool isSaved = mSaveFlagId >= 0 && SingleModeDataFunction::isGenericItemSaved(
                                               GameDataHolderAccessor(this), mSaveFlagId);

        if (!isSaved &&
            al::listenStageSwitchOnAppear(this, al::Functor(this, &Dokan::appearBySwitch))) {
            makeActorDead();
        } else {
            makeActorAppeared();
        }

        if (mIsSingleMode) {
            al::listenStageSwitchOn(this, "SwitchAppearInstant",
                                    al::Functor(this, &Dokan::appearBySwitchInstant));
        }
    }

    if (al::listenStageSwitchOnStart(this, al::Functor(this, &Dokan::activate))) {
        al::setNerve(this, &NrvDokanDeactive);
    }

    mGuideBalloons = new DokanGuideBalloon*[rc::getControlUserNumMax()];

    for (s32 i = 0; i < rc::getControlUserNumMax(); i++) {
        mGuideBalloons[i] = new DokanGuideBalloon(al::getTransPtr(this));
        mGuideBalloons[i]->init(rInfo);
    }

    mIsInWater = rc::isInWaterArea(mPairDokan);
    al::tryGetArg(&mIsBreakGiantMario, rInfo, "IsBreakGiantMario");

    if (!mIsSide && !mIsBreakGiantMario && !mIsSingleMode) {
        mIsBreakGiantMario = true;
    }

    bool isUseCamera = false;
    al::tryGetArg(&isUseCamera, rInfo, "IsUseObjectCamera");

    if (isUseCamera) {
        if (mIsSingleMode) {
            mCameraTicket = al::initObjectCamera_RS(this, rInfo, nullptr);
            alCameraFunction::initPriorityDokan(mCameraTicket);
        } else {
            mCameraInfo = al::initObjectCamera(this, rInfo, nullptr);
            al::tryChangeSingleCameraMode(mCameraInfo);
        }
    }

    al::tryGetArg(&mOutCameraPlayStep, rInfo, "OutCameraPlayStep");
    al::tryGetArg(&mPipeInitOffset, rInfo, "pipeInitOffset");
    al::tryGetArg(&mPipeInitOffsetDecayRate, rInfo, "pipeInitOffsetDecayRate");

    const char* collisionName = archiveName;

    if (isUseSingleModeCollision) {
        al::StringTmp<256> singleModeName("%sSM", archiveName);
        collisionName = singleModeName.cstr();
    }

    mCollisionObj = al::createCollisionObj(this, rInfo, collisionName,
                                           al::getHitSensor(this, "CollisionBase"),
                                           collisionArchiveName, nullptr);
    mCollisionObj->appear();

    if (al::isDead(this)) {
        al::invalidateCollisionParts(mCollisionObj);
    }

    al::tryGetArg(&mIsIgnorePlayerControlAfterExit, rInfo, "IsIgnorePlayerControlAfterExit");
}

/**
 * @brief Set whether the model of the pipe is hidden. Hiding also hides the sub actors.
 * @param isHide True to hide the model.
 */
void Dokan::setIsHideModel(bool isHide) {
    mIsHideModel = isHide;

    if (isHide) {
        auto* subActorKeeper = getSubActorKeeper();

        if (subActorKeeper != nullptr) {
            for (s32 i = 0; i < subActorKeeper->getSubActorNum(); i++) {
                subActorKeeper->getSubActorInfo(i)->mSyncType |= 4;
            }
        }

        al::hideModelIfShow(this);
    }
}

/// Make the pipe appear (with its appearance animation) when its appear switch turns on.
void Dokan::appearBySwitch() {
    if (mSaveFlagId >= 0) {
        SingleModeDataFunction::setGenericItemSaved(GameDataHolderWriter(this), mSaveFlagId);
    }

    appear();
    al::setNerve(this, &NrvDokanAppear);
    al::hideModel(this);
}

/// Make the pipe appear immediately when its instant appear switch turns on.
void Dokan::appearBySwitchInstant() {
    if (mSaveFlagId >= 0) {
        SingleModeDataFunction::setGenericItemSaved(GameDataHolderWriter(this), mSaveFlagId);
    }

    appear();
    al::setNerve(this, &NrvDokanWait);
    al::tryStartAction(this, "Wait");
    al::validateCollisionParts(mCollisionObj);
    al::showModelIfHide(this);
    al::validateClipping(this);
    mIsRequestedBindAll = false;
}

/// Make the pipe usable once its start switch turns on.
void Dokan::activate() {
    al::setNerve(this, &NrvDokanWait);
}

/// Update the material code of the pipe for water outside of the single mode.
void Dokan::initAfterPlacement() {
    if (!mIsSingleMode) {
        al::updateMaterialCodeWater(this);
    }
}

/**
 * @brief Handle the messages of the pipe: giant blows, players touching it and the bind
 * messages of the players that enter it.
 * @param pMsg Received message.
 * @param pOther Sensor of the sender.
 * @param pSelf Sensor of the pipe that received the message.
 * @return True if the message was handled.
 */
bool Dokan::receiveMsg(const al::SensorMsg* pMsg, al::HitSensor* pOther, al::HitSensor* pSelf) {
    if (al::isNerve(this, &NrvDokanDeactive) || al::isNerve(this, &NrvDokanGiantBlow) ||
        isTypeOutOnly()) {
        return false;
    }

    if (al::isNerve(this, &NrvDokanWait) && mIsBreakGiantMario &&
        mGiantBlowState->tryStartBlow(pMsg, pOther, pSelf)) {
        al::setNerve(this, &NrvDokanGiantBlow);
        rc::addScore(this, pOther, 0.0f, 0);

        if (mIsSingleMode) {
            al::invalidateCollisionParts(mCollisionObj);

            if (mPairDokan != nullptr) {
                mPairDokan->startGiantBlowKill(pMsg, pOther);
            }
        }

        return true;
    }

    if (mIsSide) {
        if (al::isMsgPlayerTouch(pMsg)) {
            sead::Vector3f playerFront = rc::getPlayerFront(pOther);
            sead::Vector3f front;
            al::calcFrontDir(&front, this);

            if (playerFront.dot(front) < 0.0f) {
                getPuppeteer(pOther)->setOnPlayerCountMax();
                return true;
            }
        }
    } else if (al::isMsgPlayerFloorTouch(pMsg)) {
        if (!isEnableEnter()) {
            return false;
        }

        setLayout(pOther);
        getPuppeteer(pOther)->setOnPlayerCountMax();
        return true;
    }

    if (al::isMsgBindStart(pMsg)) {
        if (!isEnableEnter()) {
            return false;
        }

        // The binary tests the nerve twice.
        if (!isNerveWaitOrPlayerIn(this)) {
            return false;
        }

        if (!isNerveWaitOrPlayerIn(this)) {
            return false;
        }

        if (al::isSensorName(pSelf, "DokanHipDropBind")) {
            bool isHipDrop = false;

            if (!mIsSide && al::isSensorPlayer(pOther) && !rc::isPlayerOnGround(pOther) &&
                rc::isPlayerHipDropping(pOther)) {
                isHipDrop = rc::getPlayerVelocity(pOther).y < -1.0f;
            }

            mIsHipDropEnter = isHipDrop;

            if (mIsHipDropEnter) {
                getPuppeteer(pOther)->setOnPlayerCountMax();
            }

            if (!getPuppeteer(pOther)->isEnableStartBind(mIsHipDropEnter, mIsRequestedBindAll)) {
                mIsHipDropEnter = false;
                return false;
            }
        } else {
            bool isHoldSquat = rc::getPlayerKeyConfig(pOther)->isPadHoldPlayerSquat();

            if (!getPuppeteer(pOther)->isEnableStartBind(isHoldSquat, mIsRequestedBindAll)) {
                return false;
            }
        }

        if (!al::isNerve(this, &NrvDokanPlayerIn)) {
            al::setNerve(this, &NrvDokanPlayerIn);
            WarpObjUtil::stopStageTimer(this);
        }

        return true;
    }

    if (al::isMsgBindInit(pMsg)) {
        mBindSensor = pSelf;
        auto* puppeteer = getPuppeteer(pOther);
        al::LiveActor* player = al::getSensorHost(pOther);

        if (mIsSingleMode) {
            mBindPlayer = player;

            if (rc::isPlayerInvincible(this, pOther)) {
                mInvinciblePlayerSensor = pOther;
                rc::trySetPlayerInvicibleBgmState(pOther, true);
            }

            auto* raidon = al::tryGetSceneObj<RaidonSurf>(this, SceneObjID_RaidonSurf);

            if (raidon != nullptr) {
                raidon->forceSpawn(true);
            }
        }

        bool isEnableRolling;

        if (player == nullptr) {
            isEnableRolling = true;
        } else if (PlayerActionFunc::isClimb(getPlayerFigureDirector(player))) {
            isEnableRolling = false;
        } else {
            isEnableRolling = !PlayerActionFunc::isRaccoonDog(getPlayerFigureDirector(player));
        }

        bool isRolling = rc::isPlayerRollingOnGround(pOther);
        puppeteer->startBind(pOther, pSelf, this, mPairDokan, mIsRequestedBindAll,
                             mIsHipDropEnter, isEnableRolling && isRolling);
        mIsHipDropEnter = false;

        s32 bindNum = mBindOrderGroup->getPuppeteerNum();

        for (s32 i = 0; i < bindNum; i++) {
            if (mBindOrderGroup->getPuppeteer(i)->getControlUserId() ==
                puppeteer->getControlUserId()) {
                mBindOrderGroup->insertPuppeteer(i, puppeteer);
                return true;
            }
        }

        mBindOrderGroup->registerPuppeteer(puppeteer);
        return true;
    }

    if (al::isMsgBindCancel(pMsg)) {
        auto* puppeteer = getPuppeteer(pOther);
        puppeteer->cancelBind();
        mBindOrderGroup->erasePuppeteer(puppeteer);

        if (mIsSingleMode) {
            mBindPlayer = nullptr;

            if (rc::isPlayerInvincible(this, pOther)) {
                rc::trySetPlayerInvicibleBgmState(pOther, false);
            }
        }

        if (mBindOrderGroup->getPuppeteerNum() == 0) {
            al::setNerve(this, &NrvDokanWait);
            WarpObjUtil::restartStageTimer(this);
            rc::cancelRequestBindAndResetDisableReviveBubbleForAllPlayer(this, pSelf);
        }

        return true;
    }

    return false;
}

/**
 * @brief Check whether the pipe can only be exited (it is the destination of another pipe).
 * @return True for an out-only pipe.
 */
bool Dokan::isTypeOutOnly() const {
    return mType == Type::OutOnly;
}

/**
 * @brief Try to blow the pipe away because a giant player rammed its pair pipe.
 * @param pMsg Message received by the pair pipe.
 * @param pOther Sensor of the giant player.
 * @return True if the pipe started to be blown away.
 */
bool Dokan::startGiantBlowKill(const al::SensorMsg* pMsg, al::HitSensor* pOther) {
    if (!al::isNerve(this, &NrvDokanWait)) {
        return false;
    }

    if (!mIsBreakGiantMario) {
        return false;
    }

    if (!mGiantBlowState->tryStartBlow(pMsg, pOther, al::getHitSensor(this, "CollisionBase"))) {
        return false;
    }

    al::setNerve(this, &NrvDokanGiantBlow);

    if (mIsSingleMode) {
        al::invalidateCollisionParts(mCollisionObj);
    }

    return true;
}

/**
 * @brief Check whether the enter switch allows players to enter the pipe.
 * @return True if the pipe can be entered.
 */
bool Dokan::isEnableEnter() {
    if (al::isValidStageSwitch(this, "SwitchEnter") && !al::isOnStageSwitch(this, "SwitchEnter")) {
        return false;
    }

    return true;
}

/**
 * @brief Show the guide balloon of a player standing on the pipe, or tell it that the player
 * still touches the pipe.
 * @param pPlayerSensor Sensor of the player.
 */
void Dokan::setLayout(const al::HitSensor* pPlayerSensor) {
    if (rc::isPlayerBinded(pPlayerSensor)) {
        return;
    }

    s32 userId = rc::findControlUserId(pPlayerSensor);

    if (al::isDead(mGuideBalloons[userId])) {
        s32 port = rc::getPlayerInputPort(pPlayerSensor);
        mGuideBalloons[userId]->setPlayerSensor(pPlayerSensor);
        mGuideBalloons[userId]->startShow(port);
    } else {
        mGuideBalloons[userId]->playerTouched();
    }
}

/**
 * @brief Get the puppeteer that drives a player.
 * @param pPlayerSensor Sensor of the player.
 * @return The puppeteer of the player.
 */
DokanBindPuppeteer* Dokan::getPuppeteer(const al::HitSensor* pPlayerSensor) const {
    return mPuppeteerGroup->getPuppeteerByPlayerIndex<DokanBindPuppeteer>(pPlayerSensor);
}

/// Drop the shadow of a side pipe along its front direction.
void Dokan::control() {
    if (mIsSide) {
        sead::Vector3f front = -sead::Vector3f::ez;
        al::calcFrontDir(&front, this);
        al::setShadowDropDir(this, -front, "Shadow01");
    }
}

/// Switch to the far LOD model unless the model is hidden.
void Dokan::startFarLod() {
    if (!mIsHideModel) {
        al::LiveActor::startFarLod();
    }
}

/// Switch back from the far LOD model unless the model is hidden.
void Dokan::endFarLod() {
    if (!mIsHideModel) {
        al::LiveActor::endFarLod();
    }
}

/// Stop waiting for the player input when clipped, and disable the collision.
void Dokan::startClipped() {
    if (mIsSingleMode && al::isNerve(this, &NrvDokanPlayerOutWaitInput)) {
        endWait();
    }

    al::LiveActor::startClipped();

    if (mIsSingleMode) {
        al::invalidateCollisionParts(mCollisionObj);
    }
}

/// End the exit of the players: give the camera back and return to the wait state.
void Dokan::endWait() {
    if (mIsSingleMode) {
        al::tryOffStageSwitchInstant(mPairDokan, "SwitchObjCameraOn");
    } else {
        al::offStageSwitch(mPairDokan, "SwitchObjCameraOn");
    }

    if (al::isActiveCamera(mPairDokan->mCameraTicket)) {
        al::endCamera_RS(this, mPairDokan->mCameraTicket, -1, false);
    }

    rc::resetDisableReviveBubbleForAllPlayer(this);

    if (mPairDokan->isTypeOutOnly()) {
        al::setNerve(mPairDokan, &NrvDokanDisappear);
    }

    mBindPlayer = nullptr;
    al::setNerve(this, &NrvDokanWait);
}

/// Enable the collision again when the pipe is no longer clipped and was not blown away.
void Dokan::endClipped() {
    al::LiveActor::endClipped();

    if (mIsSingleMode && mTraceModel != nullptr && al::isDead(mTraceModel)) {
        al::validateCollisionParts(mCollisionObj);
    }
}

/// Make an out-only pipe appear before the players come out of it.
void Dokan::appearOutOnly() {
    appear();
    al::setNerve(this, &NrvDokanAppearOutOnly);
    al::hideModel(this);
}

/// Make an out-only pipe disappear after the players came out of it.
void Dokan::disappearOutOnly() {
    al::setNerve(this, &NrvDokanDisappear);
}

/**
 * @brief Check whether the pipe waits for players.
 * @return True in the wait state.
 */
bool Dokan::isWait() const {
    return al::isNerve(this, &NrvDokanWait);
}

/// Update the puppeteers of the players.
void Dokan::updatePuppeteer() {
    mPuppeteerGroup->update();
}

/**
 * @brief Check whether every player is bound and waits to be warped.
 * @return True if the players can be warped.
 */
bool Dokan::isWarpStart() const {
    if (!rc::isAllPlayerBinded(this)) {
        return false;
    }

    rc::setDisableReviveBubbleForAllPlayer(al::getSensorHost(mBindSensor));

    s32 bindNum = mBindOrderGroup->getPuppeteerNum();

    for (s32 i = 0; i < bindNum; i++) {
        if (!mBindOrderGroup->getPuppeteer<DokanBindPuppeteer>(i)->isWaitStartWarp()) {
            return false;
        }
    }

    return true;
}

/// Warp the bound players to the pair pipe.
void Dokan::tryWarp() {
    s32 bindNum = mBindOrderGroup->getPuppeteerNum();
    bool isUseCamera = mPairDokan != this && mPairDokan != nullptr ?
                           mPairDokan->mCameraInfo != nullptr :
                           false;

    for (s32 i = 0; i < bindNum; i++) {
        mBindOrderGroup->getPuppeteer<DokanBindPuppeteer>(i)->warp(i, bindNum, isUseCamera);
    }

    if (mIsSingleMode) {
        al::tryOnStageSwitchInstant(mPairDokan, "SwitchWarpOn");
    } else {
        al::onStageSwitch(mPairDokan, "SwitchWarpOn");
    }

    if (mPairDokan != this) {
        if (mIsCameraInterpolate) {
            al::requestResetUserCameraControl(this);
        } else {
            al::requestCancelInterpole(this);
        }
    }

    if (rc::isExistGhostPlayerRecorder(this)) {
        rc::setPlacementIdObjGhostPlayerRecorder(this, mPlacementId, false);
        rc::tryStartFromObjGhostPlayerRecorder(this, mPlacementId);
    }
}

/**
 * @brief Check whether the camera of the pair pipe finished showing the players come out.
 * @return True once the out camera is done.
 */
bool Dokan::isFinishWaitOutCamera() {
    if (mPairDokan->mCameraInfo == nullptr && mPairDokan->mCameraTicket == nullptr) {
        return true;
    }

    if (mIsCameraInterpolate) {
        if (mIsSingleMode) {
            if (al::isLessStep(this, al::getCameraInterpoleStep(mPairDokan->mCameraTicket) +
                                         mPairDokan->mOutCameraPlayStep)) {
                return false;
            }
        } else {
            if (al::isLessStep(this, al::getCameraInterpoleFrame(mPairDokan->mCameraInfo) +
                                         mPairDokan->mOutCameraPlayStep)) {
                return false;
            }
        }
    } else {
        if (al::isLessStep(this, mPairDokan->mOutCameraPlayStep)) {
            return false;
        }
    }

    return true;
}

/// Wait for players and update their puppeteers.
void Dokan::exeWait() {
    if (al::isFirstStep(this)) {
        al::tryStartAction(this, "Wait");
        al::validateClipping(this);
        mIsRequestedBindAll = false;
    }

    mPuppeteerGroup->update();
}

/// Play the appearance animation, then wait.
void Dokan::exeAppear() {
    if (al::isFirstStep(this)) {
        if (mIsInWater) {
            al::startAction(this, "AppearWater");
        } else {
            al::startAction(this, "Appear");
        }

        al::invalidateClipping(this);
        al::validateCollisionParts(mCollisionObj);
        al::showModelIfHide(this);
    }

    if (al::isActionEnd(this)) {
        al::setNerve(this, &NrvDokanWait);
    }
}

/// Play the disappearance animation, then kill the pipe.
void Dokan::exeDisappear() {
    if (al::isFirstStep(this)) {
        if (mIsInWater) {
            al::startAction(this, "DisappearWater");
        } else {
            al::startAction(this, "Disappear");
        }

        al::invalidateClipping(this);
    }

    if (al::isActionEnd(this)) {
        kill();
    }
}

/// Bring the players into the pipe, then warp them once every player is ready.
void Dokan::exePlayerIn() {
    if (al::isFirstStep(this)) {
        if (mIsSingleMode) {
            rc::setDemoAudioType(this, static_cast<alSeFunction::DemoType>(4));

            if (!rc::requestStartDemoPlayerCutscene(this)) {
                al::setNerve(this, &NrvDokanPlayerIn);
                return;
            }

            rc::setUpdateItemsInDemo(this);
            rc::setDemoFullEffectUpdate(this, true);
            rc::addDemoActor(this);

            if (mPairDokan != nullptr) {
                rc::addDemoActor(mPairDokan);
            }

            if (mIsGold) {
                al::AreaObjGroup* mapDisableArea =
                    rc::tryFindAreaObjGroup(this, rc::AreaObjType::MapDisableArea);
                bool isEnterBonusArea =
                    mapDisableArea != nullptr ? !al::tryIsInAreaObjPlayer(mapDisableArea) : false;
                auto* islandMap = al::tryGetSceneObj<IslandMap>(this, SceneObjID_IslandMap);

                if (islandMap != nullptr) {
                    if (isEnterBonusArea) {
                        islandMap->updatePlayerTrackerForBonusArea(al::getTrans(this));
                    }

                    islandMap->setIsPlayerInBonusArea(isEnterBonusArea);
                    SingleModeDataFunction::setMapEnabled(GameDataHolderWriter(this), false);
                }
            } else {
                al::onUseCameraClippingPos(this);
            }

            stopPlayerCameraTargetVerticalMove(this, true);
        }

        al::invalidateClipping(this);
    }

    mPuppeteerGroup->update();

    if (al::isGreaterStep(this, 130)) {
        rc::requestBindAllPlayer(this, mBindSensor);
        mIsRequestedBindAll = true;
    }

    auto* disaster = DisasterModeController::tryGetController(this);

    if (rc::isAllPlayerBinded(this)) {
        rc::setDisableReviveBubbleForAllPlayer(this);

        if (disaster != nullptr && mIsGold &&
            (disaster->isDisasterMode() || disaster->isDisasterForeshadow())) {
            bool isStopBgm;

            if (disaster->isDisasterMode()) {
                isStopBgm = al::isBgmCurrentlyPlaying(this, "DisasterKoopa") ||
                            al::isBgmCurrentlyPlaying(this, "DisasterKoopaHard") ||
                            al::isBgmCurrentlyPlaying(this, "DisasterKoopaSuperHard");
            } else {
                isStopBgm = al::isBgmCurrentlyPlaying(this, "BeforeDisasterKoopa") ||
                            al::isBgmCurrentlyPlaying(this, "BeforeDisasterKoopaShort");
            }

            if (isStopBgm) {
                al::stopBgm(this, al::getCurPlayingBgmPlayName(this), 105, -1);
            }

            al::pauseOceanBgm(this, -1);
            al::pauseIslandBgm(this, -1);
        } else {
            al::tryPauseBgmIfDifferBgmArea(this, al::getTrans(mPairDokan), 105);
        }

        if (!mIsSetBgmVolume) {
            alSeFunction::setRequestKeeperVolumeSetting(mAudioDirector, "メイン", "土管入り", 60,
                                                        false);
            mIsSetBgmVolume = true;
        }
    }

    if (!isWarpStart()) {
        return;
    }

    if (mIsSingleMode && mIsGold) {
        if (disaster != nullptr) {
            rc::addDemoActor(disaster);

            if (disaster->getDemoSubActor() != nullptr) {
                rc::addDemoActor(disaster->getDemoSubActor());
            }
        }

        disaster->forceDisasterForeshadowOff(true, 1);
    }

    if (!mIsCameraInterpolate) {
        al::requestCaptureScreenCover(this, 4);
    }

    rc::releaseAllTouchPointerHoldItem(this);

    for (s32 i = 0; i < al::getPlayerNumMax(this); i++) {
        if (rc::isPlayerGiant(al::getHitSensor(al::getPlayerActor(this, i), "Body"))) {
            rc::cancelGiantMarioForce(al::getPlayerActor(this, i));
        }
    }

    for (s32 i = 0; i < rc::getControlUserNumMax(); i++) {
        mGuideBalloons[i]->kill();
    }

    if (mPairDokan->mCameraInfo != nullptr) {
        al::requestResetUserCameraControl(this);

        if (mIsSingleMode) {
            al::tryOnStageSwitchInstant(mPairDokan, "SwitchObjCameraOn");
        } else {
            al::onStageSwitch(mPairDokan, "SwitchObjCameraOn");
        }

        al::startCamera(this, mPairDokan->mCameraInfo, -1);
    } else if (mPairDokan->mCameraTicket != nullptr) {
        al::requestResetUserCameraControl(this);

        if (mIsSingleMode) {
            al::tryOnStageSwitchInstant(mPairDokan, "SwitchObjCameraOn");
        } else {
            al::onStageSwitch(mPairDokan, "SwitchObjCameraOn");
        }

        al::startCamera_RS(this, mPairDokan->mCameraTicket, -1);

        if (al::isEqualString(mPairDokan->mCameraTicket->getPoser()->getName(), "Fixed")) {
            mPairDokan->mCameraTicket->getPoser<al::CameraPoserFix>()->setOwnerObject(mPairDokan);
            mPairDokan->mCameraTicket->getPoser<al::CameraPoserFix>()->setDistanceInitOffset(
                mPipeInitOffset, mPipeInitOffsetDecayRate);
        }

        if (!mIsSingleMode) {
            rc::addDemoActor(this);

            if (mPairDokan != nullptr) {
                rc::addDemoActor(mPairDokan);
            }
        }
    }

    if (mIsSingleMode) {
        stopPlayerCameraTargetVerticalMove(this, false);
    }

    if (mPairDokan->isTypeOutOnly()) {
        al::setNerve(this, &NrvDokanWaitAppearDestDokan);
    } else if (mPairDokan->mCameraInfo != nullptr) {
        al::setNerve(this, &NrvDokanPlayerOutWithCameraWait);
    } else if (mPairDokan->mCameraTicket != nullptr) {
        al::setNerve(this, &NrvDokanPlayerOutWithCameraTicketWait);
    } else {
        al::setNerve(this, &NrvDokanPlayerOut);
    }
}

/// Warp the players, then wait for the object camera of the pair pipe before they come out.
void Dokan::exePlayerOutWithCameraWait() {
    if (al::isStep(this, 0)) {
        al::tryOnStageSwitchInstant(this, "SwitchPreExitPipeOn");
        tryWarp();

        if (mPairDokan != nullptr) {
            al::invalidateClipping(mPairDokan);
        }
    }

    if (al::isGreaterStep(this, 0) && isFinishWaitOutCamera()) {
        al::setNerve(this, &NrvDokanPlayerOut);

        if (mPairDokan != nullptr) {
            al::validateClipping(mPairDokan);
        }
    }
}

/// Wait for the camera ticket of the pair pipe before warping the players.
void Dokan::exePlayerOutWithCameraTicketWait() {
    if (isFinishWaitOutCamera()) {
        if (mPairDokan != nullptr) {
            al::invalidateClipping(mPairDokan);
        }

        al::tryOnStageSwitchInstant(this, "SwitchPreExitPipeOn");
        al::setNerve(this, &NrvDokanPlayerOutWaitForDstAnimDone);
    }
}

/// Wait for the pair pipe to be ready, then warp the players.
void Dokan::exePlayerOutWaitForDstAnimDone() {
    if (al::isNerve(mPairDokan, &NrvDokanWait)) {
        tryWarp();
        al::setNerve(this, &NrvDokanPlayerOut);

        if (mPairDokan != nullptr) {
            al::validateClipping(mPairDokan);
        }
    }
}

/// Bring the players out of the pair pipe one after the other.
void Dokan::exePlayerOut() {
    if (al::isFirstStep(this)) {
        if (mIsGold) {
            al::AreaObjGroup* mapDisableArea =
                rc::tryFindAreaObjGroup(this, rc::AreaObjType::MapDisableArea);
            auto* layout = al::tryGetSceneObj<SingleModeSceneLayout>(
                this, SceneObjID_SingleModeSceneLayout);

            if (mapDisableArea != nullptr && layout != nullptr) {
                if (al::tryIsInAreaObjPlayer(mapDisableArea)) {
                    layout->forceHideShineCounter();
                    layout->setDisableAreaName(true);
                } else {
                    layout->setDisableAreaName(false);
                }
            }
        }

        WarpObjUtil::restartStageTimer(this);
        alSeFunction::setRequestKeeperVolumeSetting(mAudioDirector, "メイン", "通常", 10, false);
        mIsSetBgmVolume = false;

        for (s32 i = 0; i < al::getPlayerNumMax(this); i++) {
            if (!rc::isValidPlayerEffect(al::getPlayerActor(this, i))) {
                rc::validatePlayerEffect(al::getPlayerActor(this, i));
            }
        }
    }

    if (al::isStep(this, 0) && !mPairDokan->isTypeOutOnly() &&
        mPairDokan->mCameraInfo == nullptr && mPairDokan->mCameraTicket == nullptr) {
        tryWarp();
    }

    if (al::isGreaterEqualStep(this, 0)) {
        for (s32 i = 0; i < mBindOrderGroup->getPuppeteerNum(); i++) {
            if (al::isStep(this, (mPairDokan->mIsUpsideDown ? 30 : 20) * i)) {
                mBindOrderGroup->getPuppeteer<DokanBindPuppeteer>(i)->dokanOut();
                break;
            }
        }

        mPuppeteerGroup->update();
    }

    if (!mBindOrderGroup->isEndBindAll()) {
        return;
    }

    if (mIsSingleMode) {
        al::offUseCameraClippingPos(this);
        rc::requestEndDemoPlayerCutscene(this);
        rc::setDemoAudioType(this, static_cast<alSeFunction::DemoType>(0));
        SingleModeDataFunction::setMapEnabled(GameDataHolderWriter(this), true);
    }

    if (mIsSingleMode && mInvinciblePlayerSensor != nullptr) {
        rc::trySetPlayerInvicibleBgmState(mInvinciblePlayerSensor, false);
        mInvinciblePlayerSensor = nullptr;
    }

    mBindSensor = nullptr;
    mBindOrderGroup->clearPuppeteer();
    rc::resetDisableReviveBubbleForAllPlayer(this);

    if (mPairDokan->mCameraInfo != nullptr) {
        if (mIsSingleMode) {
            al::tryOffStageSwitchInstant(mPairDokan, "SwitchObjCameraOn");
        } else {
            al::offStageSwitch(mPairDokan, "SwitchObjCameraOn");
        }

        al::endCamera(this, mPairDokan->mCameraInfo, -1);
    } else if (mPairDokan->mCameraTicket != nullptr) {
        mIsWaitInputLanding = false;
        mIsPlayerOutPosSet = false;
        al::setNerve(this, &NrvDokanPlayerOutWaitInput);
        return;
    }

    if (mPairDokan->isTypeOutOnly()) {
        al::setNerve(mPairDokan, &NrvDokanDisappear);
    }

    al::setNerve(this, &NrvDokanWait);
}

/// Keep the object camera after the exit until the player moves or turns the camera.
void Dokan::exePlayerOutWaitInput() {
    if (mBindPlayer == nullptr) {
        endWait();
        return;
    }

    const al::CameraPoser_RS* poser = mPairDokan->mCameraTicket->getPoser();

    if (!mIsWaitInputLanding && !rc::isPlayerOnGround(mBindPlayer)) {
        al::setNerve(this, &NrvDokanPlayerOutWaitInput);
        mIsWaitInputLanding = true;
        return;
    }

    if (rc::isAnyActiveDemo(this) ||
        alCameraPoserFunction::calcCameraRotateStickPower(poser) > 0.3f ||
        alCameraPoserFunction::isTriggerCameraResetRotate(poser)) {
        endWait();
        return;
    }

    if (!mIsWaitInputLanding) {
        return;
    }

    if (!mIsPlayerOutPosSet && rc::isPlayerOnGround(mBindPlayer)) {
        mPlayerOutPos = al::getTrans(mBindPlayer);
        mIsPlayerOutPosSet = true;
    }

    if (mIsPlayerOutPosSet) {
        sead::Vector3f trans = al::getTrans(mBindPlayer);

        if ((mPlayerOutPos - trans).length() > 20.0f) {
            endWait();
        }
    }
}

/// Make the out-only pair pipe appear, warp the players, then bring them out.
void Dokan::exeWaitAppearDestDokan() {
    if (al::isFirstStep(this)) {
        mPairDokan->appearOutOnly();
    }

    if (al::isStep(this, 0)) {
        tryWarp();
    }

    if (al::isStep(this, 1) && !mActorSceneInfo->isSingleMode) {
        al::invalidUserCameraControl(this);
    }

    if (al::isStep(this, 2)) {
        al::resetClippingDistanceStates(this);
    }

    mPuppeteerGroup->update();

    if (al::isGreaterStep(this, 2) && al::isNerve(mPairDokan, &NrvDokanWait) &&
        isFinishWaitOutCamera()) {
        if (!mActorSceneInfo->isSingleMode) {
            al::validUserCameraControl(this);
        }

        al::setNerve(this, &NrvDokanPlayerOut);
    }
}

/// Inactive until the start switch turns on.
void Dokan::exeDeactive() {
}

/// Blown away by a giant player.
void Dokan::exeGiantBlow() {
    if (al::isFirstStep(this)) {
        al::hideModelIfShow(this);
        al::invalidateCollisionParts(mCollisionObj);
    }

    if (al::updateNerveState(this)) {
        kill();
    }
}
