#pragma once

/// Collects the one-frame events (sensor hits and collision touches) the player got.
class PlayerTrigger {
public:
    /// Sensor events; cSensorTriggerNum means none.
    enum ESensorTrigger { cDamage = 0, cTrample = 1, cSensorTriggerNum = 23 };
    /// Collision events; cCollisionTriggerNum means none.
    enum ECollisionTrigger { cCollisionDamage = 4, cCollisionTriggerNum = 6 };

    bool isOn(ESensorTrigger) const;
    bool isOn(ECollisionTrigger) const;
};
