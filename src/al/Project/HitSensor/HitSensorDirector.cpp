#include "Library/HitSensor/HitSensorDirector.hpp"

#include "Library/Execute/ExecuteUtil.hpp"
#include "Library/HitSensor/HitSensor.hpp"
#include "Library/HitSensor/SensorHitGroup.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"

namespace al {
namespace {
inline void checkHit(HitSensor* pA, HitSensor* pB) {
    if (pA->mHostActor == pB->mHostActor) {
        return;
    }
    sead::Vector3f diff = pA->mPos - pB->mPos;
    f32 radius = pA->mRadius + pB->mRadius;
    if (diff.squaredLength() >= radius * radius) {
        return;
    }
    switch (pB->mSensorType) {
    case HitSensorType::Eye:
    case HitSensorType::PlayerEye:
        break;
    default:
        pA->addHitSensor(pB);
        break;
    }
    switch (pA->mSensorType) {
    case HitSensorType::Eye:
    case HitSensorType::PlayerEye:
        break;
    default:
        pB->addHitSensor(pA);
        break;
    }
}
}  // namespace

/**
 * Constructs the threaded update executor of a hit sensor director.
 * @param pExecuteDirector The execute director to register in.
 * @param pDirector The hit sensor director.
 */
HitSensorDirector::MultiThreadUpdate::MultiThreadUpdate(ExecuteDirector* pExecuteDirector,
                                                        HitSensorDirector* pDirector)
    : mDirector(pDirector) {
    registerExecutorUser(this, pExecuteDirector, "HitSensorThreadUpdate");
}

/**
 * Destroys the threaded update executor after waiting for the thread.
 */
HitSensorDirector::MultiThreadUpdate::~MultiThreadUpdate() {
    waitDone();
}

/**
 * Requests the hit check to run on the queue thread.
 */
void HitSensorDirector::MultiThreadUpdate::execute() {
    mDirector->mQueueThread->requestExecute(mDirector);
}

/**
 * Waits for the queue thread to finish.
 */
void HitSensorDirector::MultiThreadUpdate::waitDone() {
    mDirector->mQueueThread->waitDone();
}

/**
 * Constructs a hit sensor director and its sensor groups.
 * @param pExecuteDirector The execute director to register in.
 * @param scale The group size multiplier.
 * @param pThread The queue thread to run the hit checks on, or nullptr.
 */
HitSensorDirector::HitSensorDirector(ExecuteDirector* pExecuteDirector, s32 scale,
                                     MultiCoreQueueThread* pThread)
    : mQueueThread(pThread) {
    s32 size = scale > 1 ? scale : 1;
    mPlayerGroup = new SensorHitGroup(256, "Player");
    mPlayerEyeGroup = new SensorHitGroup(128, "PlayerEye");
    mRideGroup = new SensorHitGroup(128, "Ride");
    mEyeGroup = new SensorHitGroup(1024, "Eye");
    mSimpleGroup = new SensorHitGroup(size * 2048, "Simple");
    mMapObjGroup = new SensorHitGroup(size * 1024 + 512, "MapObj");
    mCharacterGroup = new SensorHitGroup(size * 1024, "Character");
    if (mQueueThread != nullptr) {
        mThreadUpdate = new MultiThreadUpdate(pExecuteDirector, this);
    }
    registerExecutorUser(this, pExecuteDirector, "センサー");
}

/**
 * Assigns a sensor to the hit group matching its type.
 * @param pSensor The sensor.
 */
void HitSensorDirector::initGroup(HitSensor* pSensor) {
    if (isSensorPlayerEye(pSensor)) {
        pSensor->mHitGroup = mPlayerEyeGroup;
    } else if (isSensorPlayer(pSensor)) {
        pSensor->mHitGroup = mPlayerGroup;
    } else if (isSensorRide(pSensor)) {
        pSensor->mHitGroup = mRideGroup;
    } else if (isSensorEye(pSensor)) {
        pSensor->mHitGroup = mEyeGroup;
    } else if (isSensorSimple(pSensor)) {
        pSensor->mHitGroup = mSimpleGroup;
    } else if (isSensorMapObj(pSensor)) {
        pSensor->mHitGroup = mMapObjGroup;
    } else {
        pSensor->mHitGroup = mCharacterGroup;
    }
}

/**
 * Clears all groups and runs the hit checks between them.
 */
void HitSensorDirector::trueExecute() {
    mPlayerGroup->clear();
    mPlayerEyeGroup->clear();
    mRideGroup->clear();
    mEyeGroup->clear();
    mSimpleGroup->clear();
    mMapObjGroup->clear();
    mCharacterGroup->clear();
    mPlayerGroup->executeHitCheckInSameGroup();
    mPlayerGroup->executeHitCheckGroup(mPlayerEyeGroup);
    mPlayerGroup->executeHitCheckGroup(mCharacterGroup);
    mPlayerGroup->executeHitCheckGroup(mMapObjGroup);
    mPlayerGroup->executeHitCheckGroup(mRideGroup);
    mPlayerGroup->executeHitCheckGroup(mSimpleGroup);
    mPlayerGroup->executeHitCheckGroup(mEyeGroup);
    mPlayerEyeGroup->executeHitCheckGroup(mCharacterGroup);
    mPlayerEyeGroup->executeHitCheckGroup(mMapObjGroup);
    mPlayerEyeGroup->executeHitCheckGroup(mRideGroup);
    mPlayerEyeGroup->executeHitCheckGroup(mSimpleGroup);
    mRideGroup->executeHitCheckGroup(mCharacterGroup);
    mRideGroup->executeHitCheckGroup(mMapObjGroup);
    mRideGroup->executeHitCheckGroup(mSimpleGroup);
    mRideGroup->executeHitCheckGroup(mEyeGroup);
    mEyeGroup->executeHitCheckGroup(mCharacterGroup);
    mEyeGroup->executeHitCheckGroup(mMapObjGroup);
    mEyeGroup->executeHitCheckGroup(mSimpleGroup);
    mCharacterGroup->executeHitCheckGroup(mMapObjGroup);
    mCharacterGroup->executeHitCheckInSameGroup();
}

/**
 * Runs the hit checks directly, or waits for the threaded update to finish.
 */
void HitSensorDirector::execute() {
    if (mThreadUpdate == nullptr || mIsExecuteDirect) {
        mIsExecuteDirect = false;
        trueExecute();
    } else {
        mThreadUpdate->waitDone();
    }
}

/**
 * Runs the hit checks on the queue thread.
 */
void HitSensorDirector::executeOnThread() {
    trueExecute();
}

/**
 * Checks every sensor of a group against every sensor of another group.
 * @param pGroupA The first group.
 * @param pGroupB The second group.
 */
void HitSensorDirector::executeHitCheckGroup(SensorHitGroup* pGroupA,
                                             SensorHitGroup* pGroupB) const {
    s32 count = pGroupA->getSensorCount();
    for (s32 i = 0; i < count; i++) {
        HitSensor* sensor = pGroupA->getSensor(i);
        s32 otherCount = pGroupB->getSensorCount();
        for (s32 j = 0; j < otherCount; j++) {
            executeHitCheck(sensor, pGroupB->getSensor(j));
        }
    }
}

/**
 * Checks two sensors against each other and registers the hits.
 * @param pA The first sensor.
 * @param pB The second sensor.
 */
void HitSensorDirector::executeHitCheck(HitSensor* pA, HitSensor* pB) const {
    checkHit(pA, pB);
}

/**
 * Checks every pair of sensors in a group.
 * @param pGroup The group.
 */
void HitSensorDirector::executeHitCheckInSameGroup(SensorHitGroup* pGroup) const {
    s32 count = pGroup->getSensorCount();
    for (s32 i = 0; i < count; i++) {
        HitSensor* sensor = pGroup->getSensor(i);
        for (s32 j = i; j != count; j++) {
            executeHitCheck(sensor, pGroup->getSensor(j));
        }
    }
}

/**
 * Destroys the director and its threaded update executor.
 */
HitSensorDirector::~HitSensorDirector() {
    if (mThreadUpdate != nullptr) {
        delete mThreadUpdate;
        mThreadUpdate = nullptr;
    }
}
}  // namespace al
