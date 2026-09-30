#include "Library/LiveActor/ActorParamHolder.hpp"

#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
inline ActorParamInfo::ActorParamInfo() = default;

/**
 * Finds an integer parameter by name.
 * @param pName The parameter name.
 * @return The parameter, or a zero default.
 */
const ActorParamS32* ActorParamHolder::findParamS32(const char* pName) const {
    ActorParamInfo* info = tryFindParamInfoByName(pName);
    if (info == nullptr) {
        return reinterpret_cast<const ActorParamS32*>("");
    }
    return &info->mParamS32;
}

/**
 * Finds a parameter entry by name.
 * @param pName The parameter name.
 * @return The entry, or nullptr.
 */
ActorParamInfo* ActorParamHolder::tryFindParamInfoByName(const char* pName) const {
    for (s32 i = 0; i < mSize; i++) {
        ActorParamInfo* info = &mInfoArray[i];
        if (isEqualString(info->mName, pName)) {
            return info;
        }
    }
    return nullptr;
}

/**
 * Finds a float parameter by name.
 * @param pName The parameter name.
 * @return The parameter, or a zero default.
 */
const ActorParamF32* ActorParamHolder::findParamF32(const char* pName) const {
    ActorParamInfo* info = tryFindParamInfoByName(pName);
    if (info == nullptr) {
        return reinterpret_cast<const ActorParamF32*>("");
    }
    return &info->mParamF32;
}

/**
 * Finds a movement parameter by name.
 * @param pName The parameter name.
 * @return The parameter, or a zero default.
 */
const ActorParamMove* ActorParamHolder::findParamMove(const char* pName) const {
    ActorParamInfo* info = tryFindParamInfoByName(pName);
    if (info == nullptr) {
        return reinterpret_cast<const ActorParamMove*>("");
    }
    return info->mParamMove;
}

/**
 * Finds a jump parameter by name.
 * @param pName The parameter name.
 * @return The parameter, or a zero default.
 */
const ActorParamJump* ActorParamHolder::findParamJump(const char* pName) const {
    ActorParamInfo* info = tryFindParamInfoByName(pName);
    if (info == nullptr) {
        return reinterpret_cast<const ActorParamJump*>("");
    }
    return info->mParamJump;
}

/**
 * Finds a sight parameter by name.
 * @param pName The parameter name.
 * @return The parameter, or a zero default.
 */
const ActorParamSight* ActorParamHolder::findParamSight(const char* pName) const {
    ActorParamInfo* info = tryFindParamInfoByName(pName);
    if (info == nullptr) {
        return reinterpret_cast<const ActorParamSight*>("");
    }
    return info->mParamSight;
}

/**
 * Finds a rebound parameter by name.
 * @param pName The parameter name.
 * @return The parameter, or a zero default.
 */
const ActorParamRebound* ActorParamHolder::findParamRebound(const char* pName) const {
    ActorParamInfo* info = tryFindParamInfoByName(pName);
    if (info == nullptr) {
        return reinterpret_cast<const ActorParamRebound*>("");
    }
    return info->mParamRebound;
}

/**
 * Reads the parameters from the actor's ActorParam file.
 * @param pActor The actor.
 */
ActorParamHolder::ActorParamHolder(LiveActor* pActor) {
    getModelResource(pActor);
    ByamlIter paramIter(getModelResourceYaml(pActor, "ActorParam", nullptr));
    mSize = paramIter.getSize();
    mInfoArray = new ActorParamInfo[mSize];
    for (s32 i = 0; i < mSize; i++) {
        ActorParamInfo* info = &mInfoArray[i];
        ByamlIter iter;
        paramIter.tryGetIterByIndex(&iter, i);
        iter.tryGetStringByKey(&info->mName, "ParamName");

        ByamlIter valueIter;
        if (iter.tryGetIterByKey(&valueIter, "S32")) {
            info->mType = ActorParamType::S32;
            valueIter.tryGetIntByKey(&info->mParamS32.value, "ParamS32");
            continue;
        }
        if (iter.tryGetIterByKey(&valueIter, "F32")) {
            info->mType = ActorParamType::F32;
            valueIter.tryGetFloatByKey(&info->mParamF32.value, "ParamF32");
            continue;
        }
        if (iter.tryGetIterByKey(&valueIter, "ActorParamMove")) {
            info->mType = ActorParamType::Move;
            auto* param = new ActorParamMove;
            valueIter.tryGetFloatByKey(&param->moveAccel, "MoveAccel");
            valueIter.tryGetFloatByKey(&param->gravity, "Gravity");
            valueIter.tryGetFloatByKey(&param->moveFriction, "MoveFriction");
            valueIter.tryGetFloatByKey(&param->turnSpeedDegree, "TurnSpeedDegree");
            info->mParamMove = param;
            continue;
        }
        if (iter.tryGetIterByKey(&valueIter, "ActorParamJump")) {
            info->mType = ActorParamType::Jump;
            auto* param = new ActorParamJump;
            valueIter.tryGetFloatByKey(&param->speedFront, "SpeedFront");
            valueIter.tryGetFloatByKey(&param->speedUp, "SpeedUp");
            info->mParamJump = param;
            continue;
        }
        if (iter.tryGetIterByKey(&valueIter, "ActorParamSight")) {
            info->mType = ActorParamType::Sight;
            auto* param = new ActorParamSight;
            valueIter.tryGetFloatByKey(&param->distance, "Distance");
            valueIter.tryGetFloatByKey(&param->degreeH, "DegreeH");
            valueIter.tryGetFloatByKey(&param->degreeV, "DegreeV");
            info->mParamSight = param;
            continue;
        }
        if (iter.tryGetIterByKey(&valueIter, "ActorParamRebound")) {
            info->mType = ActorParamType::Rebound;
            auto* param = new ActorParamRebound;
            valueIter.tryGetFloatByKey(&param->reboundRate, "ReboundRate");
            valueIter.tryGetFloatByKey(&param->speedMinToRebound, "SpeedMinToRebound");
            valueIter.tryGetFloatByKey(&param->frictionH, "FrictionH");
            info->mParamRebound = param;
            continue;
        }
    }
}
}  // namespace al
