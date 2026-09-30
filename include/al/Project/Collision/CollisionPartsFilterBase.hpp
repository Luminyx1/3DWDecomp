#pragma once

#include <basis/seadTypes.h>

namespace al {
class CollisionParts;
class HitSensor;
class LiveActor;

class CollisionPartsFilterBase {
public:
    virtual bool isInvalidParts(const CollisionParts& rParts) const = 0;
};

class CollisionPartsFilterOnlySpecialPurpose : public CollisionPartsFilterBase {
public:
    CollisionPartsFilterOnlySpecialPurpose(const char* pSpecialPurpose)
        : mSpecialPurpose(pSpecialPurpose) {}

    bool isInvalidParts(const CollisionParts& rParts) const override;

private:
    const char* mSpecialPurpose;
};

class CollisionPartsFilterActor : public CollisionPartsFilterBase {
public:
    CollisionPartsFilterActor(const LiveActor* pActor) : mActor(pActor) {}

    bool isInvalidParts(const CollisionParts& rParts) const override;

private:
    const LiveActor* mActor;
    bool mIsInvalidActorParts = true;
};

class CollisionPartsFilterSpecialPurpose : public CollisionPartsFilterBase {
public:
    CollisionPartsFilterSpecialPurpose(const char* pSpecialPurpose)
        : mSpecialPurpose(pSpecialPurpose) {}

    bool isInvalidParts(const CollisionParts& rParts) const override;

private:
    const char* mSpecialPurpose;
};

class CollisionPartsFilterMergePair : public CollisionPartsFilterBase {
public:
    CollisionPartsFilterMergePair(CollisionPartsFilterBase* pFilterA,
                                  CollisionPartsFilterBase* pFilterB)
        : mFilterA(pFilterA), mFilterB(pFilterB) {}

    bool isInvalidParts(const CollisionParts& rParts) const override;

private:
    CollisionPartsFilterBase* mFilterA;
    CollisionPartsFilterBase* mFilterB;
};

class CollisionPartsFilterConnectedSensor : public CollisionPartsFilterBase {
public:
    CollisionPartsFilterConnectedSensor(const HitSensor* pSensor) : mSensor(pSensor) {}

    bool isInvalidParts(const CollisionParts& rParts) const override;

private:
    const HitSensor* mSensor;
};

class CollisionPartsFilterConnectedSensorType : public CollisionPartsFilterBase {
public:
    CollisionPartsFilterConnectedSensorType(s32 sensorType) : mSensorType(sensorType) {}

    bool isInvalidParts(const CollisionParts& rParts) const override;

private:
    s32 mSensorType;
};

class CollisionPartsFilterNoSpecialPurpose : public CollisionPartsFilterBase {
public:
    bool isInvalidParts(const CollisionParts& rParts) const override;
};

class CollisionPartsFilterOrPair : public CollisionPartsFilterBase {
public:
    CollisionPartsFilterOrPair(CollisionPartsFilterBase* pFilterA,
                               CollisionPartsFilterBase* pFilterB)
        : mFilterA(pFilterA), mFilterB(pFilterB) {}

    bool isInvalidParts(const CollisionParts& rParts) const override;

private:
    CollisionPartsFilterBase* mFilterA;
    CollisionPartsFilterBase* mFilterB;
};

using CollisionPartsFilterFuncPtr = bool (*)(const CollisionParts&);
using CollisionPartsFilterSendMsgFuncPtr = bool (*)(const CollisionParts&, HitSensor*);

class CollisionPartsFilterFunc : public CollisionPartsFilterBase {
public:
    CollisionPartsFilterFunc(CollisionPartsFilterFuncPtr pFunc) : mFunc(pFunc) {}

    bool isInvalidParts(const CollisionParts& rParts) const override;

private:
    CollisionPartsFilterFuncPtr mFunc;
};

class CollisionPartsFilterFuncSendMsg : public CollisionPartsFilterBase {
public:
    CollisionPartsFilterFuncSendMsg(CollisionPartsFilterSendMsgFuncPtr pFunc, HitSensor* pSensor)
        : mFunc(pFunc), mSensor(pSensor) {}

    bool isInvalidParts(const CollisionParts& rParts) const override;

private:
    CollisionPartsFilterSendMsgFuncPtr mFunc;
    HitSensor* mSensor;
};

CollisionPartsFilterBase* createCollisionPartsFilterFunc(CollisionPartsFilterFuncPtr pFunc);
CollisionPartsFilterBase*
createCollisionPartsFilterFuncSendMsg(CollisionPartsFilterSendMsgFuncPtr pFunc, HitSensor* pSensor);
CollisionPartsFilterBase* createCollisionPartsFilterConnectedSensor(const HitSensor* pSensor);
}  // namespace al
