#pragma once

namespace agl {
class RenderBuffer;
}

namespace al {
class AudioDirector;
class GamePadSystem;
class LayoutActor;
class LayoutInitInfo;
class LayoutKit;
class LayoutSystem;
class MessageSystem;
class SceneObjHolder;

void initLayoutInitInfo(LayoutInitInfo* pInfo, const LayoutKit* pKit,
                        SceneObjHolder* pSceneObjHolder, const AudioDirector* pAudioDirector,
                        const LayoutSystem* pLayoutSystem, const MessageSystem* pMessageSystem,
                        const GamePadSystem* pGamePadSystem);
void setRenderBuffer(LayoutKit* pKit, const agl::RenderBuffer* pRenderBuffer);
void executeUpdate(LayoutKit* pKit);
void executeUpdateList(LayoutKit* pKit, const char* pTableName, const char* pListName);
void executeUpdateEffect(LayoutKit* pKit);
void executeDraw(const LayoutKit* pKit, const char* pTableName);
void executeDrawEffect(const LayoutKit* pKit);
void reinitializeShaders(LayoutActor* pActor);
}  // namespace al
