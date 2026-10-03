#pragma once
class GameDataFile;
namespace SelectedGameDataFlagFunction {
bool isAlreadyOpenRosetta(const GameDataFile* pFile);
bool isAlreadyShowSpecialWorldOpenDemo(const GameDataFile* pFile);
bool isAlreadyOpenAllClearCharacter(const GameDataFile* pFile, int characterType);
} // namespace SelectedGameDataFlagFunction
