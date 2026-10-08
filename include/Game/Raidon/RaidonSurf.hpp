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

    void forceSpawn(bool isForce);
    void updateSpawns(bool);
};
