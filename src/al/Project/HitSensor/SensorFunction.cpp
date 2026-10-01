#include "Library/HitSensor/SensorFunction.hpp"

#include "Library/HitSensor/HitSensorKeeper.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
struct SensorTypeEntry {
    const char* mName;
    al::HitSensorType mType;
};

const SensorTypeEntry cSensorTypeTable[] = {
    {"Eye", al::HitSensorType::Eye},
    {"Player", al::HitSensorType::Player},
    {"PlayerEye", al::HitSensorType::PlayerEye},
    {"KickKoura", al::HitSensorType::KickKoura},
    {"Npc", al::HitSensorType::Npc},
    {"NpcAvoid", al::HitSensorType::NpcAvoid},
    {"Ride", al::HitSensorType::Ride},
    {"Enemy", al::HitSensorType::Enemy},
    {"EnemyBody", al::HitSensorType::EnemyBody},
    {"EnemyAttack", al::HitSensorType::EnemyAttack},
    {"Dossun", al::HitSensorType::Dossun},
    {"KillerMagnum", al::HitSensorType::KillerMagnum},
    {"EnemySimple", al::HitSensorType::EnemySimple},
    {"MapObj", al::HitSensorType::MapObj},
    {"MapObjSimple", al::HitSensorType::MapObjSimple},
    {"CollisionParts", al::HitSensorType::CollisionParts},
    {"PlayerFireBall", al::HitSensorType::PlayerFireBall},
    {"WooGanSandBody", al::HitSensorType::WooGanSandBody},
    {"HoldObj", al::HitSensorType::HoldObj},
    {"MultiPlayer", al::HitSensorType::MultiPlayer},
    {"BindableGigaBell", al::HitSensorType::BindableGigaBell},
    {"BindableGoal", al::HitSensorType::BindableGoal},
    {"BindableAllPlayer", al::HitSensorType::BindableAllPlayer},
    {"BindableBubbleOutScreen", al::HitSensorType::BindableBubbleOutScreen},
    {"BindableKoura", al::HitSensorType::BindableKoura},
    {"BindableNpc", al::HitSensorType::BindableNpc},
    {"BindableRouteDokan", al::HitSensorType::BindableRouteDokan},
    {"BindableBubblePadInput", al::HitSensorType::BindableBubblePadInput},
    {"Bindable", al::HitSensorType::Bindable},
    {"KoopaJr", al::HitSensorType::KoopaJr},
    {"CutsceneStart", al::HitSensorType::CutsceneStart},
    {"BindableGoalItem", al::HitSensorType::BindableGoalItem},
};
}  // namespace

namespace alSensorFunction {
/**
 * Updates the positions of all sensors of an actor.
 * @param pActor The actor.
 */
void updateHitSensorsAll(al::LiveActor* pActor) {
    pActor->getHitSensorKeeper()->update();
}

/**
 * Clears the hit sensors of all sensors of an actor.
 * @param pActor The actor.
 */
void clearHitSensors(al::LiveActor* pActor) {
    pActor->getHitSensorKeeper()->clear();
}

/**
 * Finds a sensor type by its name.
 * @param pName The sensor type name.
 * @return The sensor type, or MapObj if the name is unknown.
 */
al::HitSensorType findSensorTypeByName(const char* pName) {
    for (s32 i = 0; i < static_cast<s32>(sizeof(cSensorTypeTable) / sizeof(cSensorTypeTable[0])); i++) {
        if (al::isEqualString(cSensorTypeTable[i].mName, pName)) {
            return cSensorTypeTable[i].mType;
        }
    }

    return al::HitSensorType::MapObj;
}
}  // namespace alSensorFunction
