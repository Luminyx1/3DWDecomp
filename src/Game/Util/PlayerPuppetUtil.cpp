#include "Util/PlayerPuppetUtil.hpp"
#include "Library/Layout/LayoutActor.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Message/MessageHolder.hpp"
#include "Player/IUsePlayerCollision.hpp"
#include "Player/IUsePlayerInput.hpp"
#include "Player/IUsePlayerInputArranger.hpp"
#include "Player/IUsePlayerPuppet.hpp"
#include "System/GameDataConst.hpp"
#include "Util/PlayerUtil.hpp"

// PlayerActor has no header yet; only the member used by this unit is declared here.

/**
 * @brief The player actor.
 */
class PlayerActor : public al::LiveActor {
public:
    IUsePlayerPuppet* getPlayerPuppet();
};

namespace {

/**
 * @brief Get the player actor owning a sensor.
 * @param pSensor Sensor of the player.
 * @return The player actor.
 */
inline PlayerActor* getSensorPlayerActor(const al::HitSensor* pSensor) {
    return static_cast<PlayerActor*>(al::getSensorHost(pSensor));
}

} // namespace

namespace rc {

/**
 * @brief Get the display name message of a player character.
 * @param pActor Layout actor whose message system is used.
 * @param characterType Player character to get the name of.
 * @return The character name message.
 */
const char16_t* getPlayerCharacterMessageName(const al::LayoutActor* pActor, s32 characterType) {
    return al::getSystemMessageString(pActor, "PlayerName",
                                      GameDataConst::getPlayerCharacterName(characterType));
}

/**
 * @brief Start driving the player of a sensor as a puppet.
 * @param pBinderSensor Sensor of the object binding the player.
 * @param pPlayerSensor Sensor of the player to bind.
 * @return The player's puppet.
 */
IUsePlayerPuppet* startPuppet(al::HitSensor* pBinderSensor, al::HitSensor* pPlayerSensor) {
    IUsePlayerPuppet* puppet = getSensorPlayerActor(pPlayerSensor)->getPlayerPuppet();
    puppet->start(pBinderSensor, pPlayerSensor);
    return puppet;
}

/**
 * @brief End a puppet bind and clear the puppet pointer.
 * @param ppPuppet Puppet to release; set to nullptr.
 * @param pParam How the bind ended (may be nullptr).
 */
void endBindAndPuppetNull(IUsePlayerPuppet** ppPuppet, const PlayerBindEndParam* pParam) {
    if (pParam != nullptr) {
        (*ppPuppet)->setBindEndParam(pParam);
    }

    (*ppPuppet)->end();
    *ppPuppet = nullptr;
}

/**
 * @brief End a puppet bind on the ground and clear the puppet pointer.
 * @param ppPuppet Puppet to release; set to nullptr.
 */
void endBindOnGroundAndPuppetNull(IUsePlayerPuppet** ppPuppet) {
    (*ppPuppet)->setBindEndOnGround();
    (*ppPuppet)->end();
    *ppPuppet = nullptr;
}

/**
 * @brief End a puppet bind in the squat pose and clear the puppet pointer.
 * @param ppPuppet Puppet to release; set to nullptr.
 */
void endBindSquatAndPuppetNull(IUsePlayerPuppet** ppPuppet) {
    (*ppPuppet)->setBindEndSquat();
    (*ppPuppet)->end();
    *ppPuppet = nullptr;
}

/**
 * @brief End a puppet bind by making the player fall into the abyss, and clear the puppet pointer.
 * @param ppPuppet Puppet to release; set to nullptr.
 */
void endBindForceAbyssAndPuppetNull(IUsePlayerPuppet** ppPuppet) {
    al::HitSensor* hostSensor = (*ppPuppet)->getHostSensor();
    al::HitSensor* targetSensor = (*ppPuppet)->getMsgTargetSensor();
    (*ppPuppet)->end();
    *ppPuppet = nullptr;
    al::sendMsgForceAbyss(targetSensor, hostSensor);
}

/**
 * @brief Set the puppet's position.
 * @param pPuppet Puppet.
 * @param rTrans New position.
 */
void setPuppetTrans(IUsePlayerPuppet* pPuppet, const sead::Vector3f& rTrans) {
    pPuppet->setTrans(rTrans);
}

/**
 * @brief Set the puppet's velocity.
 * @param pPuppet Puppet.
 * @param rVelocity New velocity.
 */
void setPuppetVelocity(IUsePlayerPuppet* pPuppet, const sead::Vector3f& rVelocity) {
    pPuppet->setVelocity(rVelocity);
}

/**
 * @brief Set the puppet's front direction.
 * @param pPuppet Puppet.
 * @param rFront New front direction.
 */
void setPuppetFrontVec(IUsePlayerPuppet* pPuppet, const sead::Vector3f& rFront) {
    pPuppet->setFrontVec(rFront);
}

/**
 * @brief Set the puppet's up direction.
 * @param pPuppet Puppet.
 * @param rUp New up direction.
 */
void setPuppetUpVec(IUsePlayerPuppet* pPuppet, const sead::Vector3f& rUp) {
    pPuppet->setUpVec(rUp);
}

/**
 * @brief Set the puppet's orientation from a quaternion.
 * @param pPuppet Puppet.
 * @param rQuat New orientation.
 */
void setPuppetQuat(IUsePlayerPuppet* pPuppet, const sead::Quatf& rQuat) {
    sead::Vector3f up;
    al::calcQuatUp(&up, rQuat);
    sead::Vector3f front;
    al::calcQuatFront(&front, rQuat);
    pPuppet->setUpVec(up);
    pPuppet->setFrontVec(front);
}

/**
 * @brief Set the puppet's orientation and position from a matrix.
 * @param pPuppet Puppet.
 * @param pMtx New pose.
 */
void setPuppetMtx(IUsePlayerPuppet* pPuppet, const sead::Matrix34f* pMtx) {
    sead::Vector3f up;
    pMtx->getBase(up, 1);
    sead::Vector3f front;
    pMtx->getBase(front, 2);
    sead::Vector3f trans;
    pMtx->getTranslation(trans);
    pPuppet->setUpVec(up);
    pPuppet->setFrontVec(front);
    pPuppet->setTrans(trans);
}

/**
 * @brief Get the puppet's position.
 * @param pPuppet Puppet.
 * @return The position.
 */
const sead::Vector3f& getPuppetTrans(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getTrans();
}

/**
 * @brief Get the puppet's velocity.
 * @param pPuppet Puppet.
 * @return The velocity.
 */
const sead::Vector3f& getPuppetVelocity(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getVelocity();
}

/**
 * @brief Get the puppet's front direction.
 * @param pPuppet Puppet.
 * @return The front direction.
 */
const sead::Vector3f& getPuppetFrontVec(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getFrontVec();
}

/**
 * @brief Get the puppet's up direction.
 * @param pPuppet Puppet.
 * @return The up direction.
 */
const sead::Vector3f& getPuppetUpVec(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getUpVec();
}

/**
 * @brief Start an action (animation) on the puppet.
 * @param pPuppet Puppet.
 * @param rActionName Action to start.
 */
void startPuppetAction(IUsePlayerPuppet* pPuppet, const sead::SafeString& rActionName) {
    pPuppet->startAction(rActionName);
}

/**
 * @brief Set the playback rate of the puppet's action.
 * @param pPuppet Puppet.
 * @param rate New playback rate.
 */
void setPuppetActionRate(IUsePlayerPuppet* pPuppet, f32 rate) {
    pPuppet->setActionRate(rate);
}

/**
 * @brief Check whether the puppet's action has ended.
 * @param pPuppet Puppet.
 * @return Whether the action has ended.
 */
bool isPuppetActionEnd(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->isActionEnd();
}

/**
 * @brief Check whether the puppet is playing an action.
 * @param pPuppet Puppet.
 * @param rActionName Action to check.
 * @return Whether that action is playing.
 */
bool isPuppetAction(const IUsePlayerPuppet* pPuppet, const sead::SafeString& rActionName) {
    return pPuppet->isAction(rActionName);
}

/**
 * @brief Set the current frame of the puppet's action.
 * @param pPuppet Puppet.
 * @param frame New frame.
 */
void setPuppetActionFrame(IUsePlayerPuppet* pPuppet, f32 frame) {
    pPuppet->setActionFrame(frame);
}

/**
 * @brief Get the current frame of the puppet's action.
 * @param pPuppet Puppet.
 * @return The current frame.
 */
f32 getPuppetActionFrame(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getActionFrame();
}

/**
 * @brief Get the last frame of the puppet's action.
 * @param pPuppet Puppet.
 * @return The last frame.
 */
f32 getPuppetActionFrameMax(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getActionFrameMax();
}

/**
 * @brief Get the last frame of one of the puppet's actions.
 * @param pPuppet Puppet.
 * @param rActionName Action to query.
 * @return The last frame of that action.
 */
f32 getPuppetActionFrameMax(const IUsePlayerPuppet* pPuppet, const sead::SafeString& rActionName) {
    return pPuppet->getActionFrameMax(rActionName);
}

/**
 * @brief Set the weights of the puppet's blended animations.
 * @param pPuppet Puppet.
 * @param weight0 Weight of the first animation.
 * @param weight1 Weight of the second animation.
 * @param weight2 Weight of the third animation.
 * @param weight3 Weight of the fourth animation.
 * @param weight4 Weight of the fifth animation.
 * @param weight5 Weight of the sixth animation.
 */
void setPuppetBlendAnimWeight(IUsePlayerPuppet* pPuppet, f32 weight0, f32 weight1, f32 weight2,
                              f32 weight3, f32 weight4, f32 weight5) {
    pPuppet->setBlendAnimWeight(weight0, weight1, weight2, weight3, weight4, weight5);
}

/**
 * @brief Get the weight of one of the puppet's blended animations.
 * @param pPuppet Puppet.
 * @param index Index of the blended animation.
 * @return The weight.
 */
f32 getPuppetBlendAnimWeight(const IUsePlayerPuppet* pPuppet, u32 index) {
    return pPuppet->getBlendAnimWeight(index);
}

/**
 * @brief Check whether the puppet's stick is tilted.
 * @param pPuppet Puppet.
 * @return Whether the stick is tilted.
 */
bool isPuppetStickOn(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->isStickOn();
}

/**
 * @brief Get the controller port driving the puppet.
 * @param pPuppet Puppet.
 * @return The controller port.
 */
s32 getPuppetInputPort(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->getPort();
}

/**
 * @brief Get the horizontal tilt of the puppet's stick.
 * @param pPuppet Puppet.
 * @return The horizontal tilt.
 */
f32 getPuppetStickX(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->getStickX();
}

/**
 * @brief Get the vertical tilt of the puppet's stick.
 * @param pPuppet Puppet.
 * @return The vertical tilt.
 */
f32 getPuppetStickY(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->getStickY();
}

/**
 * @brief Get the puppet's stick direction in world space, snapped to the camera axes.
 * @param pPuppet Puppet.
 * @return The world move direction.
 */
const sead::Vector3f& getPuppetStickWorldWithSnap(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->getMoveVec();
}

/**
 * @brief Get the puppet's stick direction in world space, without snapping.
 * @param pPuppet Puppet.
 * @return The world move direction.
 */
const sead::Vector3f& getPuppetStickWorldWithoutSnap(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->getMoveVecNoArrange();
}

/**
 * @brief Check whether the puppet's jump was triggered (including preceding input).
 * @param pPuppet Puppet.
 * @return Whether the jump was triggered.
 */
bool isPuppetTrigJumpButton(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->isJumpTrigOn();
}

/**
 * @brief Check whether the puppet's jump button was pressed this very frame.
 * @param pPuppet Puppet.
 * @return Whether the jump button was pressed this frame.
 */
bool isPuppetTrigJumpButtonWithoutPrecedeInput(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->getFrameFromLastJumpTrig() == 0;
}

/**
 * @brief Check whether the puppet's jump button is held.
 * @param pPuppet Puppet.
 * @return Whether the jump button is held.
 */
bool isPuppetHoldJumpButton(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->isJumpButtonOn();
}

/**
 * @brief Check whether the puppet's squat button is held.
 * @param pPuppet Puppet.
 * @return Whether the squat button is held.
 */
bool isPuppetHoldSquatButton(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->isSquatButtonOn();
}

/**
 * @brief Check whether the puppet's squat button was pressed.
 * @param pPuppet Puppet.
 * @return Whether the squat button was pressed.
 */
bool isPuppetTrigSquatButton(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->isSquatTrigOn();
}

/**
 * @brief Check whether the puppet's dash button was pressed.
 * @param pPuppet Puppet.
 * @return Whether the dash button was pressed.
 */
bool isPuppetTrigDashButton(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->isDashTrigOn();
}

/**
 * @brief Check whether the puppet's dash button is held.
 * @param pPuppet Puppet.
 * @return Whether the dash button is held.
 */
bool isPuppetHoldDashButton(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->isDashButtonOn();
}

/**
 * @brief Get the number of frames since the puppet's jump was last triggered.
 * @param pPuppet Puppet.
 * @return The frames since the last jump trigger.
 */
s32 getPuppetTrigJumpFrame(const IUsePlayerPuppet* pPuppet) {
    return pPuppet->getInput()->getFrameFromLastJumpTrig();
}

/**
 * @brief Compute the puppet's orientation as a quaternion.
 * @param pQuat Receives the orientation.
 * @param pPuppet Puppet.
 */
void calcPuppetQuat(sead::Quatf* pQuat, const IUsePlayerPuppet* pPuppet) {
    al::makeQuatUpFront(pQuat, pPuppet->getUpVec(), pPuppet->getFrontVec());
}

/**
 * @brief Compute the puppet's orientation as a quaternion and get its position.
 * @param pQuat Receives the orientation.
 * @param pTrans Receives the position.
 * @param pPuppet Puppet.
 */
void calcPuppetQuatAndTrans(sead::Quatf* pQuat, sead::Vector3f* pTrans,
                            const IUsePlayerPuppet* pPuppet) {
    calcPuppetQuat(pQuat, pPuppet);
    pTrans->set(pPuppet->getTrans());
}

/**
 * @brief Play a sound effect on the puppet.
 * @param pPuppet Puppet.
 * @param rSeName Sound effect to play.
 */
void startPuppetSe(const IUsePlayerPuppet* pPuppet, const sead::SafeString& rSeName) {
    pPuppet->startSe(rSeName);
}

/**
 * @brief Force the material code the puppet stands on.
 * @param pPuppet Puppet.
 * @param rMaterialCode Material code to use.
 */
void changePuppetMaterialCode(IUsePlayerPuppet* pPuppet, const sead::SafeString& rMaterialCode) {
    pPuppet->forceMaterial(rMaterialCode.cstr());
}

/**
 * @brief Damage the puppet.
 * @param pPuppet Puppet.
 */
void damagePuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->damage();
}

/**
 * @brief Damage the puppet unless the player is currently immune to damage.
 * @param pPuppet Puppet.
 */
void tryDamagePuppet(IUsePlayerPuppet* pPuppet) {
    if (pPuppet->getMsgTargetSensor() == nullptr) {
        return;
    }

    al::LiveActor* player = al::getSensorHost(pPuppet->getMsgTargetSensor());
    if (isPlayerDamageInvalid(player) || isPlayerInvincible(player)) {
        return;
    }

    pPuppet->damage();
}

/**
 * @brief Check whether the puppet stands on a deadly map code.
 * @param pPuppet Puppet.
 */
void checkDeathMapCodePuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->checkDeathMapCode();
}

/**
 * @brief Hide the puppet with its fur, shadow, silhouette and rain effects.
 * @param pPuppet Puppet.
 */
void hidePuppetAllParts(IUsePlayerPuppet* pPuppet) {
    hidePuppet(pPuppet);
    hidePuppetFur(pPuppet);
    hidePuppetShadow(pPuppet);
    hidePuppetSilhouette(pPuppet);
    hidePuppetRain(pPuppet);
}

/**
 * @brief Hide the puppet's model.
 * @param pPuppet Puppet.
 */
void hidePuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->hide();
}

/**
 * @brief Hide the puppet's fur.
 * @param pPuppet Puppet.
 */
void hidePuppetFur(IUsePlayerPuppet* pPuppet) {
    al::HitSensor* sensor = pPuppet->getMsgTargetSensor();
    if (sensor != nullptr) {
        hidePlayerFur(sensor);
    }
}

/**
 * @brief Hide the puppet's shadow.
 * @param pPuppet Puppet.
 */
void hidePuppetShadow(IUsePlayerPuppet* pPuppet) {
    pPuppet->hideShadow();
}

/**
 * @brief Hide the puppet's silhouette.
 * @param pPuppet Puppet.
 */
void hidePuppetSilhouette(IUsePlayerPuppet* pPuppet) {
    pPuppet->hideSilhouette();
}

/**
 * @brief Hide the rain effect on the puppet.
 * @param pPuppet Puppet.
 */
void hidePuppetRain(IUsePlayerPuppet* pPuppet) {
    pPuppet->hideRain();
}

/**
 * @brief Show the puppet with its fur, shadow and silhouette.
 * @param pPuppet Puppet.
 */
void showPuppetAllParts(IUsePlayerPuppet* pPuppet) {
    showPuppet(pPuppet);
    showPuppetFur(pPuppet);
    showPuppetShadow(pPuppet);
    showPuppetSilhouette(pPuppet);
}

/**
 * @brief Show the puppet's model.
 * @param pPuppet Puppet.
 */
void showPuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->show();
}

/**
 * @brief Show the puppet's fur.
 * @param pPuppet Puppet.
 */
void showPuppetFur(IUsePlayerPuppet* pPuppet) {
    al::HitSensor* sensor = pPuppet->getMsgTargetSensor();
    if (sensor != nullptr) {
        showPlayerFur(sensor);
    }
}

/**
 * @brief Show the puppet's shadow.
 * @param pPuppet Puppet.
 */
void showPuppetShadow(IUsePlayerPuppet* pPuppet) {
    pPuppet->showShadow();
}

/**
 * @brief Show the puppet's silhouette.
 * @param pPuppet Puppet.
 */
void showPuppetSilhouette(IUsePlayerPuppet* pPuppet) {
    pPuppet->showSilhouette();
}

/**
 * @brief Check whether the puppet's model is hidden.
 * @param pPuppet Puppet.
 * @return Whether the model is hidden.
 */
bool isPuppetHidden(IUsePlayerPuppet* pPuppet) {
    return pPuppet->isHidden();
}

/**
 * @brief Move the puppet by its velocity with simple collision.
 * @param pPuppet Puppet.
 */
void moveSimplePuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->getCollider()->moveSimple(true);
}

/**
 * @brief Solve the puppet's collision while in the air.
 * @param pPuppet Puppet.
 */
void solveAirPuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->getCollider()->solveAir();
}

/**
 * @brief Solve the puppet's collision while in the air, ignoring floors.
 * @param pPuppet Puppet.
 */
void solveAirNoFloorPuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->getCollider()->solveAirNoFloor();
}

/**
 * @brief Snap the puppet to the ground below it.
 * @param pPuppet Puppet.
 */
void snapGroundPuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->getCollider()->snapGround();
}

/**
 * @brief Snap the puppet to nearby walls.
 * @param pPuppet Puppet.
 * @param isSnapAll Whether every wall is snapped to.
 */
void snapWallPuppet(IUsePlayerPuppet* pPuppet, bool isSnapAll) {
    pPuppet->getCollider()->snapWall(isSnapAll);
}

/**
 * @brief Check whether the puppet is on a floor.
 * @param pPuppet Puppet.
 * @return Whether the puppet is on a floor.
 */
bool isOnFloorPuppet(IUsePlayerPuppet* pPuppet) {
    return pPuppet->getCollider()->isOnFloor();
}

/**
 * @brief Check whether the puppet touches a ceiling.
 * @param pPuppet Puppet.
 * @return Whether the puppet touches a ceiling.
 */
bool isOnCeilingPuppet(IUsePlayerPuppet* pPuppet) {
    return pPuppet->getCollider()->isOnCeiling();
}

/**
 * @brief Check whether the puppet touches a wall on any side.
 * @param pPuppet Puppet.
 * @return Whether the puppet touches a wall.
 */
bool isOnWallPuppet(IUsePlayerPuppet* pPuppet) {
    IUsePlayerCollision* collider = pPuppet->getCollider();
    return collider->isOnFrontWall() || collider->isOnBackWall() || collider->isOnRightWall() ||
           collider->isOnLeftWall();
}

/**
 * @brief Check whether the puppet touches any collision.
 * @param pPuppet Puppet.
 * @return Whether the puppet touches a floor, wall or ceiling.
 */
bool isCollidedPuppet(IUsePlayerPuppet* pPuppet) {
    IUsePlayerCollision* collider = pPuppet->getCollider();
    return collider->isOnFloor() || collider->isOnFrontWall() || collider->isOnBackWall() ||
           collider->isOnCeiling() || collider->isOnRightWall() || collider->isOnLeftWall();
}

/**
 * @brief Get the normal of the ceiling the puppet touches.
 * @param pNormal Receives the ceiling normal (zero if there is none).
 * @param pPuppet Puppet.
 */
void getCeilingNormalPuppet(sead::Vector3f* pNormal, IUsePlayerPuppet* pPuppet) {
    IUsePlayerCollision* collider = pPuppet->getCollider();
    IUsePlayerCollision::Info info;
    collider->getCeilingInfo(&info);
    pNormal->set(info.mNormal);
}

/**
 * @brief Get the normal of the wall the puppet touches.
 * @param pNormal Receives the wall normal (zero if there is none).
 * @param pPuppet Puppet.
 */
void getWallNormalPuppet(sead::Vector3f* pNormal, IUsePlayerPuppet* pPuppet) {
    IUsePlayerCollision* collider = pPuppet->getCollider();
    IUsePlayerCollision::Info info;
    if (collider->isOnFrontWall()) {
        collider->getFrontWallInfo(&info);
    } else if (collider->isOnBackWall()) {
        collider->getBackWallInfo(&info);
    } else if (collider->isOnRightWall()) {
        collider->getRightWallInfo(&info);
    } else if (collider->isOnLeftWall()) {
        collider->getLeftWallInfo(&info);
    }

    pNormal->set(info.mNormal);
}

/**
 * @brief Clear the puppet's collision results.
 * @param pPuppet Puppet.
 */
void clearPuppetCollisionInfo(IUsePlayerPuppet* pPuppet) {
    clearPlayerCollisionInfo(pPuppet->getMsgTargetSensor());
}

/**
 * @brief Clear the external push applied to the puppet.
 * @param pPuppet Puppet.
 */
void clearPuppetExPush(IUsePlayerPuppet* pPuppet) {
    clearPlayerExPush(pPuppet->getMsgTargetSensor());
}

/**
 * @brief Get the sensor that receives messages for the puppet.
 * @param pPuppet Puppet.
 * @return The puppet's sensor.
 */
al::HitSensor* getPuppetSensor(IUsePlayerPuppet* pPuppet) {
    return pPuppet->getMsgTargetSensor();
}

/**
 * @brief Check whether a sensor belongs to the puppet's player.
 * @param pPuppet Puppet.
 * @param pSensor Sensor to check (may be nullptr).
 * @return Whether the sensor belongs to the puppet's player.
 */
bool isPuppetSensor(IUsePlayerPuppet* pPuppet, const al::HitSensor* pSensor) {
    al::HitSensor* sensor = pPuppet->getMsgTargetSensor();
    if (pSensor == nullptr || sensor == nullptr) {
        return false;
    }

    return al::getSensorHost(sensor) == al::getSensorHost(pSensor);
}

/**
 * @brief Allow the puppet's sub actions.
 * @param pPuppet Puppet.
 */
void validateSubActionPuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->validateSubAction();
}

/**
 * @brief Enable or disable the puppet's alpha control.
 * @param pPuppet Puppet.
 * @param isEnable Whether alpha control is enabled.
 */
void setPuppetAlphaCtrl(IUsePlayerPuppet* pPuppet, bool isEnable) {
    pPuppet->enableAlphaCtrl(isEnable);
}

/**
 * @brief Forbid the puppet's sub actions.
 * @param pPuppet Puppet.
 */
void invalidateSubActionPuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->invalidateSubAction();
}

/**
 * @brief Force the puppet's running sub action to end.
 * @param pPuppet Puppet.
 */
void forceEndSubActionPuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->forceEndSubAction();
}

/**
 * @brief Forbid the puppet's upper-body sub actions.
 * @param pPuppet Puppet.
 */
void invalidateUpperSubActionPuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->invalidateSubActionUpper();
}

/**
 * @brief Ignore the puppet's input for some frames.
 * @param pPuppet Puppet.
 * @param frame Number of frames to ignore input for.
 */
void invalidatePuppetInput(IUsePlayerPuppet* pPuppet, u32 frame) {
    pPuppet->getInputArranger()->invalidateFrame(frame);
}

/**
 * @brief Forbid the puppet from gliding for some frames.
 * @param pPuppet Puppet.
 * @param frame Number of frames gliding is forbidden for.
 */
void setPuppetGlideInhibitFrame(IUsePlayerPuppet* pPuppet, s32 frame) {
    pPuppet->requestGlideInhibit(frame);
}

/**
 * @brief Allow the puppet to collect items.
 * @param pPuppet Puppet.
 */
void validatePuppetGetItem(IUsePlayerPuppet* pPuppet) {
    pPuppet->validateGetItem();
}

/**
 * @brief Forbid the puppet from collecting items.
 * @param pPuppet Puppet.
 */
void invalidatePuppetGetItem(IUsePlayerPuppet* pPuppet) {
    pPuppet->invalidateGetItem();
}

/**
 * @brief Enable the puppet's sensors.
 * @param pPuppet Puppet.
 */
void validatePuppetSensors(IUsePlayerPuppet* pPuppet) {
    pPuppet->validateSensors();
}

/**
 * @brief Disable the puppet's sensors.
 * @param pPuppet Puppet.
 */
void invalidatePuppetSensors(IUsePlayerPuppet* pPuppet) {
    pPuppet->invalidateSensors();
}

/**
 * @brief Reset the puppet's once-per-air-time actions.
 * @param pPuppet Puppet.
 */
void resetAirLimitedActionPuppet(IUsePlayerPuppet* pPuppet) {
    pPuppet->resetAirLimitedAction();
}

/**
 * @brief Let the puppet's material follow the route pipe it travels through.
 * @param pPuppet Puppet.
 */
void validateMaterialRouteDokan(IUsePlayerPuppet* pPuppet) {
    pPuppet->updateMaterial(true);
}

/**
 * @brief Stop the puppet's material from following the route pipe.
 * @param pPuppet Puppet.
 */
void invalidateMaterialRouteDokan(IUsePlayerPuppet* pPuppet) {
    pPuppet->updateMaterial(false);
}

/**
 * @brief Delete every effect emitted by the puppet.
 * @param pPuppet Puppet.
 */
void tryDeleteEmitterAndParticleAll(IUsePlayerPuppet* pPuppet) {
    pPuppet->tryDeleteEmitterAndParticleAll();
}

/**
 * @brief Send the puppet's player a message making it fall into the abyss.
 * @param pPuppet Puppet.
 * @param pSender Sensor sending the message.
 * @return Whether the message was received.
 */
bool sendPuppetMsgForceAbyss(IUsePlayerPuppet* pPuppet, al::HitSensor* pSender) {
    return al::sendMsgForceAbyss(pPuppet->getMsgTargetSensor(), pSender);
}

/**
 * @brief Send the puppet's player a message making it dash after being flung from a pole.
 * @param pPuppet Puppet.
 * @param pSender Sensor sending the message.
 * @param frame Duration of the dash in frames.
 * @return Whether the message was received.
 */
bool sendPuppetMsgForceDash(IUsePlayerPuppet* pPuppet, al::HitSensor* pSender, s32 frame) {
    return sendMsgFlingPoleDash(pPuppet->getMsgTargetSensor(), pSender, frame);
}

/**
 * @brief Enable the puppet's dynamics (cloth/hair physics).
 * @param pPuppet Puppet.
 */
void validatePuppetDynamics(IUsePlayerPuppet* pPuppet) {
    validatePlayerDynamics(pPuppet->getMsgTargetSensor());
}

/**
 * @brief Disable the puppet's dynamics (cloth/hair physics).
 * @param pPuppet Puppet.
 */
void invalidatePuppetDynamics(IUsePlayerPuppet* pPuppet) {
    invalidatePlayerDynamics(pPuppet->getMsgTargetSensor());
}

/**
 * @brief Reset the puppet's dynamics (cloth/hair physics).
 * @param pPuppet Puppet.
 */
void resetPuppetDynamics(IUsePlayerPuppet* pPuppet) {
    resetPlayerDynamics(pPuppet->getMsgTargetSensor());
}

/**
 * @brief Allow the puppet's player to take damage.
 * @param pPuppet Puppet.
 */
void validatePuppetDamage(IUsePlayerPuppet* pPuppet) {
    validatePlayerDamage(pPuppet->getMsgTargetSensor());
}

/**
 * @brief Make the puppet's player immune to damage for some frames.
 * @param pPuppet Puppet.
 * @param frame Number of immune frames.
 */
void invalidatePuppetDamage(IUsePlayerPuppet* pPuppet, u32 frame) {
    invalidatePlayerDamage(getSensorPlayerActor(pPuppet->getMsgTargetSensor()), frame);
}

/**
 * @brief Allow the puppet's player to flash (after taking damage).
 * @param pPuppet Puppet.
 */
void validatePuppetFlash(IUsePlayerPuppet* pPuppet) {
    validatePlayerFlash(getSensorPlayerActor(pPuppet->getMsgTargetSensor()));
}

/**
 * @brief Stop the puppet's player from flashing.
 * @param pPuppet Puppet.
 */
void invalidatePuppetFlash(IUsePlayerPuppet* pPuppet) {
    invalidatePlayerFlash(getSensorPlayerActor(pPuppet->getMsgTargetSensor()));
}

/**
 * @brief Turn on a pre-pass light attached to the puppet's player.
 * @param pPuppet Puppet.
 * @param pName Name of the light.
 */
void appearPuppetPrePassLight(IUsePlayerPuppet* pPuppet, const char* pName) {
    appearPlayerPrePassLight(pPuppet->getMsgTargetSensor(), pName);
}

/**
 * @brief Turn off a pre-pass light attached to the puppet's player.
 * @param pPuppet Puppet.
 * @param pName Name of the light.
 */
void killPuppetPrePassLight(IUsePlayerPuppet* pPuppet, const char* pName) {
    killPlayerPrePassLight(pPuppet->getMsgTargetSensor(), pName);
}

/**
 * @brief Turn the puppet toward a direction around its up axis, by at most a given angle.
 * @param pPuppet Puppet.
 * @param rDir Direction to face.
 * @param maxDegree Largest rotation applied this call, in degrees.
 * @param endDegree Rotation at or below which the turn counts as finished, in degrees.
 * @return Whether the applied rotation was at most endDegree.
 */
bool faceToDirection(IUsePlayerPuppet* pPuppet, const sead::Vector3f& rDir, f32 maxDegree,
                     f32 endDegree) {
    sead::Vector3f front = pPuppet->getFrontVec();
    sead::Vector3f up = pPuppet->getUpVec();
    f32 degree = al::calcAngleOnPlaneDegree(front, rDir, up);

    if (degree > 0.0f && degree > maxDegree) {
        degree = maxDegree;
    } else if (degree < 0.0f && degree < -maxDegree) {
        degree = -maxDegree;
    }

    front.set(pPuppet->getFrontVec());
    up.set(pPuppet->getUpVec());
    al::rotateVectorDegree(&front, front, up, degree);
    pPuppet->setFrontVec(front);
    return sead::Mathf::abs(degree) <= endDegree;
}

} // namespace rc
