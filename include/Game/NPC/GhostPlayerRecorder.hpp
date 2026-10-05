#pragma once

class CheckpointFlag;

namespace al {
class IUseSceneObjHolder;
class PlacementId;
}  // namespace al

namespace rc {
void setFlagShakeAfterGhostPlayerRecorder(const CheckpointFlag* flag);
bool isExistGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser);
void setPlacementIdObjGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser,
                                          const al::PlacementId* pPlacementId, bool isStart);
void tryStartFromObjGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser,
                                        const al::PlacementId* pPlacementId);
}  // namespace rc
