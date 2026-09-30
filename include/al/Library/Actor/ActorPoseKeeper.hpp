#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ActorPoseKeeperBase {
public:
    ActorPoseKeeperBase();

    virtual const sead::Vector3f& getRotate() const { return sead::Vector3f::zero; }
    virtual const sead::Vector3f& getScale() const { return sead::Vector3f::ones; }
    virtual const sead::Vector3f& getVelocity() const { return sead::Vector3f::zero; }
    virtual const sead::Vector3f& getFront() const { return sead::Vector3f::ez; }
    virtual const sead::Quatf& getQuat() const { return sead::Quatf::unit; }
    virtual const sead::Vector3f& getGravity() const { return sDefaultVelocity; }
    virtual const sead::Matrix34f& getMtx() const { return sead::Matrix34f::ident; }
    virtual sead::Vector3f* getRotatePtr() { return nullptr; }
    virtual sead::Vector3f* getScalePtr() { return nullptr; }
    virtual sead::Vector3f* getVelocityPtr() { return nullptr; }
    virtual sead::Vector3f* getFrontPtr() { return nullptr; }
    virtual sead::Quatf* getQuatPtr() { return nullptr; }
    virtual sead::Vector3f* getGravityPtr() { return nullptr; }
    virtual sead::Matrix34f* getMtxPtr() { return nullptr; }
    virtual void updatePoseRotate(const sead::Vector3f& rRotate) = 0;
    virtual void updatePoseQuat(const sead::Quatf& rQuat) = 0;
    virtual void updatePoseMtx(const sead::Matrix34f* pMtx) = 0;
    virtual void copyPose(const ActorPoseKeeperBase* pOther);
    virtual void calcBaseMtx(sead::Matrix34f* pMtx) const = 0;

    static sead::Vector3f sDefaultVelocity;

    sead::Vector3f mTranslation{0.0f, 0.0f, 0.0f};
};

class ActorPoseKeeperTRSV : public ActorPoseKeeperBase {
public:
    ActorPoseKeeperTRSV();

    const sead::Vector3f& getRotate() const override { return mRotate; }
    const sead::Vector3f& getScale() const override { return mScale; }
    const sead::Vector3f& getVelocity() const override { return mVelocity; }
    sead::Vector3f* getRotatePtr() override { return &mRotate; }
    sead::Vector3f* getScalePtr() override { return &mScale; }
    sead::Vector3f* getVelocityPtr() override { return &mVelocity; }
    void updatePoseRotate(const sead::Vector3f& rRotate) override;
    void updatePoseQuat(const sead::Quatf& rQuat) override;
    void updatePoseMtx(const sead::Matrix34f* pMtx) override;
    void calcBaseMtx(sead::Matrix34f* pMtx) const override;

    sead::Vector3f mRotate = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mScale = {1.0f, 1.0f, 1.0f};
    sead::Vector3f mVelocity = {0.0f, 0.0f, 0.0f};
};

class ActorPoseKeeperTRMSV : public ActorPoseKeeperBase {
public:
    ActorPoseKeeperTRMSV();

    const sead::Vector3f& getRotate() const override { return mRotate; }
    const sead::Vector3f& getScale() const override { return mScale; }
    const sead::Vector3f& getVelocity() const override { return mVelocity; }
    const sead::Matrix34f& getMtx() const override { return mMtx; }
    sead::Vector3f* getRotatePtr() override { return &mRotate; }
    sead::Vector3f* getScalePtr() override { return &mScale; }
    sead::Vector3f* getVelocityPtr() override { return &mVelocity; }
    sead::Matrix34f* getMtxPtr() override { return &mMtx; }
    void updatePoseRotate(const sead::Vector3f& rRotate) override;
    void updatePoseQuat(const sead::Quatf& rQuat) override;
    void updatePoseMtx(const sead::Matrix34f* pMtx) override;
    void calcBaseMtx(sead::Matrix34f* pMtx) const override;

    sead::Vector3f mRotate = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mScale = {1.0f, 1.0f, 1.0f};
    sead::Vector3f mVelocity = {0.0f, 0.0f, 0.0f};
    sead::Matrix34f mMtx;
};

class ActorPoseKeeperTFSV : public ActorPoseKeeperBase {
public:
    ActorPoseKeeperTFSV();

    const sead::Vector3f& getScale() const override { return mScale; }
    const sead::Vector3f& getVelocity() const override { return mVelocity; }
    const sead::Vector3f& getFront() const override { return mFront; }
    sead::Vector3f* getScalePtr() override { return &mScale; }
    sead::Vector3f* getVelocityPtr() override { return &mVelocity; }
    sead::Vector3f* getFrontPtr() override { return &mFront; }
    void updatePoseRotate(const sead::Vector3f& rRotate) override;
    void updatePoseQuat(const sead::Quatf& rQuat) override;
    void updatePoseMtx(const sead::Matrix34f* pMtx) override;
    void calcBaseMtx(sead::Matrix34f* pMtx) const override;

    sead::Vector3f mFront = sead::Vector3f::ez;
    sead::Vector3f mScale = {1.0f, 1.0f, 1.0f};
    sead::Vector3f mVelocity = {0.0f, 0.0f, 0.0f};
};

class ActorPoseKeeperTFGSV : public ActorPoseKeeperTFSV {
public:
    ActorPoseKeeperTFGSV();

    const sead::Vector3f& getGravity() const override { return mGravity; }
    sead::Vector3f* getGravityPtr() override { return &mGravity; }
    void updatePoseRotate(const sead::Vector3f& rRotate) override;
    void updatePoseQuat(const sead::Quatf& rQuat) override;
    void updatePoseMtx(const sead::Matrix34f* pMtx) override;
    void calcBaseMtx(sead::Matrix34f* pMtx) const override;

    sead::Vector3f mGravity = {0.0f, -1.0f, 0.0f};
};

class ActorPoseKeeperTQSV : public ActorPoseKeeperBase {
public:
    ActorPoseKeeperTQSV();

    const sead::Vector3f& getScale() const override { return mScale; }
    const sead::Vector3f& getVelocity() const override { return mVelocity; }
    const sead::Quatf& getQuat() const override { return mQuat; }
    sead::Vector3f* getScalePtr() override { return &mScale; }
    sead::Vector3f* getVelocityPtr() override { return &mVelocity; }
    sead::Quatf* getQuatPtr() override { return &mQuat; }
    void updatePoseRotate(const sead::Vector3f& rRotate) override;
    void updatePoseQuat(const sead::Quatf& rQuat) override;
    void updatePoseMtx(const sead::Matrix34f* pMtx) override;
    void calcBaseMtx(sead::Matrix34f* pMtx) const override;

    sead::Quatf mQuat = sead::Quatf::unit;
    sead::Vector3f mScale = {1.0f, 1.0f, 1.0f};
    sead::Vector3f mVelocity = {0.0f, 0.0f, 0.0f};
};
}  // namespace al
