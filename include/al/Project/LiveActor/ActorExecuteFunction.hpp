#pragma once

namespace al {
class DepthShadowDrawer;
class LiveActorKit;

void executeUpdate(LiveActorKit* pKit);
void setLODForceLevel0(LiveActorKit* pKit);
void forceUpdateLOD(LiveActorKit* pKit, bool isForceLevel0);
void executeUpdateList(LiveActorKit* pKit, const char* pListName);
void executeUpdateListPaused(LiveActorKit* pKit, const char* pListName);
void executeUpdateListStall(LiveActorKit* pKit, const char* pListName);
void executeDraw(const LiveActorKit* pKit, const char* pTableName);
void executeDrawList(const LiveActorKit* pKit, const char* pTableName, const char* pListName);
bool isActiveDraw(const LiveActorKit* pKit, const char* pTableName);
DepthShadowDrawer* getDepthShadowDrawer(LiveActorKit* pKit);
void updatePadRumbleDirector(LiveActorKit* pKit);
}  // namespace al
