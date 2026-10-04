#pragma once
#include "System/GameDataHolderAccessor.hpp"
namespace GameDataFlagFunction {
bool isAlreadyOpenRosetta(GameDataHolderAccessor accessor);
bool isAlreadyOpenAllClearCharacter(GameDataHolderAccessor accessor, int characterType);
bool isShowWorldStartDemo(GameDataHolderAccessor accessor, int worldId);
} // namespace GameDataFlagFunction
