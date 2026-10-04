#include "MapObj/DokanBindPuppeteer.hpp"
#include "Camera/DummyCameraTarget.hpp"
#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Camera/CameraUtil.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "MapObj/BindWarpEffect.hpp"
#include "MapObj/Dokan.hpp"
#include "Player/IUsePlayerPuppet.hpp"
#include "Player/Normal/PlayerActor.hpp"
#include "Player/Normal/PlayerEquipmentDirector.hpp"
#include "Player/Normal/PlayerProperty.hpp"
#include "Player/Player.hpp"
#include "Player/PlayerBindEndParam.hpp"
#include "Util/AreaObjUtil.hpp"
#include "Util/PlayerPuppetUtil.hpp"
#include "Util/PlayerUtil.hpp"
#include "nerd/nerdMath.h"

namespace {
    NERVE_DECL(DokanBindPuppeteer, Deactive);
    NERVE_DECL(DokanBindPuppeteer, DokanInMove);
    NERVE_DECL(DokanBindPuppeteer, DokanInDown);
    NERVE_DECL(DokanBindPuppeteer, DokanInSideMove);
    NERVE_DECL(DokanBindPuppeteer, DokanInHipDrop);
    NERVE_DECL(DokanBindPuppeteer, DokanInRolling);
    NERVE_DECL(DokanBindPuppeteer, WaitStartWarp);
    NERVE_DECL(DokanBindPuppeteer, ForceBindWarp);
    NERVE_DECL(DokanBindPuppeteer, ForceBindWarpEnd);
    NERVE_DECL(DokanBindPuppeteer, WaitStartDokanOut);
    NERVE_DECL(DokanBindPuppeteer, DokanOutUp);
    NERVE_DECL(DokanBindPuppeteer, WaitStartWorldWarp);
    NERVES_MAKE_NOSTRUCT(DokanBindPuppeteer, Deactive, DokanInMove, DokanInDown, DokanInSideMove,
                         DokanInHipDrop, DokanInRolling, WaitStartWarp, ForceBindWarp,
                         ForceBindWarpEnd, WaitStartDokanOut, DokanOutUp, WaitStartWorldWarp)
};  // namespace

/// Number of frames a hip drop onto the pipe stays usable to enter it.
constexpr s32 cOnPlayerCountMax = 5;

/// Most players that get their own spot around the destination pipe.
constexpr s32 cOutPlayerNumMax = 12;

/// Height of the pipe mouth above the pipe's origin.
const sead::Vector3f cDokanInOffset(0.0f, 160.0f, 0.0f);

/// Offset from an upside-down pipe's origin to where the player comes out of it.
const sead::Vector3f cDokanOutUpsideDownOffset(0.0f, -320.0f, 0.0f);

/// Local offset of the point a player enters a side pipe from; also the out-move of a player
/// leaving a pipe that lies in water.
const sead::Vector3f cSideDokanInOffset(0.0f, 0.0f, 150.0f);

/// Out-moves that spread the players leaving a pipe together around it.
const sead::Vector3f cOutOffsets[] = {
    {5.4f, 19.0f, 5.5f},   {3.6f, 19.0f, 5.5f},   {1.8f, 19.0f, 5.5f},   {0.0f, 19.0f, 5.5f},
    {-1.8f, 19.0f, 5.5f},  {-3.6f, 19.0f, 5.5f},  {-5.4f, 19.0f, 5.5f},  {5.4f, 19.0f, 9.0f},
    {3.6f, 19.0f, 9.0f},   {1.8f, 19.0f, 9.0f},   {0.0f, 19.0f, 9.0f},   {-1.8f, 19.0f, 9.0f},
    {-3.6f, 19.0f, 9.0f},  {-5.4f, 19.0f, 9.0f},  {5.4f, 19.0f, 12.5f},  {3.6f, 19.0f, 12.5f},
    {1.8f, 19.0f, 12.5f},  {0.0f, 19.0f, 12.5f},  {-1.8f, 19.0f, 12.5f}, {-3.6f, 19.0f, 12.5f},
    {-5.4f, 19.0f, 12.5f},
};

/// Index into cOutOffsets of each player, per number of players leaving the pipe together.
const s32 cOutOffsetIndices[cOutPlayerNumMax][cOutPlayerNumMax] = {
    {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 2, 4, 6, 0, 0, 0, 0, 0, 0, 0, 0},
    {8, 10, 12, 2, 4, 0, 0, 0, 0, 0, 0, 0},
    {8, 10, 12, 1, 3, 5, 0, 0, 0, 0, 0, 0},
    {7, 9, 11, 13, 1, 3, 5, 0, 0, 0, 0, 0},
    {7, 9, 11, 13, 0, 2, 4, 6, 0, 0, 0, 0},
    {15, 17, 19, 8, 10, 12, 1, 3, 5, 0, 0, 0},
    {14, 16, 18, 20, 8, 10, 12, 1, 3, 5, 0, 0},
    {14, 16, 18, 20, 7, 9, 11, 13, 1, 3, 5, 0},
    {14, 16, 18, 20, 7, 9, 11, 13, 0, 2, 4, 6},
};

/// End of the bind after leaving a pipe.
const PlayerBindEndParam cBindEndParamDokanOut = {
    {}, 0, 30, true, true, true, 0, 1.2f, 0, false, {}};

/// End of the bind after leaving a pipe that ignores the player input for a moment.
const PlayerBindEndParam cBindEndParamDokanOutIgnoreControl = {
    {}, 0, 30, false, false, false, 0, 1.2f, 90, false, {}};

/// End of the bind after dropping out of an upside-down pipe.
const PlayerBindEndParam cBindEndParamDokanOutUpsideDown = {
    {}, 0, 15, false, false, false, 0, -1.0f, 0, false, {}};

/// End of the bind after dropping out of an upside-down pipe that keeps the player controlled.
const PlayerBindEndParam cBindEndParamDokanOutUpsideDownControl = {
    {}, 1, 15, true, true, true, 0, -1.0f, 0, false, {}};

/**
 * @brief Get the player actor a puppet drives.
 * @param pPuppet Puppet of the player.
 * @return The player actor, or nullptr.
 */
inline PlayerActor* getPuppetPlayerActor(IUsePlayerPuppet* pPuppet) {
    return static_cast<PlayerActor*>(al::getSensorHost(pPuppet->getMsgTargetSensor()));
}

/**
 * @brief Construct the puppeteer of one player.
 * @param pName Name of the puppeteer.
 * @param isSide True for a pipe lying on its side.
 * @param isWorldWarp True when the pipe leads to another world.
 * @param pHost Actor that owns the puppeteer.
 */
DokanBindPuppeteer::DokanBindPuppeteer(const char* pName, bool isSide, bool isWorldWarp,
                                       al::LiveActor* pHost)
    : BindPuppeteer(pName), mStartMtx(sead::Matrix34f::ident), mEndMtx(sead::Matrix34f::ident),
      mOutOffset(sead::Vector3f::zero), mIsSide(isSide), mIsWorldWarp(isWorldWarp), mHost(pHost),
      mVelocity(sead::Vector3f::zero) {
    initNerve(&NrvDokanBindPuppeteerDeactive, 0);
}

/**
 * @brief Create the warp light and, in single-player mode, the camera target.
 * @param rInfo Init info of the host actor.
 */
void DokanBindPuppeteer::init(const al::ActorInitInfo& rInfo) {
    mWarpEffect = new BindWarpEffect();
    mWarpEffect->init(rInfo);
    mIsSingleMode = rInfo.getActorSceneInfo().isSingleMode;

    if (mIsSingleMode) {
        mCameraTarget = new DummyCameraTarget("DokanBindPuppeteerCamera");
        mCameraTarget->init(rInfo);
    }
}

/**
 * @brief Bind a player and start moving it into the pipe.
 * @param pPlayerSensor Sensor of the player.
 * @param pBinderSensor Sensor of the pipe that binds the player.
 * @param pDokan Pipe the player enters.
 * @param pDestDokan Pipe the player comes out of.
 * @param isBindAll True when all players are pulled in at once.
 * @param isHipDrop True when the player entered with a hip drop.
 * @param isRolling True when the player rolled into the pipe.
 */
void DokanBindPuppeteer::startBind(al::HitSensor* pPlayerSensor, al::HitSensor* pBinderSensor,
                                   const al::LiveActor* pDokan, const Dokan* pDestDokan,
                                   bool isBindAll, bool isHipDrop, bool isRolling) {
    BindPuppeteer::startBind(pPlayerSensor, pBinderSensor);
    mBinderSensor = pBinderSensor;
    mIsHipDrop = isHipDrop;
    mIsRolling = isRolling;
    mDokan = pDokan;
    mDestDokan = pDestDokan;
    al::sendMsgWarpStart(rc::getPuppetSensor(getPlayerPuppet()), mBinderSensor);
    rc::hidePuppetSilhouette(getPlayerPuppet());

    if (mIsSide) {
        rc::hidePuppetShadow(getPlayerPuppet());
    }

    mIsInWater = rc::isInWaterArea(pDestDokan);
    rc::invalidatePlayerEffect(al::getSensorHost(pPlayerSensor));

    if (!mIsSide) {
        mIsRolling = false;
    }

    if (!mIsHipDrop && !mIsRolling) {
        rc::startPuppetAction(getPlayerPuppet(), mIsSide ? "DokanSideIn" : "DokanIn");
    }

    if (isBindAll) {
        mWarpEffect->start(pPlayerSensor, al::getSensorPos(al::getHitSensor(pDokan, "Dokan")),
                           false);
        al::setNerve(this, &NrvDokanBindPuppeteerForceBindWarp);
        return;
    }

    al::makeMtxFrontUpPos(&mStartMtx, rc::getPuppetFrontVec(getPlayerPuppet()),
                          rc::getPuppetUpVec(getPlayerPuppet()),
                          rc::getPuppetTrans(getPlayerPuppet()));
    mVelocity.set(rc::getPuppetVelocity(getPlayerPuppet()));
    const sead::Vector3f& velocity = rc::getPuppetVelocity(getPlayerPuppet());
    mSpeedH = nerd::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);

    if (mIsSide) {
        sead::Vector3f pos;
        al::calcTransLocalOffset(&pos, pDokan, cSideDokanInOffset);
        sead::Vector3f front;
        al::calcFrontDir(&front, pDokan);
        al::makeMtxUpFrontPos(&mEndMtx, sead::Vector3f::ey, -front, pos);
    } else if (mIsHipDrop) {
        setupHipDropIn(pDokan);
    } else {
        sead::Vector3f cameraDir;
        al::calcCameraDir(&cameraDir, pDokan);
        al::makeMtxUpFrontPos(&mEndMtx, sead::Vector3f::ey, cameraDir,
                              al::getTrans(pDokan) + cDokanInOffset);
    }

    if (mIsHipDrop) {
        al::setNerve(this, &NrvDokanBindPuppeteerDokanInHipDrop);
    } else if (mIsRolling) {
        al::setNerve(this, &NrvDokanBindPuppeteerDokanInRolling);
    } else {
        al::setNerve(this, &NrvDokanBindPuppeteerDokanInMove);
    }
}

/**
 * @brief Aim the move of a hip drop at the pipe and pull the player halfway onto it.
 * @param pDokan Pipe the player drops into.
 */
void DokanBindPuppeteer::setupHipDropIn(const al::LiveActor* pDokan) {
    mEndMtx = mStartMtx;
    mEndMtx.setTranslation(al::getTrans(pDokan));
    sead::Vector3f dokanTrans(mEndMtx(0, 3), mEndMtx(1, 3), mEndMtx(2, 3));
    sead::Vector3f trans = rc::getPuppetTrans(getPlayerPuppet());
    f32 y = sead::Mathf::clampMin(trans.y + mVelocity.y, dokanTrans.y);
    trans += (dokanTrans - trans) * 0.5f;
    trans.y = y;
    rc::setPuppetTrans(getPlayerPuppet(), trans);
}

/**
 * @brief Bind a player that enters a pipe leading to another world.
 * @param pPlayerSensor Sensor of the player.
 * @param pBinderSensor Sensor of the pipe that binds the player.
 * @param pDokan Pipe the player enters.
 * @param isBindAll True when all players are pulled in at once.
 * @param isHipDrop True when the player entered with a hip drop.
 */
void DokanBindPuppeteer::startBindWorldWarp(al::HitSensor* pPlayerSensor,
                                            al::HitSensor* pBinderSensor,
                                            const al::LiveActor* pDokan, bool isBindAll,
                                            bool isHipDrop) {
    BindPuppeteer::startBind(pPlayerSensor, pBinderSensor);
    mBinderSensor = pBinderSensor;
    mIsHipDrop = isHipDrop;
    mDokan = pDokan;
    al::sendMsgWarpStart(rc::getPuppetSensor(getPlayerPuppet()), mBinderSensor);
    rc::hidePuppetSilhouette(getPlayerPuppet());

    if (isBindAll) {
        mWarpEffect->start(pPlayerSensor, al::getSensorPos(al::getHitSensor(pDokan, "Dokan")),
                           false);
        al::setNerve(this, &NrvDokanBindPuppeteerForceBindWarp);
        return;
    }

    al::makeMtxFrontUpPos(&mStartMtx, rc::getPuppetFrontVec(getPlayerPuppet()),
                          rc::getPuppetUpVec(getPlayerPuppet()),
                          rc::getPuppetTrans(getPlayerPuppet()));
    mVelocity.set(rc::getPuppetVelocity(getPlayerPuppet()));

    if (mIsHipDrop) {
        setupHipDropIn(pDokan);
    } else {
        sead::Vector3f cameraDir;
        al::calcCameraDir(&cameraDir, pDokan);
        al::makeMtxUpFrontPos(&mEndMtx, sead::Vector3f::ey, cameraDir,
                              al::getTrans(pDokan) + cDokanInOffset);
    }

    if (mIsHipDrop) {
        al::setNerve(this, &NrvDokanBindPuppeteerDokanInHipDrop);
    } else {
        al::setNerve(this, &NrvDokanBindPuppeteerDokanInMove);
    }
}

/**
 * @brief Release the player at the end of the out-move.
 * @param pParam How the bind ends.
 */
void DokanBindPuppeteer::endBind(const PlayerBindEndParam* pParam) {
    al::sendMsgWarpEnd(rc::getPuppetSensor(getPlayerPuppet()), mBinderSensor);
    rc::validatePuppetDynamics(getPlayerPuppet());
    BindPuppeteer::endBind(pParam);
    mOnPlayerCount = 0;
    mDokan = nullptr;
    mDestDokan = nullptr;
    al::setNerve(this, &NrvDokanBindPuppeteerDeactive);
}

/// Release the player before the warp is over.
void DokanBindPuppeteer::cancelBind() {
    rc::validatePuppetDynamics(getPlayerPuppet());
    BindPuppeteer::cancelBind();
    mWarpEffect->cancel();
    mOnPlayerCount = 0;
    mDokan = nullptr;
    mDestDokan = nullptr;
    al::setNerve(this, &NrvDokanBindPuppeteerDeactive);
}

/// Run the current state.
void DokanBindPuppeteer::update() {
    updateNerve();
}

/// Note that the player stands on the pipe, which lets a hip drop enter it for a few frames.
void DokanBindPuppeteer::setOnPlayerCountMax() {
    mOnPlayerCount = cOnPlayerCountMax;
}

/**
 * @brief Check whether no player is bound.
 * @return True while no player is bound.
 */
bool DokanBindPuppeteer::isDeactive() const {
    return al::isNerve(this, &NrvDokanBindPuppeteerDeactive);
}

/**
 * @brief Check whether the player may enter the pipe now.
 * @param isHipDrop True when the player hip drops onto the pipe.
 * @param isBindAll True when all players are pulled in at once.
 * @return True when the bind may start.
 */
bool DokanBindPuppeteer::isEnableStartBind(bool isHipDrop, bool isBindAll) const {
    if (!al::isNerve(this, &NrvDokanBindPuppeteerDeactive)) {
        return false;
    }

    if (mIsSide) {
        return true;
    }

    if (isHipDrop && mOnPlayerCount > 0) {
        return true;
    }

    return isBindAll;
}

/**
 * @brief Check whether the player is in the pipe and waits for the warp.
 * @return True while waiting for the warp.
 */
bool DokanBindPuppeteer::isWaitStartWarp() const {
    return al::isNerve(this, &NrvDokanBindPuppeteerWaitStartWarp);
}

/**
 * @brief Check whether the player is in the pipe and waits for the warp to another world.
 * @return True while waiting for the world warp.
 */
bool DokanBindPuppeteer::isWaitStartWorldWarp() const {
    return al::isNerve(this, &NrvDokanBindPuppeteerWaitStartWorldWarp);
}

/**
 * @brief Put the player into the destination pipe.
 * @param index Order of the player among the players leaving together.
 * @param num Number of players leaving together.
 * @param isUseCamera True when the camera shows the destination before the player comes out.
 */
void DokanBindPuppeteer::warp(s32 index, s32 num, bool isUseCamera) {
    sead::Vector3f front;
    al::calcFrontDir(&front, mDestDokan);

    if (mDestDokan->isUpsideDown()) {
        al::rotateVectorDegreeY(&front, 180.0f);
        rc::setPuppetFrontVec(getPlayerPuppet(), front);
        rc::setPuppetUpVec(getPlayerPuppet(), sead::Vector3f::ey);
        rc::setPuppetTrans(getPlayerPuppet(),
                           al::getTrans(mDestDokan) + cDokanOutUpsideDownOffset);
    } else {
        rc::setPuppetFrontVec(getPlayerPuppet(), front);
        rc::setPuppetUpVec(getPlayerPuppet(), sead::Vector3f::ey);
        rc::setPuppetTrans(getPlayerPuppet(), al::getTrans(mDestDokan) + cDokanInOffset);

        if (mIsInWater) {
            mOutOffset.set(cSideDokanInOffset);
        } else {
            s32 indexRow = sead::Mathi::min(num, cOutPlayerNumMax) - 1;
            mOutOffset.set(cOutOffsets[cOutOffsetIndices[indexRow][index % cOutPlayerNumMax]]);
        }
    }

    al::setNerve(this, &NrvDokanBindPuppeteerWaitStartDokanOut);

    if (!mDestDokan->isUpsideDown() && isUseCamera) {
        auto* player = getPuppetPlayerActor(getPlayerPuppet());
        rc::hidePuppet(getPlayerPuppet());
        player->getPlayer()->getEquipmentDirector()->hideCrown();
    }
}

/// Start moving the player out of the destination pipe.
void DokanBindPuppeteer::dokanOut() {
    al::setNerve(this, &NrvDokanBindPuppeteerDokanOutUp);
}

/**
 * @brief Get the player action of leaving the destination pipe.
 * @return Name of the action.
 */
const char* DokanBindPuppeteer::getDokanOutActionName() const {
    return mDestDokan->isUpsideDown() ? "DokanOutUpsideDown" : "DokanOut";
}

/// Pull the hip-dropping player halfway onto the pipe while it falls in.
void DokanBindPuppeteer::updateHipDropPos() {
    sead::Vector3f dokanTrans;
    mEndMtx.getTranslation(dokanTrans);
    sead::Vector3f trans = rc::getPuppetTrans(getPlayerPuppet());
    f32 y = sead::Mathf::max(dokanTrans.y, trans.y + mVelocity.y);
    trans += (dokanTrans - trans) * 0.5f;
    trans.y = y;
    rc::setPuppetTrans(getPlayerPuppet(), trans);
}

/// Let the camera follow a dummy target at the player instead of the player itself.
inline void DokanBindPuppeteer::startCameraTarget() {
    sead::Vector3f trans = rc::getPuppetTrans(getPlayerPuppet());
    const sead::Vector3f& cameraPos = al::getCameraPos_RS(mHost, 0);
    f32 dx = trans.x - cameraPos.x;
    f32 dz = trans.z - cameraPos.z;
    f32 distance = nerd::sqrt(dx * dx + dz * dz);
    mCameraTarget->setTrans(trans);
    mCameraTarget->setRequestDistance(distance);
    mCameraTarget->onTarget();
    mIsCameraTargetOn = true;
}

/// Let the camera follow the player again.
inline void DokanBindPuppeteer::endCameraTarget() {
    if (mIsSingleMode) {
        if (mIsCameraTargetOn) {
            mCameraTarget->offTarget();
        }

        mIsCameraTargetOn = false;
    }
}

/// Count down the frames a hip drop may still enter the pipe.
void DokanBindPuppeteer::exeDeactive() {
    if (mOnPlayerCount > 0) {
        mOnPlayerCount--;
    }
}

/// Move the player in front of the pipe mouth.
void DokanBindPuppeteer::exeDokanInMove() {
    s32 moveStep = mIsSide ? 10 : 8;
    f32 rate = al::calcNerveEaseOutRate(this, moveStep);
    sead::Matrix34f mtx;
    al::blendMtx(&mtx, mStartMtx, mEndMtx, rate);
    sead::Vector3f trans;
    mtx.getTranslation(trans);
    sead::Vector3f front;
    mtx.getBase(front, 2);
    rc::setPuppetFrontVec(getPlayerPuppet(), front);
    rc::setPuppetTrans(getPlayerPuppet(), trans);

    if (al::isStep(this, moveStep)) {
        if (mIsSide) {
            al::setNerve(this, &NrvDokanBindPuppeteerDokanInSideMove);
        } else {
            al::setNerve(this, &NrvDokanBindPuppeteerDokanInDown);
        }
    }
}

/// Sink the player down into the pipe.
void DokanBindPuppeteer::exeDokanInDown() {
    if (al::isFirstStep(this)) {
        al::startSe(mDokan, "PgIn");

        if (mIsWorldWarp) {
            rc::startPuppetAction(getPlayerPuppet(), "DokanIn");
        }
    }

    sead::Vector3f trans = rc::getPuppetTrans(getPlayerPuppet());

    if (al::isGreaterEqualStep(this, 45)) {
        if (mIsSingleMode && al::isStep(this, 45)) {
            // same as startCameraTarget(), written out (the helper changes the stack layout)
            sead::Vector3f playerTrans = rc::getPuppetTrans(getPlayerPuppet());
            const sead::Vector3f& cameraPos = al::getCameraPos_RS(mHost, 0);
            f32 dx = playerTrans.x - cameraPos.x;
            f32 dz = playerTrans.z - cameraPos.z;
            f32 distance = nerd::sqrt(dx * dx + dz * dz);
            mCameraTarget->setTrans(playerTrans);
            mCameraTarget->setRequestDistance(distance);
            mCameraTarget->onTarget();
            mIsCameraTargetOn = true;
        }

        trans.y += -3.0f;
        rc::setPuppetTrans(getPlayerPuppet(), trans);
    }

    if (al::isStep(this, 85)) {
        endCameraTarget();

        if (mIsWorldWarp) {
            al::setNerve(this, &NrvDokanBindPuppeteerWaitStartWorldWarp);
        } else {
            al::setNerve(this, &NrvDokanBindPuppeteerWaitStartWarp);
        }
    }
}

/// Push the player sideways into a pipe lying on its side.
void DokanBindPuppeteer::exeDokanInSideMove() {
    if (al::isFirstStep(this)) {
        al::startSe(mDokan, "PgIn");

        if (mIsWorldWarp) {
            rc::startPuppetAction(getPlayerPuppet(), "DokanSideIn");
        }
    }

    if (al::isStep(this, 56)) {
        rc::hidePuppet(getPlayerPuppet());
    }

    sead::Vector3f trans = rc::getPuppetTrans(getPlayerPuppet());

    if (al::isLessEqualStep(this, 15)) {
        sead::Vector3f move = rc::getPuppetFrontVec(getPlayerPuppet());
        move *= 8.0f;
        IUsePlayerPuppet* puppet = getPlayerPuppet();
        trans = move + trans;
        rc::setPuppetTrans(puppet, trans);
        auto* player = getPuppetPlayerActor(getPlayerPuppet());

        if (player != nullptr) {
            player->getProperty()->_78 = 0.9f;
        }
    }

    if (al::isStep(this, 0)) {
        auto* player = getPuppetPlayerActor(getPlayerPuppet());

        if (player != nullptr) {
            player->getPlayer()->getEquipmentDirector()->hideCrown();
            player->getPlayer()->getEquipmentDirector()->hideHeadgear();
        }
    }

    if (al::isStep(this, 5)) {
        rc::hidePuppet(getPlayerPuppet());
    }

    if (al::isStep(this, 85)) {
        rc::hidePuppet(getPlayerPuppet());

        if (mIsWorldWarp) {
            al::setNerve(this, &NrvDokanBindPuppeteerWaitStartWorldWarp);
        } else {
            al::setNerve(this, &NrvDokanBindPuppeteerWaitStartWarp);
        }
    }
}

/// Let the hip-dropping player fall into the pipe.
void DokanBindPuppeteer::exeDokanInHipDrop() {
    if (al::isFirstStep(this)) {
        al::startHitReaction(mDokan, "HipDropIn");

        if (mIsSingleMode) {
            startCameraTarget();
        }
    }

    sead::Vector3f startTrans;
    mStartMtx.getTranslation(startTrans);
    sead::Vector3f endTrans;
    mEndMtx.getTranslation(endTrans);
    f32 speed = sead::Mathf::abs(mVelocity.y);
    s32 fallStep = sead::Mathf::ceil(sead::Mathf::abs(endTrans.y - startTrans.y) / speed) - 1;

    if (al::isStep(this, fallStep)) {
        rc::hidePuppet(getPlayerPuppet());
    }

    if (al::isLessEqualStep(this, fallStep)) {
        updateHipDropPos();
    }

    if (al::isStep(this, 85)) {
        endCameraTarget();

        if (mIsWorldWarp) {
            al::setNerve(this, &NrvDokanBindPuppeteerWaitStartWorldWarp);
        } else {
            al::setNerve(this, &NrvDokanBindPuppeteerWaitStartWarp);
        }
    }
}

/// Let the rolling player roll into a pipe lying on its side.
void DokanBindPuppeteer::exeDokanInRolling() {
    if (al::isFirstStep(this)) {
        al::startHitReaction(mDokan, "RollingIn");
    }

    f32 speed = sead::Mathf::abs(mSpeedH);

    if (al::isStep(this, 15)) {
        rc::hidePuppet(getPlayerPuppet());
    }

    if (al::isLessEqualStep(this, 2)) {
        f32 rate = al::calcNerveEaseOutRate(this, 2);
        sead::Matrix34f mtx;
        al::blendMtx(&mtx, mStartMtx, mEndMtx, rate);
        sead::Vector3f trans;
        mtx.getTranslation(trans);
        sead::Vector3f front;
        mtx.getBase(front, 2);
        rc::setPuppetFrontVec(getPlayerPuppet(), front);
        trans.y += 30.0f;
        rc::setPuppetTrans(getPlayerPuppet(), trans);
    }

    if (al::isLessEqualStep(this, 15) && al::isGreaterStep(this, 3)) {
        sead::Vector3f trans = rc::getPuppetTrans(getPlayerPuppet());
        sead::Vector3f move = rc::getPuppetFrontVec(getPlayerPuppet());
        move *= speed;
        IUsePlayerPuppet* puppet = getPlayerPuppet();
        trans = move + trans;
        rc::setPuppetTrans(puppet, trans);
    }

    if (al::isStep(this, 85)) {
        al::setNerve(this, &NrvDokanBindPuppeteerWaitStartWarp);
    }
}

/// Keep the player hidden in the pipe until the warp starts.
void DokanBindPuppeteer::exeWaitStartWarp() {
    if (al::isFirstStep(this)) {
        rc::hidePuppet(getPlayerPuppet());
    }
}

/// Carry the player to the pipe with the warp light.
void DokanBindPuppeteer::exeForceBindWarp() {
    if (al::isFirstStep(this)) {
        rc::hidePuppet(getPlayerPuppet());
        rc::startPuppetAction(getPlayerPuppet(), "WarpWait");
    }

    if (mWarpEffect->isEnd()) {
        al::setNerve(this, &NrvDokanBindPuppeteerForceBindWarpEnd);
    }
}

/// Wait a moment after the warp light reached the pipe.
void DokanBindPuppeteer::exeForceBindWarpEnd() {
    if (al::isGreaterEqualStep(this, 20)) {
        if (mIsWorldWarp) {
            al::setNerve(this, &NrvDokanBindPuppeteerWaitStartWorldWarp);
        } else {
            al::setNerve(this, &NrvDokanBindPuppeteerWaitStartWarp);
        }
    }
}

/// Keep the player hidden in the destination pipe until it may come out.
void DokanBindPuppeteer::exeWaitStartDokanOut() {
    if (al::isFirstStep(this)) {
        rc::hidePuppet(getPlayerPuppet());
        rc::startPuppetAction(getPlayerPuppet(), getDokanOutActionName());
        rc::setPuppetActionRate(getPlayerPuppet(), 0.0f);
        rc::invalidatePuppetDynamics(getPlayerPuppet());
        auto* player = getPuppetPlayerActor(getPlayerPuppet());

        if (player != nullptr && mIsSide) {
            player->getProperty()->_78 = 1.0f;
            player->getPlayer()->getEquipmentDirector()->showCrown();
            player->getPlayer()->getEquipmentDirector()->showHeadgear();
        }
    }
}

/// Move the player out of the destination pipe and release it.
void DokanBindPuppeteer::exeDokanOutUp() {
    if (al::isFirstStep(this)) {
        rc::showPuppet(getPlayerPuppet());
        rc::startPuppetAction(getPlayerPuppet(), getDokanOutActionName());
        al::startSe(mDestDokan, "PgOut");

        if (mIsSingleMode && mIsHipDrop && mIsCameraTargetOn) {
            mCameraTarget->offTarget();
            mCameraTarget->setRequestDistance(-1.0f);
            mIsCameraTargetOn = false;
        }

        auto* player = getPuppetPlayerActor(getPlayerPuppet());

        if (player != nullptr) {
            player->getProperty()->_78 = 1.0f;
            player->getPlayer()->getEquipmentDirector()->showCrown();
        }
    }

    if (rc::isPuppetActionEnd(getPlayerPuppet())) {
        if (mIsSide) {
            rc::showPuppetShadow(getPlayerPuppet());
        }

        rc::showPuppetSilhouette(getPlayerPuppet());

        if (mDestDokan->isUpsideDown()) {
            rc::startPuppetAction(getPlayerPuppet(), "DokanJump");
            rc::setPuppetVelocity(getPlayerPuppet(), sead::Vector3f::zero);
            endBind(mDestDokan->isControlPlayerOut() ? &cBindEndParamDokanOutUpsideDownControl :
                                                       &cBindEndParamDokanOutUpsideDown);
        } else {
            rc::startPuppetAction(getPlayerPuppet(), "DokanJump");
            sead::Matrix34f mtx = sead::Matrix34f::ident;
            al::makeMtxFrontUp(&mtx, rc::getPuppetFrontVec(getPlayerPuppet()),
                               rc::getPuppetUpVec(getPlayerPuppet()));
            sead::Vector3f velocity = mtx * mOutOffset;
            rc::setPuppetVelocity(getPlayerPuppet(), velocity);
            endBind(mDestDokan->isIgnorePlayerControlAfterExit() ?
                        &cBindEndParamDokanOutIgnoreControl :
                        &cBindEndParamDokanOut);
        }
    }
}

/// Wait for the warp to another world (the scene change ends the bind).
void DokanBindPuppeteer::exeWaitStartWorldWarp() {}
