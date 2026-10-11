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


#include "math/seadVector.h"
namespace al {
class IUseSceneObjHolder;
struct SceneInitInfo;
class PlayerHolder;
} // namespace al
class GameDataHolder;
class GhostPlayerRecorder {
public:
    GhostPlayerRecorder(const al::IUseSceneObjHolder*, const al::SceneInitInfo&,
                        const GameDataHolder*, al::PlayerHolder*);
    virtual bool tryStartRecord();
    virtual int endRecord();
    virtual bool saveRecord();
    virtual void recordPosRotData(int, const sead::Vector3f&, const sead::Vector3<short>&);
    virtual bool tryCancelRecording();
    virtual int getHeaderVersion() const;
    void update();
    void setClearTime(int);
    /** @brief Reports whether recording has stopped. @return Whether the recorder is idle. */
    bool isEndRecord() const { return !mRecording; }

private:
    unsigned char mStorage8[0x38];
    bool mRecording;
    unsigned char mStorage41[0x6f];
};
static_assert(sizeof(GhostPlayerRecorder) == 0xb0);
