#include "Project/Action/Common/ActionFlagCtrl.hpp"

#include "Project/Base/StringUtil.hpp"
#include "Library/HitSensor/HitSensorKeeper.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Library/HitSensor/HitSensor.hpp"

namespace alAnimFunction {
bool checkPass(f32 checkFrame, f32 frame, f32 frameRate);
}  // namespace alAnimFunction

namespace al {
const char* createStringIfInStack(const char* pStr);
bool tryGetActorInitFileName(sead::BufferedSafeString* pName, const LiveActor* pActor,
                             const char* pSuffixName, const char* pSuffix);
void createFileNameBySuffix(sead::BufferedSafeString* pName, const char* pSuffixName,
                            const char* pSuffix);
bool isExistModelResourceYaml(const LiveActor* pActor, const char* pFileName,
                              const char* pSuffix);
bool isValidCollisionParts(const LiveActor* pActor);
void validateCollisionParts(LiveActor* pActor);
void invalidateCollisionParts(LiveActor* pActor);
bool isNoCollide(const LiveActor* pActor);
void onCollide(LiveActor* pActor);
void offCollide(LiveActor* pActor);
bool isInvalidClipping(const LiveActor* pActor);
bool tryGetByamlS16(s16* pOut, const ByamlIter& rIter, const char* pKey);
ActionSensorCtrlInfo::ActionSensorCtrlInfo() = default;
ActionFlagCtrlInfo::ActionFlagCtrlInfo() = default;

ActionFlagCtrl::ActionFlagCtrl(LiveActor* actor, const char* name)
    : mParentActor(actor), mArchiveName(createStringIfInStack(name)) {}

ActionFlagCtrl* ActionFlagCtrl::tryCreate(LiveActor* actor, const char* name) {
    if (!isExistModelResource(actor))
        return nullptr;

    StringTmp<128> initFileName;

    if (!tryGetActorInitFileName(&initFileName, actor, "ActionFlagCtrl", name))
        createFileNameBySuffix(&initFileName, "ActionFlagCtrl", name);

    if (!isExistModelResourceYaml(actor, initFileName.cstr(), nullptr))
        return nullptr;

    return new ActionFlagCtrl(actor, name);
}

inline CtrlFlag getCtrlFlagByKey(ByamlIter iter, const char* name) {
    bool isValid = false;

    if (!iter.tryGetBoolByKey(&isValid, name))
        return CtrlFlag::NotFound;

    return isValid ? CtrlFlag::ValidOn : CtrlFlag::ValidOff;
}

void ActionFlagCtrl::initPost() {
    mHitSensorKeeper = mParentActor->mHitSensorKeeper;

    StringTmp<128> initFileName;
    tryGetActorInitFileName(&initFileName, mParentActor, "ActionFlagCtrl", mArchiveName);

    ByamlIter fileIter;
    tryGetActorInitFileIter(&fileIter, mParentActor, initFileName.cstr(), mArchiveName);

    mInfoCount = fileIter.getSize();
    mCtrlInfoArray = new ActionFlagCtrlInfo*[mInfoCount];

    for (s32 i = 0; i < mInfoCount; i++) {
        ByamlIter iter;
        fileIter.tryGetIterByIndex(&iter, i);
        ActionFlagCtrlInfo* flagCtrlInfo = new ActionFlagCtrlInfo();
        mCtrlInfoArray[i] = flagCtrlInfo;

        iter.tryGetStringByKey(&flagCtrlInfo->actionName, "ActionName");

        flagCtrlInfo->ctrlFlags = new CtrlFlag[5];
        flagCtrlInfo->ctrlFlags[0] = getCtrlFlagByKey(iter, "ShowModel");
        flagCtrlInfo->ctrlFlags[1] = getCtrlFlagByKey(iter, "CollisionParts");
        flagCtrlInfo->ctrlFlags[2] = getCtrlFlagByKey(iter, "Collider");
        flagCtrlInfo->ctrlFlags[3] = getCtrlFlagByKey(iter, "ClippingOff");
        flagCtrlInfo->ctrlFlags[4] = getCtrlFlagByKey(iter, "FaceCtrl");

        ByamlIter sensorListIter;

        if (iter.tryGetIterByKey(&sensorListIter, "SensorList") && mHitSensorKeeper) {
            flagCtrlInfo->sensorCtrlInfoArray =
                new ActionSensorCtrlInfo[mHitSensorKeeper->mSensorCount];

            for (s32 j = 0; j < mHitSensorKeeper->mSensorCount; j++) {
                ActionSensorCtrlInfo* sensorInfoArray = flagCtrlInfo->sensorCtrlInfoArray;
                sensorInfoArray[j].name = mHitSensorKeeper->getSensor(j)->getName();

                ByamlIter sensorIter;

                if (sensorListIter.tryGetIterByKey(&sensorIter, sensorInfoArray[j].name)) {
                    sensorInfoArray[j].state = getCtrlFlagByKey(sensorIter, "State");
                    tryGetByamlS16(&sensorInfoArray[j].startFrame, sensorIter, "StartFrame");
                    tryGetByamlS16(&sensorInfoArray[j].endFrame, sensorIter, "EndFrame");
                }
            }
        }
    }
}

void ActionFlagCtrl::start(const char* name) {
    mCurrentCtrlInfo = findFlagInfo(name);
    mIsUpdateNeeded = false;

    if (!mCurrentCtrlInfo)
        return;

    startCtrlFlag();
    startCtrlSensor();
}

ActionFlagCtrlInfo* ActionFlagCtrl::findFlagInfo(const char* name) const {
    for (s32 i = 0; i < mInfoCount; i++) {
        ActionFlagCtrlInfo* flagInfo = mCtrlInfoArray[i];

        if (isEqualStringCase(flagInfo->actionName, name))
            return flagInfo;
    }

    return nullptr;
}

void ActionFlagCtrl::startCtrlFlag() {
    if (isFlagValidOn(0, !isHideModel(mParentActor)))
        showModel(mParentActor);
    else if (isFlagValidOff(0, !isHideModel(mParentActor)))
        hideModel(mParentActor);

    if (mParentActor->mCollisionParts) {
        if (isFlagValidOn(1, isValidCollisionParts(mParentActor)))
            validateCollisionParts(mParentActor);
        else if (isFlagValidOff(1, isValidCollisionParts(mParentActor)))
            invalidateCollisionParts(mParentActor);
    }

    if (mParentActor->mCollider) {
        if (isFlagValidOn(2, !isNoCollide(mParentActor)))
            onCollide(mParentActor);
        else if (isFlagValidOff(2, !isNoCollide(mParentActor)))
            offCollide(mParentActor);
    }

    if (isFlagValidOn(3, isInvalidClipping(mParentActor)))
        invalidateClipping(mParentActor);
    else if (isFlagValidOff(3, isInvalidClipping(mParentActor)))
        validateClipping(mParentActor);
}

void ActionFlagCtrl::startCtrlSensor() {
    if (!mHitSensorKeeper)
        return;

    for (s32 i = 0; i < mHitSensorKeeper->mSensorCount; i++) {
        ActionSensorCtrlInfo* sensor = mCurrentCtrlInfo->sensorCtrlInfoArray;

        if (sensor[i].startFrame > 0) {
            mIsUpdateNeeded = true;
            continue;
        }

        if (sensor[i].endFrame > 0)
            mIsUpdateNeeded = true;

        switch (sensor[i].state) {
        case CtrlFlag::ValidOn:
            validateHitSensor(mParentActor, sensor[i].name);
            break;
        case CtrlFlag::ValidOff:
            invalidateHitSensor(mParentActor, sensor[i].name);
            break;
        default:
            break;
        }
    }
}

void ActionFlagCtrl::update(f32 frame, f32 frameRate) {
    if (!mCurrentCtrlInfo || !mIsUpdateNeeded)
        return;
    if (!mHitSensorKeeper)
        return;

    updateCtrlSensor(frame, frameRate);
}

void ActionFlagCtrl::updateCtrlSensor(f32 frame, f32 frameRate) {
    for (s32 i = 0; i < mHitSensorKeeper->mSensorCount; i++) {
        ActionSensorCtrlInfo* ctrlInfo = mCurrentCtrlInfo->sensorCtrlInfoArray;

        f32 startFrame = ctrlInfo[i].startFrame;

        if (startFrame > 0 &&
            alAnimFunction::checkPass(startFrame, frame, frameRate))
            validateHitSensor(mParentActor, ctrlInfo[i].name);

        f32 endFrame = ctrlInfo[i].endFrame;

        if (endFrame > 0 &&
            alAnimFunction::checkPass(endFrame, frame, frameRate))
            invalidateHitSensor(mParentActor, ctrlInfo[i].name);
    }
}

bool ActionFlagCtrl::isFlagValidOn(s32 index, bool isEnabled) const {
    return mCurrentCtrlInfo && mCurrentCtrlInfo->ctrlFlags[index] == CtrlFlag::ValidOn &&
           !isEnabled;
}

bool ActionFlagCtrl::isFlagValidOff(s32 index, bool isEnabled) const {
    return mCurrentCtrlInfo && mCurrentCtrlInfo->ctrlFlags[index] == CtrlFlag::ValidOff &&
           isEnabled;
}

}  // namespace al
