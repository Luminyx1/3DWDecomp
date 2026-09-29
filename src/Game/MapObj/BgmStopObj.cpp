#include "MapObj/BgmStopObj.hpp"
#include "Library/ActorUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/Bgm/BgmUtil.hpp"

namespace {
    NERVE_DECL(BgmStopObj, Wait);
    NERVE_DECL(BgmStopObj, Stop);
    NERVES_MAKE_NOSTRUCT(BgmStopObj, Wait, Stop)
};  // namespace

/**
 * Stage object that fades out all BGM once its stage switch turns on.
 * @param pName actor name
 */
BgmStopObj::BgmStopObj(const char* pName) : al::LiveActor(pName) {
}

/**
 * Reads FadeOutFrameNum and waits for the "start" stage switch.
 * @param rInfo placement / init info
 */
void BgmStopObj::init(const al::ActorInitInfo& rInfo) {
    al::initActorSceneInfo(this, rInfo);
    al::initActorPoseTRMSV(this);
    al::initActorSRT(this, rInfo);
    al::initStageSwitch(this, rInfo);
    al::initExecutorWatchObj(this, rInfo);
    al::initActorAudioKeeperWithout3D(this, rInfo, nullptr, nullptr);
    al::initActorClipping(this, rInfo);

    if (!al::tryGetArg(&mFadeOutFrameNum, rInfo, "FadeOutFrameNum")) {
        mFadeOutFrameNum = 0;
    }

    al::invalidateClipping(this);

    if (al::listenStageSwitchOnStart(this, al::Functor(this, &BgmStopObj::start))) {
        al::initNerve(this, &nrvBgmStopObjWait, 0);
    }

    makeActorAppeared();
}

/**
 * Stage switch callback: go to the Stop nerve.
 */
void BgmStopObj::start() {
    al::setNerve(this, &nrvBgmStopObjStop);
}

/**
 * Idle until the stage switch fires.
 */
void BgmStopObj::exeWait() {
}

/**
 * Stops all BGM over mFadeOutFrameNum frames, then removes itself.
 */
void BgmStopObj::exeStop() {
    if (al::isFirstStep(this)) {
        al::tryStopAllBgm(this, mFadeOutFrameNum);
        kill();
    }
}

BgmStopObj::~BgmStopObj() {
}