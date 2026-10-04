#pragma once

namespace al {
class HitSensor;
}

/// Decides which of two objects gets to bind the player.
class BindPriority {
public:
    BindPriority();

    bool isGreater(const al::HitSensor* pSensor, const al::HitSensor* pOther) const;

private:
    const void* mTable;  // 0x0
};

namespace rc {
int getSensorPriority(const al::HitSensor* pSensor);
}  // namespace rc
