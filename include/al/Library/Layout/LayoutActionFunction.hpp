#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace al {
class IUseAudioKeeper;
class IUseLayoutAction;
class LayoutActor;
class MessageTagDataHolder;
class Nerve;
class ReplaceTagProcessorBase;

void startAction(IUseLayoutAction* pLayout, const char* pActionName, const char* pPaneName = nullptr);
bool isPausedAction(IUseLayoutAction* pLayout, const char* pActionName, const char* pPaneName);
void pauseAction(IUseLayoutAction* pLayout, const char* pPaneName);
void unpauseAction(IUseLayoutAction* pLayout, const char* pPaneName);
s32 startActionAtRandomFrame(IUseLayoutAction* pLayout, const char* pActionName,
                             const char* pPaneName = nullptr);
void startFreezeAction(IUseLayoutAction* pLayout, const char* pActionName, f32 frame,
                       const char* pPaneName = nullptr);
void startFreezeActionEnd(IUseLayoutAction* pLayout, const char* pActionName,
                          const char* pPaneName = nullptr);
f32 getActionFrameMax(const IUseLayoutAction* pLayout, const char* pActionName,
                      const char* pPaneName);
void startFreezeGaugeAction(IUseLayoutAction* pLayout, f32 value, f32 minFrame, f32 maxFrame,
                            const char* pActionName, const char* pPaneName = nullptr);
bool tryStartAction(IUseLayoutAction* pLayout, const char* pActionName,
                    const char* pPaneName = nullptr);
bool isExistAction(const IUseLayoutAction* pLayout, const char* pActionName,
                   const char* pPaneName);
bool isActionEnd(const IUseLayoutAction* pLayout, const char* pPaneName = nullptr);
bool isExistAction(const IUseLayoutAction* pLayout, const char* pPaneName);
bool isActionOneTime(const IUseLayoutAction* pLayout, const char* pActionName,
                     const char* pPaneName = nullptr);
f32 getActionFrame(const IUseLayoutAction* pLayout, const char* pPaneName = nullptr);
void setActionFrame(IUseLayoutAction* pLayout, f32 frame, const char* pPaneName = nullptr);
f32 getActionFrameMax(const IUseLayoutAction* pLayout, const char* pPaneName);
f32 getActionFrameRate(const IUseLayoutAction* pLayout, const char* pPaneName = nullptr);
void setActionFrameRate(IUseLayoutAction* pLayout, f32 frameRate, const char* pPaneName = nullptr);
const char* getActionName(const IUseLayoutAction* pLayout, const char* pPaneName = nullptr);
bool isActionPlaying(const IUseLayoutAction* pLayout, const char* pActionName,
                     const char* pPaneName = nullptr);
bool isAnyActionPlaying(const IUseLayoutAction* pLayout, const char* pPaneName = nullptr);
void setNerveAtActionEnd(LayoutActor* pActor, const Nerve* pNerve);
void startTextPaneAnim(LayoutActor* pActor, const char16_t* pMessage,
                       const MessageTagDataHolder* pTagDataHolder = nullptr,
                       const ReplaceTagProcessorBase* pReplaceTagProcessor = nullptr);
void startTextPaneAnimWithAudioUser(LayoutActor* pActor, const char16_t* pMessage,
                                    const MessageTagDataHolder* pTagDataHolder,
                                    const ReplaceTagProcessorBase* pReplaceTagProcessor,
                                    const IUseAudioKeeper* pAudioKeeper);
void startAndSetTextPaneAnimStage(LayoutActor* pActor, const char* pFileName, const char* pLabel,
                                  const MessageTagDataHolder* pTagDataHolder = nullptr,
                                  const ReplaceTagProcessorBase* pReplaceTagProcessor = nullptr);
void startAndSetTextPaneAnimSystem(LayoutActor* pActor, const char* pFileName, const char* pLabel,
                                   const MessageTagDataHolder* pTagDataHolder = nullptr,
                                   const ReplaceTagProcessorBase* pReplaceTagProcessor = nullptr);
void endTextPaneAnim(LayoutActor* pActor);
void skipTextPaneAnim(LayoutActor* pActor);
void flushTextPaneAnim(LayoutActor* pActor);
void changeNextPage(LayoutActor* pActor, const MessageTagDataHolder* pTagDataHolder = nullptr,
                    const ReplaceTagProcessorBase* pReplaceTagProcessor = nullptr);
bool tryChangeNextPage(LayoutActor* pActor, const MessageTagDataHolder* pTagDataHolder = nullptr,
                       const ReplaceTagProcessorBase* pReplaceTagProcessor = nullptr);
bool isExistNextPage(const LayoutActor* pActor);
bool isEndTextPaneAnim(const LayoutActor* pActor, bool isCheckNextPage);
const char16_t* getCurrentMessagePaneAnim(const LayoutActor* pActor);
s32 calcCurrentMessageTextNum(const LayoutActor* pActor);
s32 calcShowTextTime(s32 textNum);
bool tryStartTextAnim(LayoutActor* pActor, const char16_t* pMessage);
bool tryStartTextTagVoice(LayoutActor* pActor, const char16_t* pMessage,
                          const IUseAudioKeeper* pAudioKeeper, const char* pName,
                          sead::FixedSafeString<64>* pVoiceName);
void startHitReaction(const LayoutActor* pActor, const char* pName, const char* pPaneName);
}  // namespace al
