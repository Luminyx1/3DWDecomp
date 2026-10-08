#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Scene/ISceneObj.hpp"
#include "Raidon/RaidonActor.hpp"

/// Plessie in her surfing form; registered as a scene object while she exists.
class RaidonSurf : public RaidonActor, public al::ISceneObj {
public:
    virtual const sead::Vector3f& getDashBlurCenter() const;
    virtual const sead::Vector3f& getBaseFrontDir() const;
    virtual const sead::Vector3f& getGroundUpVec() const;
    virtual const sead::Quatf& getBaseQuat() const;
    virtual const sead::Vector3f& getGoalPosition() const;
    virtual bool isEnableGoalPosition() const;
    virtual f32 getHandle() const;
    virtual f32 getAccel() const;
    virtual f32 getRotateY() const;
    virtual void updatePuppetInput();
    virtual void updateHandleAndAccel();
    virtual void updateGroundUpVec();
    virtual void updateOnGround();
    virtual void updateMatrialCode();
    virtual void updateStart();
    virtual void updateRide();
    virtual void clearGroundCount();
    virtual bool isAllGetOffPlayer() const;
    virtual void startPuppetSe(const char* pName);
    virtual bool isNotChangeBgm() const;
    virtual const char* getMaterialCode();
    virtual f32 getPuppetInputStickY();

    void forceSpawn(bool isForce);
    void getClosestSpawnPosFront(sead::Vector3f pos, sead::Vector3f* pSpawnPos,
                                 sead::Vector3f* pSpawnFront);
    void plessieChaseHitBells(al::HitSensor* pSensor);
    void updateSpawns(bool);
    bool isUnderwater();
    void doJump(bool isPerfect);
    void doDive(bool);
    void toggleGameWindow(bool isShow);

    /** @return Whether Plessie is standing on ground she slides down. */
    bool isOnSlideGround() const { return mIsOnSlideGround; }

    /** @return Height of the water surface Plessie is swimming in. */
    s32 getWaterSurfaceY() const { return mWaterSurfaceY; }

    /** @return Depth below which Plessie starts surfacing again. */
    f32 getDiveDepthLimit() const { return mDiveDepthLimit; }

    /** @return Current depth below the water surface. */
    f32 getDiveDepth() const { return mDiveDepth; }

private:
    u8 _178[0x29d - 0x178];
    bool mIsOnSlideGround;  // 0x29d
    u8 _29e[0x354 - 0x29e];
    s32 mWaterSurfaceY;     // 0x354
    f32 mDiveDepthLimit;    // 0x358
    f32 mDiveDepth;         // 0x35c
};
