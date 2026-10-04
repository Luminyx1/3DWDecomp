#pragma once

#include "System/GameDataHolderWriter.hpp"

namespace PlayerEntryFunction {
void entryPlayer(GameDataHolderWriter writer, int userId, int characterType);
void retirePlayer(GameDataHolderWriter writer, int userId);
}  // namespace PlayerEntryFunction
