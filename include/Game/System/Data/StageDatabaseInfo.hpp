#pragma once
#include <basis/seadTypes.h>
class StageDatabaseInfo {
  public:
    StageDatabaseInfo();
    void initialize(int worldId, int stageId, int courseId, int attribute0c, int attribute10, int attribute1c,
                    int attribute14, int attribute20, int attribute24, int attribute28, const char* pTypeName,
                    const char* pStageName);
    bool isEvent() const;
    bool isKinopioHouse() const;
    bool isCasinoRoom() const;
    bool isFairyHouse() const;
    bool isKinopioHouseHide() const;
    bool isDokanHide() const;
    bool isGoldenExpress() const;
    bool isNormal() const;
    bool isKinopioBrigade() const;
    bool isContinuousMysteryBox() const;
    bool isKoopaCastleNormal() const;
    bool isKoopaCastleTank() const;
    bool isKoopaCastleExpress() const;
    bool isKoopaCastleExpressNormal() const;
    bool isKoopaCastleFortress() const;
    bool isGateKeeperNoGoalPole() const;
    bool isGateKeeperGoalPole() const;
    bool isChampionShip() const;
    bool isUseGoalPole() const;
    bool isNeedDrc() const;
    bool isKoopaCastle() const;
    bool isGateKeeper() const;
    bool isUseCourseInfo() const;
    /**
     * @brief Read the world this stage belongs to.
     * @return The world identifier.
     */
    int getWorldId() const { return mAttributes00[0]; }

    /**
     * @brief Read the stage number inside its world.
     * @return The stage identifier.
     */
    int getStageId() const { return mAttributes00[1]; }

    /**
     * @brief Read the unique course identifier.
     * @return The course identifier.
     */
    int getCourseId() const { return mAttributes00[2]; }

    /**
     * @brief Read the stage resource name.
     * @return The stage resource name.
     */
    const char* getStageName() const { return mStageName; }

    /**
     * @brief Read the textual stage type.
     * @return The stage-type name.
     */
    const char* getTypeName() const { return mTypeName; }

    /**
     * @brief Read the parsed stage type.
     * @return The stage-type identifier.
     */
    int getTypeId() const { return mStageType; }

  private:
    s32 mAttributes00[6]{};
    s32 mStageType = -1;
    s32 mAttribute1c = 0;
    s32 mAttribute20 = -1;
    s32 mAttribute24 = 0;
    s32 mAttribute28 = 0;
    const char* mTypeName = nullptr;
    const char* mStageName = nullptr;
};
static_assert(sizeof(StageDatabaseInfo) == 0x40);
