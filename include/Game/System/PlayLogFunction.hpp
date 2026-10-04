#pragma once
#include "System/GameDataHolderWriter.hpp"
#include <basis/seadTypes.h>
namespace PlayLogFunction {
int getPlayLogDataNum(GameDataHolderAccessor accessor);
u32* getPlayLogData(GameDataHolderAccessor accessor);
void setPlayStart(GameDataHolderWriter writer);
void setPlayTime(GameDataHolderWriter writer, int time);
void setGreenStarNum(GameDataHolderWriter writer, int courseId, int num);
void setMiiverseFlag(GameDataHolderWriter writer, bool isEnable);
void setPlayChara(GameDataHolderWriter writer, int characterType);
void setUserNum(GameDataHolderWriter writer, int num);
void setPlayerEntry(GameDataHolderWriter writer);
bool isUseDrcOneUser(GameDataHolderAccessor accessor);
void useAssistPlayer(GameDataHolderWriter writer);
void useCrossKey(GameDataHolderWriter writer);
void useTouchPanel(GameDataHolderWriter writer);
void useCameraRotate(GameDataHolderWriter writer);
void setPostMiiverse(GameDataHolderWriter writer);
void startStage(GameDataHolderWriter writer, int courseId);
void onStageEnd(GameDataHolderWriter writer);
void clearStage(GameDataHolderWriter writer, int courseId, int time, int greenStarNum,
                bool isAcquireIllustItem, bool isUseAssistBlock);
} // namespace PlayLogFunction
