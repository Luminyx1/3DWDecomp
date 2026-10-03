#pragma once
#include "System/GameDataHolderWriter.hpp"
namespace PlayingStageDataFunction {
void setStagePlayerNumForAssistBlock(GameDataHolderWriter writer, int count);
int getStagePlayerNumForAssistBlock(GameDataHolderAccessor accessor);
} // namespace PlayingStageDataFunction
