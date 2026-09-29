#pragma once

/// Collects the one-frame events (sensor hits and collision touches) the player got.
class PlayerTrigger {
public:
    /// Sensor events; cSensorTriggerNum means none.
    enum ESensorTrigger { cTrample = 1, cSensorTriggerNum = 23 };
    /// Collision events; cCollisionTriggerNum means none.
    enum ECollisionTrigger { cCollisionTriggerNum = 6 };

    bool isOn(ESensorTrigger) const;
    bool isOn(ECollisionTrigger) const;
};
