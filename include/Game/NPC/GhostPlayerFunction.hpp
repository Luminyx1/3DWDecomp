#pragma once
namespace al {
class IUseSceneObjHolder;
}
class GhostPlayerPlayer;
namespace GhostPlayerFunction {
void registerGhostPlayerPlayer(GhostPlayerPlayer* pPlayer);
bool isExistGhostPlayerDirector(const al::IUseSceneObjHolder* pUser);
} // namespace GhostPlayerFunction
