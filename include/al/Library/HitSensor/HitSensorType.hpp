#pragma once

#include <basis/seadTypes.h>

namespace al {
    /// The kind of a hit sensor; names come from alSensorFunction::findSensorTypeByName.
    enum class HitSensorType : s32 {
        Eye = 0,
        Player = 1,
        PlayerEye = 2,
        Npc = 3,
        Ride = 4,
        Enemy = 5,
        EnemyBody = 6,
        EnemyAttack = 7,
        KillerMagnum = 8,
        Dossun = 9,
        EnemySimple = 10,
        MapObj = 11,
        MapObjSimple = 12,
        Bindable = 13,
        CollisionParts = 14,
        KickKoura = 15,
        PlayerFireBall = 16,
        WooGanSandBody = 17,
        HoldObj = 18,
        BindableGigaBell = 19,
        BindableGoal = 20,
        BindableAllPlayer = 21,
        BindableBubbleOutScreen = 22,
        BindableKoura = 23,
        BindableRouteDokan = 24,
        BindableBubblePadInput = 25,
        MultiPlayer = 26,
        KoopaJr = 27,
        CutsceneStart = 28,
        NpcAvoid = 29,
        BindableNpc = 30,
        BindableGoalItem = 31
    };
};
