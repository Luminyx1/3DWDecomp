#pragma once
#include "System/GameDataHolderAccessor.hpp"
namespace GameDataFlagFunction {
bool isAlreadyOpenRosetta(GameDataHolderAccessor accessor);
bool isAlreadyOpenAllClearCharacter(GameDataHolderAccessor accessor, int characterType);
bool isAlreadyOpenAllClearCharacter(GameDataHolderAccessor accessor, int characterType,
                                    int fileId);
bool isAlreadyOpenShowBestTime(GameDataHolderAccessor accessor);
bool isShowWorldStartDemo(GameDataHolderAccessor accessor, int worldId);
bool isShowWorldStartDemo(GameDataHolderAccessor accessor, int worldId, int fileId);
} // namespace GameDataFlagFunction
