#include "System/Data/StageDatabaseInfo.hpp"

/**
 * @brief Creates empty stage metadata with unresolved type and database index.
 */
StageDatabaseInfo::StageDatabaseInfo() {}

/**
 * @brief Tests the stage classification Event.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isEvent() const {
    switch (mStageType) {
    case 2:
    case 5:
    case 13:
    case 14:
    case 15:
    case 16:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Tests the stage classification KinopioHouse.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isKinopioHouse() const { return mStageType == 5; }

/**
 * @brief Tests the stage classification CasinoRoom.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isCasinoRoom() const { return mStageType == 13; }

/**
 * @brief Tests the stage classification FairyHouse.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isFairyHouse() const { return mStageType == 14; }

/**
 * @brief Tests the stage classification KinopioHouseHide.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isKinopioHouseHide() const { return mStageType == 15; }

/**
 * @brief Tests the stage classification DokanHide.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isDokanHide() const { return mStageType == 16; }

/**
 * @brief Tests the stage classification GoldenExpress.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isGoldenExpress() const { return mStageType == 2; }

/**
 * @brief Tests the stage classification Normal.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isNormal() const {
    switch (mStageType) {
    case 0:
    case 1:
    case 11:
    case 17:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Tests the stage classification KinopioBrigade.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isKinopioBrigade() const { return mStageType == 3; }

/**
 * @brief Tests the stage classification ContinuousMysteryBox.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isContinuousMysteryBox() const { return mStageType == 4; }

/**
 * @brief Tests the stage classification KoopaCastleNormal.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isKoopaCastleNormal() const { return mStageType == 8; }

/**
 * @brief Tests the stage classification KoopaCastleTank.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isKoopaCastleTank() const { return mStageType == 9; }

/**
 * @brief Tests the stage classification KoopaCastleExpress.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isKoopaCastleExpress() const { return mStageType == 10; }

/**
 * @brief Tests the stage classification KoopaCastleExpressNormal.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isKoopaCastleExpressNormal() const { return mStageType == 11; }

/**
 * @brief Tests the stage classification KoopaCastleFortress.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isKoopaCastleFortress() const { return mStageType == 12; }

/**
 * @brief Tests the stage classification GateKeeperNoGoalPole.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isGateKeeperNoGoalPole() const { return mStageType == 7; }

/**
 * @brief Tests the stage classification GateKeeperGoalPole.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isGateKeeperGoalPole() const { return mStageType == 6; }

/**
 * @brief Tests the stage classification ChampionShip.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isChampionShip() const { return mStageType == 17; }

/**
 * @brief Tests the stage classification UseGoalPole.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isUseGoalPole() const {
    switch (mStageType) {
    case 0:
    case 1:
    case 6:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 17:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Tests the stage classification NeedDrc.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isNeedDrc() const { return mStageType == 1 || mStageType == 3; }

/**
 * @brief Tests the stage classification KoopaCastle.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isKoopaCastle() const {
    switch (mStageType) {
    case 8:
    case 9:
    case 10:
    case 12:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Tests the stage classification GateKeeper.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isGateKeeper() const { return mStageType == 6 || mStageType == 7; }

/**
 * @brief Tests the stage classification UseCourseInfo.
 * @return True when the stage belongs to this classification.
 */
bool StageDatabaseInfo::isUseCourseInfo() const {
    return isNormal() || isContinuousMysteryBox() || isKinopioBrigade() || isGateKeeper() || isKoopaCastle();
}

#include "System/Data/StageType.hpp"

/**
 * @brief Populate a stage database record and resolve its textual type.
 * @param worldId One-based world identifier.
 * @param stageId Stage identifier within the world.
 * @param courseId Unique course identifier.
 * @param attribute0c Database attribute stored at offset 0x0c; meaning is not yet established.
 * @param attribute10 Database attribute stored at offset 0x10; meaning is not yet established.
 * @param attribute1c Database attribute stored at offset 0x1c; meaning is not yet established.
 * @param attribute14 Database attribute stored at offset 0x14; meaning is not yet established.
 * @param attribute20 Database attribute stored at offset 0x20; meaning is not yet established.
 * @param attribute24 Database attribute stored at offset 0x24; meaning is not yet established.
 * @param attribute28 Database attribute stored at offset 0x28; meaning is not yet established.
 * @param pTypeName Non-null stage-type name; the string must outlive this record.
 * @param pStageName Stage resource name; the string must outlive this record.
 */
void StageDatabaseInfo::initialize(int worldId, int stageId, int courseId, int attribute0c, int attribute10,
                                   int attribute1c, int attribute14, int attribute20, int attribute24,
                                   int attribute28, const char* pTypeName, const char* pStageName) {
    mAttributes00[0] = worldId;
    mAttributes00[1] = stageId;
    mAttributes00[2] = courseId;
    mAttributes00[3] = attribute0c;
    mAttributes00[4] = attribute10;
    mAttributes00[5] = attribute14;
    mAttribute1c = attribute1c;
    mAttribute20 = attribute20;
    mAttribute24 = attribute24;
    mStageType = StageType::calcStageTypeID(pTypeName);
    mAttribute28 = attribute28;
    mTypeName = pTypeName;
    mStageName = pStageName;
}
