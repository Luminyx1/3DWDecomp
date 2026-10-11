#pragma once

namespace al { class SensorMsg; }

namespace EnemyStateUtil {
bool isMsgBlowDown(const al::SensorMsg* pMessage);
}
