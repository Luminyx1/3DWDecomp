#pragma once
namespace GameDataConst {
unsigned int getSaveDataSizeGhostMax();
int getSaveDataVersionGhost();
const char* getPlayerCharacterName(int characterType);
int getPlayerCharacterTypeFromName(const char* pName);
int getPadPortListNum();
const int* getPadPortList();
} // namespace GameDataConst
