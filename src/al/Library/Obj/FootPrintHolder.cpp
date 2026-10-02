#include "Library/Obj/FootPrintHolder.hpp"

#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/Collision/CollisionPartsKeeperUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Obj/FootPrintServer.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Collision/CollisionPartsTriangle.hpp"
#include "Project/Collision/CollisionUtil.hpp"
#include "Project/Obj/FootPrint.hpp"

namespace {
struct MaterialAnimEntry {
    const char* materialName;
    const char* animName;
};

const MaterialAnimEntry sMaterialAnimTable[] = {
    {"Soil", "Normal"},        {"Lawn", "Normal"},           {"LallenLeaves", "Normal"},
    {"Cream", "Snow"},         {"ChocoCream", "Snow"},       {"Sand", "Snow"},
    {"Snow", "Snow"},          {"Cloud", "Snow"},            {"CatFur", "Normal"},
    {"CatFurGreen", "Normal"}, {"CatLitterBlack", "Snow"},   {"CatLitterBlue", "Snow"},
    {"CatLitterWhite", "Snow"}, {"Ink", "Snow"},
};

constexpr s32 cMaterialAnimNum = sizeof(sMaterialAnimTable) / sizeof(sMaterialAnimTable[0]);

/**
 * Checks for a floor polygon below a position.
 * @param pHitPos hit position
 * @param pTriangle hit triangle
 * @param pActor actor checking the collision
 * @param rPos base position of the check
 * @param rUp up direction of the actor
 * @param rFront front direction of the actor
 * @param frontOffset offset of the check along the front direction
 * @return whether a polygon was hit
 */
inline bool checkFloorPoly(sead::Vector3f* pHitPos, al::Triangle* pTriangle,
                           const al::LiveActor* pActor, const sead::Vector3f& rPos,
                           const sead::Vector3f& rUp, const sead::Vector3f& rFront,
                           f32 frontOffset) {
    sead::Vector3f start = rPos + rUp * 30.0f + rFront * frontOffset;
    return alCollisionUtil::getFirstPolyOnArrow(pActor, pHitPos, pTriangle, start, rUp * -50.0f,
                                                nullptr, nullptr);
}
}  // namespace

namespace al {
/**
 * Constructs a foot print holder, reading the actor's foot print info.
 * @param pActor actor leaving the foot prints
 * @param pArchiveName archive holding the FootPrintInfo resource
 * @param pServer server owning the foot prints
 */
FootPrintHolder::FootPrintHolder(LiveActor* pActor, const char* pArchiveName,
                                 FootPrintServer* pServer)
    : mServer(pServer), mActor(pActor) {
    const u8* byml = tryGetBymlFromObjectResource(pArchiveName, "FootPrintInfo");

    if (byml != nullptr) {
        mInfoIter = new ByamlIter(byml);
    }

    createActionList();
    mFootPrints = new sead::RingBuffer<FootPrint*>;
    mFootPrints->allocBuffer(mServer->mFootPrints->capacity(), nullptr);
}

/**
 * Creates the action list from the foot print info.
 */
void FootPrintHolder::createActionList() {
    mActionMax = mInfoIter->getSize();
    mActionNum = mInfoIter->getSize();
    mActions = new Action*[mActionMax];

    for (u32 i = 0; i < mActionNum; i++) {
        mActions[i] = new Action;
        ByamlIter actionIter;

        if (!mInfoIter->tryGetIterByIndex(&actionIter, i)) {
            continue;
        }

        const char* actionName;

        if (!actionIter.tryGetStringByKey(&actionName, "ActionName")) {
            continue;
        }

        mActions[i]->name = actionName;
        createTimingList(mActions[i], &actionIter);
    }
}

/**
 * Releases dead foot prints and appears new ones at the timings of the current action.
 */
void FootPrintHolder::update() {
    while (mFootPrints->size() > 0 && isDead(mFootPrints->front())) {
        mFootPrints->popFront();
    }

    for (s32 i = mFootPrints->size() - 1; i >= 0; i--) {
        if (isDead((*mFootPrints)(i))) {
            mFootPrints->remove(i);
        }
    }

    const char* actionName = getActionName(mActor);

    if (actionName == nullptr) {
        return;
    }

    if (mPrevActionName == nullptr || !isEqualString(mPrevActionName, actionName)) {
        mPrevActionFrame = 0.0f;
    }

    f32 frame = getActionFrame(mActor);

    for (u32 i = 0; i < mActionNum; i++) {
        if (!isEqualString(actionName, mActions[i]->name)) {
            continue;
        }

        for (u32 j = 0; j < mActions[i]->timingNum; j++) {
            if (mPrevActionFrame == frame) {
                continue;
            }

            Timing* timing = &mActions[i]->timings[j];
            f32 timingFrame = timing->frame;

            if (mPrevActionFrame > frame) {
                if (!(mPrevActionFrame <= timingFrame || frame > timingFrame)) {
                    continue;
                }
            } else if (!(mPrevActionFrame <= timingFrame && frame > timingFrame)) {
                continue;
            }

            appearFootPrint(timing->offset);

            if (mServer->mFootPrints->capacity() - 20 < mFootPrints->size()) {
                for (s32 k = 0; k < mFootPrints->size(); k++) {
                    FootPrint* footPrint = (*mFootPrints)(k);

                    if (!footPrint->isDisappear()) {
                        footPrint->startDisappear();
                        break;
                    }
                }
            }
        }
    }

    mPrevActionName = actionName;
    mPrevActionFrame = frame;
}

/**
 * Appears a foot print on the floor below the given offset from the actor.
 * @param rOffset offset in the actor's local axes
 */
void FootPrintHolder::appearFootPrint(const sead::Vector3f& rOffset) {
    sead::Vector3f side;
    sead::Vector3f up;
    sead::Vector3f front;
    calcSideDir(&side, mActor);
    calcUpDir(&up, mActor);
    calcFrontDir(&front, mActor);
    sead::Vector3f pos = getTrans(mActor) + side * rOffset.x + up * rOffset.y + front * rOffset.z;

    Triangle triangle;
    sead::Vector3f hitPos;

    if (checkFloorPoly(&hitPos, &triangle, mActor, pos, up, front, 15.0f)) {
        checkFloorPoly(&hitPos, &triangle, mActor, pos, up, front, 0.0f);
    }

    const char* materialName = getMaterialCodeName(triangle);

    if (materialName == nullptr) {
        return;
    }

    const MaterialAnimEntry* entry = nullptr;

    for (s32 i = 0; i < cMaterialAnimNum; i++) {
        if (isEqualString(sMaterialAnimTable[i].materialName, materialName)) {
            entry = &sMaterialAnimTable[i];
            break;
        }
    }

    if (entry == nullptr) {
        return;
    }

    FootPrint* footPrint = findDeadFootPrint();

    if (footPrint == nullptr) {
        if (mFootPrints->empty()) {
            return;
        }

        footPrint = findDeadFootPrintByForce();
    }

    if (sendMsgInvalidateFootPrint(triangle.getSensor(), getHitSensor(mActor, 0))) {
        return;
    }

    const char* animName = entry->animName;
    mFootPrints->pushBack(footPrint);

    sead::Matrix34f baseMtx;
    baseMtx.setTranslation(0.0f, 0.0f, 0.0f);
    baseMtx.setBase(0, side);
    baseMtx.setBase(1, up);
    baseMtx.setBase(2, front);
    sead::Quatf quat;
    sead::Matrix34f rotateMtx;

    if (quat.makeVectorRotation(up, *triangle.getNormal(0))) {
        rotateMtx.makeQT(quat, sead::Vector3f(0.0f, 0.0f, 0.0f));
    } else {
        rotateMtx = sead::Matrix34f(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    1.0f, 0.0f);
    }

    sead::Matrix34f poseMtx;
    poseMtx.setMul(rotateMtx, baseMtx);
    updatePoseMtx(footPrint, &poseMtx);
    setTrans(footPrint, hitPos);
    sead::Vector3f* trans = getTransPtr(footPrint);
    trans->setScaleAdd(5.0f, *triangle.getNormal(0), *trans);
    footPrint->setFollowCollisionParts(triangle.mCollisionParts);
    footPrint->appear();

    if (mCharacterName != nullptr) {
        footPrint->setAnimationByCharacter(mCharacterName);
    }

    if (mMetamorphosisName != nullptr) {
        footPrint->setAnimationByMetamorphosis(mMetamorphosisName);
    }

    footPrint->setAnimationByMaterial(animName);
}

/**
 * Finds a dead foot print in the server.
 * @return dead foot print, or null
 */
FootPrint* FootPrintHolder::findDeadFootPrint() {
    return mServer->findDeadFootPrint();
}

/**
 * Kills the oldest foot print of this holder so it can be reused.
 * @return the killed foot print
 */
FootPrint* FootPrintHolder::findDeadFootPrintByForce() {
    FootPrint* footPrint = mFootPrints->front();
    mFootPrints->popFront();
    footPrint->makeActorDead();
    return footPrint;
}

/**
 * Gets the character name used for the foot print animation.
 * @return character name, or null
 */
const char* FootPrintHolder::getCharacterName() const {
    return mCharacterName;
}

/**
 * Gets the metamorphosis name used for the foot print animation.
 * @return metamorphosis name, or null
 */
const char* FootPrintHolder::getMetamorphosisName() const {
    return mMetamorphosisName;
}

/**
 * Reads the foot print timings of an action.
 * @param pAction action to fill
 * @param pIter iterator of the action info
 */
void FootPrintHolder::createTimingList(Action* pAction, ByamlIter* pIter) {
    ByamlIter timingListIter;

    if (!pIter->tryGetIterByKey(&timingListIter, "Timing")) {
        return;
    }

    u32 timingNum = timingListIter.getSize();
    pAction->timingMax = timingNum;
    pAction->timingNum = timingNum;
    pAction->timings = new Timing[static_cast<s32>(timingNum)];

    for (u32 i = 0; i < timingNum; i++) {
        ByamlIter timingIter;

        if (!timingListIter.tryGetIterByIndex(&timingIter, i)) {
            continue;
        }

        if (!timingIter.tryGetIntByKey(&pAction->timings[i].frame, "Frame")) {
            pAction->timings[i].frame = 0;
        }

        if (!timingIter.tryGetFloatByKey(&pAction->timings[i].offset.x, "OffsetX")) {
            pAction->timings[i].offset.x = 0.0f;
        }

        if (!timingIter.tryGetFloatByKey(&pAction->timings[i].offset.y, "OffsetY")) {
            pAction->timings[i].offset.y = 0.0f;
        }

        if (!timingIter.tryGetFloatByKey(&pAction->timings[i].offset.z, "OffsetZ")) {
            pAction->timings[i].offset.z = 0.0f;
        }
    }
}

/**
 * Calculates the maximum number of foot prints, depending on the player count.
 * @return maximum foot print count
 */
s32 FootPrintHolder::calcMaxAppearNum() const {
    s32 playerNum = getAlivePlayerNum(mActor);

    if (playerNum < 2) {
        return 12;
    }

    if (playerNum == 2) {
        return 8;
    }

    return 5;
}
}  // namespace al
