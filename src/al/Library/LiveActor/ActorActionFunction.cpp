#include "Library/LiveActor/Util/ActorActionUtil.hpp"

#include "Library/LiveActor/HitReactionKeeper.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveAction.hpp"
#include "Library/Nerve/NerveActionCtrl.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Project/Action/Common/ActionAnimCtrl.hpp"
#include "Project/Action/Common/ActorActionKeeper.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Starts an action, falling back to starting animations of the same name.
 * @param pActor The actor.
 * @param pActionName The action name.
 */
void startAction(LiveActor* pActor, const char* pActionName) {
    if (!pActor->mActionKeeper || !pActor->mActionKeeper->startAction(pActionName)) {
        tryStartSklAnimIfExist(pActor, pActionName);
        tryStartMtpAnimIfExist(pActor, pActionName);
        tryStartMclAnimIfExist(pActor, pActionName);
        tryStartMtsAnimIfExist(pActor, pActionName);
        tryStartVisAnimIfExist(pActor, pActionName);
    }
}

/**
 * Starts an action at a random frame of its skeletal animation.
 * @param pActor The actor.
 * @param pActionName The action name.
 * @return The frame.
 */
s32 startActionAtRandomFrame(LiveActor* pActor, const char* pActionName) {
    startAction(pActor, pActionName);
    s32 frame = getRandom(0.0f, getSklAnimFrameMax(pActor, 0));
    setSklAnimFrame(pActor, frame, 0);
    return frame;
}

/**
 * Starts an action if it or an animation of the same name exists.
 * @param pActor The actor.
 * @param pActionName The action name.
 * @return Whether the action was started.
 */
bool tryStartAction(LiveActor* pActor, const char* pActionName) {
    if (pActor->mActionKeeper && pActor->mActionKeeper->getAnimCtrl() &&
        pActor->mActionKeeper->getAnimCtrl()->isExistAction(pActionName)) {
        pActor->mActionKeeper->startAction(pActionName);
        return true;
    }

    bool isSklStarted = tryStartSklAnimIfExist(pActor, pActionName);
    bool isMtpStarted = tryStartMtpAnimIfExist(pActor, pActionName);
    bool isMclStarted = tryStartMclAnimIfExist(pActor, pActionName);
    bool isMtsStarted = tryStartMtsAnimIfExist(pActor, pActionName);
    bool isVisStarted = tryStartVisAnimIfExist(pActor, pActionName);

    if (!isSklStarted && !isMtpStarted && !isMclStarted && !isMtsStarted && !isVisStarted) {
        return false;
    }

    if (pActor->mActionKeeper) {
        pActor->mActionKeeper->startAction(pActionName);
    }

    return true;
}

/**
 * Starts an action unless it is already playing.
 * @param pActor The actor.
 * @param pActionName The action name.
 * @return Whether the action was started.
 */
bool tryStartActionIfNotPlaying(LiveActor* pActor, const char* pActionName) {
    if (isActionPlaying(pActor, pActionName)) {
        return false;
    }

    startAction(pActor, pActionName);
    return true;
}

/**
 * Checks whether an action is playing.
 * @param pActor The actor.
 * @param pActionName The action name.
 * @return Whether the action is playing.
 */
bool isActionPlaying(const LiveActor* pActor, const char* pActionName) {
    const char* playingName = nullptr;
    ActorActionKeeper* keeper = pActor->mActionKeeper;

    if (keeper && keeper->getAnimCtrl()) {
        playingName = keeper->getAnimCtrl()->getPlayingActionName();
    }

    if (!playingName) {
        playingName = alAnimFunction::getAllAnimName(pActor);
    }

    return playingName && isEqualString(playingName, pActionName);
}

/**
 * Starts the non-animation parts of an action.
 * @param pActor The actor.
 * @param pActionName The action name.
 */
void tryStartActionNoAnim(LiveActor* pActor, const char* pActionName) {
    if (pActor->mActionKeeper) {
        pActor->mActionKeeper->tryStartActionNoAnim(pActionName);
    }
}

/**
 * Starts the effects of an action.
 * @param pActor The actor.
 * @param pActionName The action name.
 */
void tryStartEffectAction(LiveActor* pActor, const char* pActionName) {
    if (pActor->mActionKeeper) {
        pActor->mActionKeeper->startEffectAction(pActionName);
    }
}

/**
 * Checks whether the current action ended.
 * @param pActor The actor.
 * @return Whether the action ended.
 */
bool isActionEnd(const LiveActor* pActor) {
    if (isSklAnimExist(pActor)) {
        return isSklAnimEnd(pActor, 0);
    }

    if (isMtpAnimExist(pActor)) {
        return isMtpAnimEnd(pActor);
    }

    if (isMclAnimExist(pActor)) {
        return isMclAnimEnd(pActor);
    }

    if (isMtsAnimExist(pActor)) {
        return isMtsAnimEnd(pActor);
    }

    if (isVisAnimExist(pActor)) {
        return isVisAnimEnd(pActor);
    }

    return true;
}

/**
 * Checks whether the actor has any animation player.
 * @param pActor The actor.
 * @return Whether an animation player exists.
 */
bool isExistAction(const LiveActor* pActor) {
    return isSklAnimExist(pActor) || isMtpAnimExist(pActor) || isMclAnimExist(pActor) ||
           isMtsAnimExist(pActor) || isVisAnimExist(pActor);
}

/**
 * Checks whether an action or an animation of the same name exists.
 * @param pActor The actor.
 * @param pActionName The action name.
 * @return Whether the action exists.
 */
bool isExistAction(const LiveActor* pActor, const char* pActionName) {
    ActorActionKeeper* keeper = pActor->mActionKeeper;

    if (keeper && keeper->getAnimCtrl() && keeper->getAnimCtrl()->isExistAction(pActionName)) {
        return true;
    }

    return isSklAnimExist(pActor, pActionName) || isMtpAnimExist(pActor, pActionName) ||
           isMclAnimExist(pActor, pActionName) || isMtsAnimExist(pActor, pActionName) ||
           isVisAnimExist(pActor, pActionName);
}

/**
 * Checks whether an action plays only once.
 * @param pActor The actor.
 * @param pActionName The action name.
 * @return Whether the action plays only once.
 */
bool isActionOneTime(const LiveActor* pActor, const char* pActionName) {
    if (pActor->mActionKeeper && pActor->mActionKeeper->getAnimCtrl() &&
        pActor->mActionKeeper->getAnimCtrl()->isExistAction(pActionName)) {
        return pActor->mActionKeeper->getAnimCtrl()->isActionOneTime(pActionName);
    }

    if (isSklAnimExist(pActor, pActionName)) {
        return isSklAnimOneTime(pActor, pActionName);
    }

    if (isMtpAnimExist(pActor, pActionName)) {
        return isMtpAnimOneTime(pActor, pActionName);
    }

    if (isMclAnimExist(pActor, pActionName)) {
        return isMclAnimOneTime(pActor, pActionName);
    }

    if (isMtsAnimExist(pActor, pActionName)) {
        return isMtsAnimOneTime(pActor, pActionName);
    }

    if (isVisAnimExist(pActor, pActionName)) {
        return isVisAnimOneTime(pActor, pActionName);
    }

    return true;
}

/**
 * Gets the frame of the current action.
 * @param pActor The actor.
 * @return The frame.
 */
f32 getActionFrame(const LiveActor* pActor) {
    ActorActionKeeper* keeper = pActor->mActionKeeper;

    if (keeper && keeper->getAnimCtrl()) {
        return keeper->getAnimCtrl()->getFrame();
    }

    return alAnimFunction::getAllAnimFrame(pActor, -1);
}

/**
 * Gets the last frame of an action.
 * @param pActor The actor.
 * @param pActionName The action name.
 * @return The last frame.
 */
f32 getActionFrameMax(const LiveActor* pActor, const char* pActionName) {
    ActorActionKeeper* keeper = pActor->mActionKeeper;

    if (keeper && keeper->getAnimCtrl()) {
        return keeper->getAnimCtrl()->getActionFrameMax(pActionName);
    }

    return alAnimFunction::getAllAnimFrameMax(pActor, pActionName, -1);
}

/**
 * Gets the frame rate of the current action.
 * @param pActor The actor.
 * @return The frame rate.
 */
f32 getActionFrameRate(const LiveActor* pActor) {
    ActorActionKeeper* keeper = pActor->mActionKeeper;

    if (keeper && keeper->getAnimCtrl()) {
        return keeper->getAnimCtrl()->getFrameRate();
    }

    return alAnimFunction::getAllAnimFrameRate(pActor, -1);
}

/**
 * Gets the name of the current action.
 * @param pActor The actor.
 * @return The action name.
 */
const char* getActionName(const LiveActor* pActor) {
    ActorActionKeeper* keeper = pActor->mActionKeeper;

    if (keeper && keeper->getAnimCtrl()) {
        const char* actionName = keeper->getAnimCtrl()->getPlayingActionName();

        if (actionName) {
            return actionName;
        }
    }

    return alAnimFunction::getAllAnimName(pActor);
}

/**
 * Sets the frame of the skeletal animation of the current action.
 * @param pActor The actor.
 * @param frame The frame.
 */
void setActionFrame(LiveActor* pActor, f32 frame) {
    if (isSklAnimExist(pActor)) {
        setSklAnimFrame(pActor, frame, 0);
    }
}

/**
 * Sets the frame of all animations of the current action.
 * @param pActor The actor.
 * @param frame The frame.
 */
void trySetActionFrame(LiveActor* pActor, f32 frame) {
    if (isSklAnimExist(pActor)) {
        setSklAnimFrame(pActor, frame, 0);
    }

    if (isMtpAnimExist(pActor)) {
        setMtpAnimFrame(pActor, frame);
    }

    if (isMclAnimExist(pActor)) {
        setMclAnimFrame(pActor, frame);
    }

    if (isMtsAnimExist(pActor)) {
        setMtsAnimFrame(pActor, frame);
    }

    if (isVisAnimExist(pActor)) {
        setVisAnimFrame(pActor, frame);
    }
}

/**
 * Sets the frame rate of the skeletal animation of the current action.
 * @param pActor The actor.
 * @param frameRate The frame rate.
 */
void setActionFrameRate(LiveActor* pActor, f32 frameRate) {
    if (isSklAnimExist(pActor)) {
        setSklAnimFrameRate(pActor, frameRate, 0);
    }
}

/**
 * Sets the frame rate of all animations of the current action.
 * @param pActor The actor.
 * @param frameRate The frame rate.
 */
void trySetActionFrameRate(LiveActor* pActor, f32 frameRate) {
    if (isSklAnimExist(pActor)) {
        setSklAnimFrameRate(pActor, frameRate, 0);
    }

    if (isMtpAnimExist(pActor)) {
        setMtpAnimFrameRate(pActor, frameRate);
    }

    if (isMclAnimExist(pActor)) {
        setMclAnimFrameRate(pActor, frameRate);
    }

    if (isMtsAnimExist(pActor)) {
        setMtsAnimFrameRate(pActor, frameRate);
    }

    if (isVisAnimExist(pActor)) {
        setVisAnimFrameRate(pActor, frameRate);
    }
}

/**
 * Stops all animations of the current action.
 * @param pActor The actor.
 */
void stopAction(LiveActor* pActor) {
    if (isSklAnimExist(pActor)) {
        setSklAnimFrameRate(pActor, 0.0f, 0);
    }

    if (isMtpAnimExist(pActor)) {
        setMtpAnimFrameRate(pActor, 0.0f);
    }

    if (isMclAnimExist(pActor)) {
        setMclAnimFrameRate(pActor, 0.0f);
    }

    if (isMtsAnimExist(pActor)) {
        setMtsAnimFrameRate(pActor, 0.0f);
    }

    if (isVisAnimExist(pActor)) {
        setVisAnimFrameRate(pActor, 0.0f);
    }
}

/**
 * Restarts all animations of the current action.
 * @param pActor The actor.
 */
void restartAction(LiveActor* pActor) {
    if (isSklAnimExist(pActor)) {
        setSklAnimFrameRate(pActor, 1.0f, 0);
    }

    if (isMtpAnimExist(pActor)) {
        setMtpAnimFrameRate(pActor, 1.0f);
    }

    if (isMclAnimExist(pActor)) {
        setMclAnimFrameRate(pActor, 1.0f);
    }

    if (isMtsAnimExist(pActor)) {
        setMtsAnimFrameRate(pActor, 1.0f);
    }

    if (isVisAnimExist(pActor)) {
        setVisAnimFrameRate(pActor, 1.0f);
    }
}

/**
 * Updates sound and effect actions between two frames.
 * @param pActor The actor.
 * @param frameFrom The start frame.
 * @param frameTo The end frame.
 */
void tryUpdateSeEffect(LiveActor* pActor, f32 frameFrom, f32 frameTo) {
    if (pActor->mActionKeeper) {
        pActor->mActionKeeper->tryUpdateSeEffect(frameFrom, frameTo);
    }
}

/**
 * Starts the action another actor is playing.
 * @param pActor The actor.
 * @param pSrcActor The actor to copy from.
 */
void copyAction(LiveActor* pActor, const LiveActor* pSrcActor) {
    if (!isExistAction(pActor, getActionName(pSrcActor))) {
        return;
    }

    startAction(pActor, getActionName(pSrcActor));

    if (isSklAnimExist(pSrcActor) && isSklAnimExist(pActor)) {
        copySklAnim(pActor, pSrcActor);
    }
}

/**
 * Starts a nerve action and its non-animation parts.
 * @param pActor The actor.
 * @param pActionName The action name.
 */
void startNerveAction(LiveActor* pActor, const char* pActionName) {
    if (pActor->mActionKeeper) {
        pActor->mActionKeeper->tryStartActionNoAnim(pActionName);
    }

    alNerveFunction::setNerveAction(pActor, pActionName);
}

/**
 * Sets a nerve when the current action ended.
 * @param pActor The actor.
 * @param pNerve The nerve.
 */
void setNerveAtActionEnd(LiveActor* pActor, const Nerve* pNerve) {
    if (isActionEnd(pActor)) {
        setNerve(pActor, pNerve);
    }
}

/**
 * Restarts the nerve action matching the current nerve.
 * @param pActor The actor.
 */
void resetNerveActionForInit(LiveActor* pActor) {
    NerveActionCtrl* actionCtrl = pActor->getNerveKeeper()->mActionCtrl;
    const Nerve* currentNerve = pActor->getNerveKeeper()->getCurrentNerve();

    for (s32 i = 0; i < actionCtrl->mNumActions; i++) {
        if (actionCtrl->mActions[i] == currentNerve) {
            startNerveAction(pActor, actionCtrl->mActions[i]->getActionName());
            return;
        }
    }
}

/**
 * Starts a hit reaction.
 * @param pActor The actor.
 * @param pName The reaction name.
 */
void startHitReaction(const LiveActor* pActor, const char* pName) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start(pName, nullptr, nullptr, nullptr);
    }
}

/**
 * Starts a hit reaction at the position of two sensors.
 * @param pActor The actor.
 * @param pName The reaction name.
 * @param pOther The other sensor.
 * @param pSelf The own sensor.
 */
void startHitReactionHitEffect(const LiveActor* pActor, const char* pName, const HitSensor* pOther, const HitSensor* pSelf) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start(pName, nullptr, pOther, pSelf);
    }
}

/**
 * Starts a hit reaction at a position.
 * @param pActor The actor.
 * @param pName The reaction name.
 * @param rPos The position.
 */
void startHitReactionHitEffect(const LiveActor* pActor, const char* pName, const sead::Vector3f& rPos) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start(pName, &rPos, nullptr, nullptr);
    }
}

/**
 * Starts a hit reaction at the translation of a matrix.
 * @param pActor The actor.
 * @param pName The reaction name.
 * @param pMtx The matrix.
 */
void startHitReactionHitEffect(const LiveActor* pActor, const char* pName, const sead::Matrix34f* pMtx) {
    if (pActor->mHitReactionKeeper) {
        sead::Vector3f pos(pMtx->m[0][3], pMtx->m[1][3], pMtx->m[2][3]);
        pActor->mHitReactionKeeper->start(pName, &pos, nullptr, nullptr);
    }
}

/**
 * Starts the "吹き飛びヒット" hit reaction at the position of two sensors.
 * @param pActor The actor.
 * @param pOther The other sensor.
 * @param pSelf The own sensor.
 */
void startHitReactionBlowHit(const LiveActor* pActor, const HitSensor* pOther, const HitSensor* pSelf) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("吹き飛びヒット", nullptr, pOther, pSelf);
    }
}

/**
 * Starts the "吹き飛びヒット" hit reaction at a position.
 * @param pActor The actor.
 * @param rPos The position.
 */
void startHitReactionBlowHit(const LiveActor* pActor, const sead::Vector3f& rPos) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("吹き飛びヒット", &rPos, nullptr, nullptr);
    }
}

/**
 * Starts the "吹き飛びヒット" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionBlowHit(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("吹き飛びヒット", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "吹き飛びヒット[直接ヒット]" hit reaction at the position of two sensors.
 * @param pActor The actor.
 * @param pOther The other sensor.
 * @param pSelf The own sensor.
 */
void startHitReactionBlowHitDirect(const LiveActor* pActor, const HitSensor* pOther, const HitSensor* pSelf) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("吹き飛びヒット[直接ヒット]", nullptr, pOther, pSelf);
    }
}

/**
 * Starts the "吹き飛びヒット[直接ヒット]" hit reaction at a position.
 * @param pActor The actor.
 * @param rPos The position.
 */
void startHitReactionBlowHitDirect(const LiveActor* pActor, const sead::Vector3f& rPos) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("吹き飛びヒット[直接ヒット]", &rPos, nullptr, nullptr);
    }
}

/**
 * Starts the "吹き飛びヒット[直接ヒット]" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionBlowHitDirect(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("吹き飛びヒット[直接ヒット]", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "出現" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionAppear(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("出現", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "消滅" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionDisappear(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("消滅", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "破壊" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionBreak(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("破壊", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "死亡" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionDeath(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("死亡", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "取得" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionGet(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("取得", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "開始" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionStart(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("開始", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "終了" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionEnd(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("終了", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "命中" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionHit(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("命中", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "爆発" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionExplode(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("爆発", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "着地" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionOnGround(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("着地", nullptr, nullptr, nullptr);
    }
}

/**
 * Starts the "踏み潰され" hit reaction.
 * @param pActor The actor.
 */
void startHitReactionPressDown(const LiveActor* pActor) {
    if (pActor->mHitReactionKeeper) {
        pActor->mHitReactionKeeper->start("踏み潰され", nullptr, nullptr, nullptr);
    }
}
}  // namespace al
