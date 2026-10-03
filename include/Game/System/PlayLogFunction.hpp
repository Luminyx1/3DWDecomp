#pragma once
#include "System/GameDataHolderWriter.hpp"
#include <basis/seadTypes.h>
namespace PlayLogFunction {
int getPlayLogDataNum(GameDataHolderAccessor accessor);
u32* getPlayLogData(GameDataHolderAccessor accessor);
void setPlayStart(GameDataHolderWriter writer);
void setPlayTime(GameDataHolderWriter writer, int time);
} // namespace PlayLogFunction
