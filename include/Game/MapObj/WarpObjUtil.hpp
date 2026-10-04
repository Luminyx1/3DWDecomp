#pragma once

namespace al {
class LiveActor;
}  // namespace al

namespace WarpObjUtil {
void stopStageTimer(const al::LiveActor* pActor);
void restartStageTimer(const al::LiveActor* pActor);
}  // namespace WarpObjUtil
