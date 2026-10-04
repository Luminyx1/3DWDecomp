#pragma once

namespace al {
class IUseSceneObjHolder;
class PlacementId;
}  // namespace al

namespace rc {
bool isExistGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser);
void setPlacementIdObjGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser,
                                          const al::PlacementId* pPlacementId, bool isStart);
void tryStartFromObjGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser,
                                        const al::PlacementId* pPlacementId);
}  // namespace rc
