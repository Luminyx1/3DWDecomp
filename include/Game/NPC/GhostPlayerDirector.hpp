#pragma once
#include "Library/Scene/ISceneObj.hpp"
#include "Library/Execute/IUseExecutor.hpp"
namespace al {
class IUseSceneObjHolder;
struct SceneInitInfo;
class ExecuteDirector;
class PlayerHolder;
} // namespace al
class GameDataHolder;
class GhostPlayerRecorder;
class GhostPlayerPlayer;
class GhostPlayerDirector : public al::ISceneObj, public al::IUseExecutor {
public:
    GhostPlayerDirector(const al::IUseSceneObjHolder* pUser, const al::SceneInitInfo& rInfo,
                        al::ExecuteDirector* pExecute, al::PlayerHolder* pPlayers,
                        const GameDataHolder* pData);
    void execute() override;
    bool tryStartRecord();
    bool isEndRecord() const;
    bool tryEndRecord(bool useBaseTime);
    bool trySaveRecord();
    void tryStartPlay();
    void tryEndPlay();
    const char* getSceneObjName() const override;

private:
    void* mNetwork;
    const GameDataHolder* mData;
    GhostPlayerRecorder* mRecorder;
    GhostPlayerPlayer* mPlayer;
};
static_assert(sizeof(GhostPlayerDirector) == 0x30);
