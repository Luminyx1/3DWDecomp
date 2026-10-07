#pragma once

namespace al {
class ActorInitInfo;
class LiveActor;
}  // namespace al

class GameDataHolderAccessor;
class GameDataHolderWriter;

namespace rc {
s32 getCourseSelectObjectID(const al::LiveActor* pActor, const al::ActorInitInfo& rInfo);
bool isDisappearCourseSelectObject(GameDataHolderAccessor accessor, s32 objectId);
void acquirerCourseSelectObject(GameDataHolderWriter writer, s32 objectId);
}  // namespace rc
