#pragma once

namespace StageType {
int calcStageTypeID(const char* pName);
bool isNormal(const char* pName);
bool isDrcOnly(const char* pName);
bool isGoldenExpress(const char* pName);
bool isKinopioBrigade(const char* pName);
bool isContinuousMysteryBox(const char* pName);
bool isKinopioHouse(const char* pName);
bool isGateKeeperGoalPole(const char* pName);
bool isGateKeeperNoGoalPole(const char* pName);
bool isKoopaCastleNormal(const char* pName);
bool isKoopaCastleTank(const char* pName);
bool isKoopaCastleExpress(const char* pName);
bool isKoopaCastleExpressNormal(const char* pName);
bool isKoopaCastleFortress(const char* pName);
bool isCasinoRoom(const char* pName);
bool isFairyHouse(const char* pName);
bool isKinopioHouseHide(const char* pName);
bool isDokanHide(const char* pName);
bool isChampionShip(const char* pName);
} // namespace StageType
