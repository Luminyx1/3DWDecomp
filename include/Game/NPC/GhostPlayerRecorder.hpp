#pragma once

#include <basis/seadTypes.h>

class CheckpointFlag;
class GameDataHolder;

namespace al {
class IUseSceneObjHolder;
class PlacementId;
class PlayerHolder;
struct SceneInitInfo;
}  // namespace al

/** @brief Records the player's movement during a stage so it can be replayed as a ghost. */
class GhostPlayerRecorder {
public:
    GhostPlayerRecorder(const al::IUseSceneObjHolder* pHolder, const al::SceneInitInfo& rInfo,
                        const GameDataHolder* pGameDataHolder, al::PlayerHolder* pPlayerHolder);

    virtual bool tryStartRecord();
    virtual s32 endRecord();
    virtual bool saveRecord();

    void update();
    void setClearTime(s32 time);

    /**
     * @brief Check whether a recording is in progress.
     * @return True while recording.
     */
    bool isRecording() const { return mIsRecording; }

private:
    u8 _8[0x38];
    bool mIsRecording;
    u8 _41[0x6f];
};

static_assert(sizeof(GhostPlayerRecorder) == 0xb0);

namespace rc {
void setFlagShakeAfterGhostPlayerRecorder(const CheckpointFlag* flag);
bool isExistGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser);
void setPlacementIdObjGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser,
                                          const al::PlacementId* pPlacementId, bool isStart);
void tryStartFromObjGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser,
                                        const al::PlacementId* pPlacementId);
void setPlacementIdObjGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser, const char* pId,
                                          bool isStart);
void tryStartFromObjGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser, const char* pId);
bool isFlagShakeAfterGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser);
const al::PlacementId* tryGetPlacementIdFlagGhostPlayerRecorder(const al::IUseSceneObjHolder* pUser);
}  // namespace rc
