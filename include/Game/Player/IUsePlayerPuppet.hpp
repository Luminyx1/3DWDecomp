#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace al {
class HitSensor;
}

class IUsePlayerCollision;
class IUsePlayerInput;
class IUsePlayerInputArranger;
class PlayerBindEndParam;

/// Lets an object that binds the player drive it directly (implemented by PlayerPuppet).
class IUsePlayerPuppet {
public:
    virtual void start(al::HitSensor*, al::HitSensor*) = 0;
    virtual void end() = 0;
    virtual void setTrans(const sead::Vector3f&) = 0;
    virtual void setVelocity(const sead::Vector3f&) = 0;
    virtual void setFrontVec(const sead::Vector3f&) = 0;
    virtual void setUpVec(const sead::Vector3f&) = 0;
    virtual const sead::Vector3f& getTrans() const = 0;
    virtual const sead::Vector3f& getVelocity() const = 0;
    virtual const sead::Vector3f& getFrontVec() const = 0;
    virtual const sead::Vector3f& getUpVec() const = 0;
    virtual const IUsePlayerInput* getInput() const = 0;
    virtual IUsePlayerCollision* getCollider() const = 0;
    virtual IUsePlayerInputArranger* getInputArranger() = 0;
    virtual void enableAlphaCtrl(bool) = 0;
    virtual void validateSubAction() = 0;
    virtual void invalidateSubAction() = 0;
    virtual void forceEndSubAction() = 0;
    virtual void validateSubActionUpper() = 0;
    virtual void invalidateSubActionUpper() = 0;
    virtual void validateSubActionLower() = 0;
    virtual void invalidateSubActionLower() = 0;
    virtual void requestGlideInhibit(s32) = 0;
    virtual void validateGetItem() = 0;
    virtual void invalidateGetItem() = 0;
    virtual void validateSensors() = 0;
    virtual void invalidateSensors() = 0;
    virtual void validateCollisionMsg() = 0;
    virtual void invalidateCollisionMsg() = 0;
    virtual void startAction(const sead::SafeString&) const = 0;
    virtual void setActionRate(f32) const = 0;
    virtual bool isActionEnd() const = 0;
    virtual bool isAction(const sead::SafeString&) const = 0;
    virtual f32 getActionFrameMax() const = 0;
    virtual f32 getActionFrameMax(const sead::SafeString&) const = 0;
    virtual void setActionFrame(f32) = 0;
    virtual f32 getActionFrame() const = 0;
    virtual void setBlendAnimWeight(f32, f32, f32, f32, f32, f32) = 0;
    virtual f32 getBlendAnimWeight(u32) const = 0;
    virtual void startSe(const sead::SafeString&) const = 0;
    virtual void stopAllSe(s32) const = 0;
    virtual void tryDeleteEmitterAndParticleAll() const = 0;
    virtual void damage() = 0;
    virtual void checkDeathMapCode() = 0;
    virtual void hide() = 0;
    virtual void show() = 0;
    virtual bool isHidden() const = 0;
    virtual void hideSilhouette() = 0;
    virtual void showSilhouette() = 0;
    virtual void hideRain() = 0;
    virtual void hideShadow() = 0;
    virtual void showShadow() = 0;
    virtual void setBindEndOnGround() = 0;
    virtual void setBindEndSquat() = 0;
    virtual void setBindEndParam(const PlayerBindEndParam*) = 0;
    virtual al::HitSensor* getHostSensor() = 0;
    virtual al::HitSensor* getMsgTargetSensor() = 0;
    virtual void resetAirLimitedAction() = 0;
    virtual void updateMaterial(bool) = 0;
    virtual void forceMaterial(const char*) = 0;
};
