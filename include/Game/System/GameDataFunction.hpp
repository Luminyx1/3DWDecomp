#pragma once

#include "System/GameDataHolderAccessor.hpp"
#include "System/GameDataHolderWriter.hpp"

class StageDatabaseInfo;
namespace GameDataFunction {
bool isSingleMode(GameDataHolderAccessor);
void initTotalPlayTimeSM(GameDataHolderAccessor accessor, int fileId);
int getInvalidCourseId();
int getWorldNum(GameDataHolderAccessor accessor);
int getCourseTotalNum(GameDataHolderAccessor accessor);
StageDatabaseInfo* findStageDatabaseInfo(GameDataHolderAccessor accessor, int courseId);
int calcCourseId(GameDataHolderAccessor accessor, int worldId, int stageId);
void calcWorldAndStageId(GameDataHolderAccessor accessor, int* pWorldId, int* pStageId,
                         int courseId);
bool isInvalidCourseId(int courseId);
int getLastKoopaCourseId(GameDataHolderAccessor accessor);
int findCourseGreenStarNum(GameDataHolderAccessor accessor, int courseId);
int findIllustItemNum(GameDataHolderAccessor accessor, int courseId);
bool isStageNormal(GameDataHolderAccessor accessor, int courseId);
bool isStageKoopaCastle(GameDataHolderAccessor accessor, int courseId);
bool isStageGateKeeper(GameDataHolderAccessor accessor, int courseId);
bool isStageGoldenExpress(GameDataHolderAccessor accessor, int courseId);
bool isStageKinopioBrigade(GameDataHolderAccessor accessor, int courseId);
bool isStageKinopioHouse(GameDataHolderAccessor accessor, int courseId);
bool isStageKinopioHouseHide(GameDataHolderAccessor accessor, int courseId);
bool isStageContinuousMysteryBox(GameDataHolderAccessor accessor, int courseId);
bool isStageUseDrc(GameDataHolderAccessor accessor, int courseId);
void addMissCount(GameDataHolderWriter writer);
int calcRosettaAppearanceCourseId(GameDataHolderAccessor accessor);
int calcOpenWorldArrangeCourseId(GameDataHolderAccessor accessor);
int calcShowWorldJumpMenuInfoCourseId(GameDataHolderAccessor accessor);
int calcShowWorldJumpMenuInfo2ndCourseId(GameDataHolderAccessor accessor);
int calcShowNetworkGuideCourseId(GameDataHolderAccessor accessor);
int calcShowAmiiboGuideCourseId(GameDataHolderAccessor accessor);
int calcShowSnapshotGuideCourseId(GameDataHolderAccessor accessor);
int calcShowTouchGuideCourseId(GameDataHolderAccessor accessor);
int calcShowCharacterChangeExplainCourseId(GameDataHolderAccessor accessor);
int calcOpenNetworkSettingCourseId(GameDataHolderAccessor accessor);
}; // namespace GameDataFunction