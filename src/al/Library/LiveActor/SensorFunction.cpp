#include "Library/LiveActor/SensorFunction.hpp"
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
};

namespace alSensorFunction {
    /**
     * @brief Updates every hit sensor of an actor.
     * @param pActor The actor whose sensors to update.
     */
    void updateHitSensorsAll(al::LiveActor* pActor) {
        pActor->mHitSensorKeeper->update();
    }

    /**
     * @brief Clears the hits collected by every hit sensor of an actor.
     * @param pActor The actor whose sensors to clear.
     */
    void clearHitSensors(al::LiveActor* pActor) {
        pActor->mHitSensorKeeper->clear();
    }

    /**
     * @brief Looks up a hit sensor type by its name in the actor's sensor resource.
     * @param pName The name of the type, e.g. "Player" or "EnemyBody".
     * @return The matching type, or MapObj if the name is unknown.
     */
    al::HitSensorType findSensorTypeByName(const char* pName) {
        for (s32 i = 0; i < sizeof(cSensorTypeTable) / sizeof(cSensorTypeTable[0]); i++) {
            if (al::isEqualString(cSensorTypeTable[i].mName, pName)) {
                return cSensorTypeTable[i].mType;
            }
        }

        return al::HitSensorType::MapObj;
    }
};
