#pragma once

#include "System/CourseGreenStarInfo.hpp"

class CourseInfo {
  public:
    CourseInfo();
    void reset();
    void resetClearFlag();
    void openCasinoRoom();
    void openGoldenExpress();
    void resetKinopioHouse();
    bool isClear() const;
    bool setOpen();
    bool isOpen() const;
    bool setClear();
    bool setWorldWarpClear();
    bool setGreenStarLock();
    bool isGreenStarLock() const;
    bool isClose() const;
    bool isWorldWarpClear() const;
    void setClearCharacter(s32 characterType);
    bool isClearCharacter(s32 characterType) const;
    bool isClearAllCharacter() const;
    void setAcquireIllustItem(bool acquired);
    bool isAcquireIllustItem() const;
    s32 calcGreenStarAcquireNum(s32 starNum) const;
    bool isGreenStarAcquire(s32 starIndex) const;
    void addMissCount();
    bool isClearWithAssistBlock() const;
    void setClearWithAssistBlock();
    void resetClearWithAssistBlock();
    void setClearInfo(s32 clearType, f32 clearValue);
    void setGreenStarAcquireFlag(const CourseGreenStarInfo& rStars);
    bool setBestScore(s32 score);
    bool setBestTime(s32 time);

    /**
     * @brief Read the best recorded score.
     * @return The best score for the course.
     */
    s32 getBestScore() const { return mBestScore; }

    /**
     * @brief Read the best recorded clear time.
     * @return The best clear time for the course.
     */
    s32 getBestTime() const { return mBestTime; }

    /**
     * @brief Read the character that recorded the course's top clear.
     * @return The clear type stored by setClearInfo.
     */
    s32 getClearType() const { return mClearType; }

    /**
     * @brief Read the highest goal-pole position reached when clearing the course.
     * @return The clear value stored by setClearInfo.
     */
    f32 getClearValue() const { return mClearValue; }

    /**
     * @brief Read how many times the player missed in the course.
     * @return The miss count.
     */
    s32 getMissCount() const { return mMissCount; }

    /**
     * @brief Access the saved Green Star flags.
     * @return The course's Green Star record.
     */
    const CourseGreenStarInfo& getGreenStarInfo() const { return mGreenStars; }

  private:
    enum Flag : u32 {
        Open = 1,
        Clear = 2,
        GreenStarLock = 4,
        AllCharacters = 0xf8,
        WorldWarpClear = 0x4000,
        AssistBlock = 0x8000,
        IllustItem = 0x10000,
    };

    /**
     * @brief Tests whether any flags in a mask are set.
     * @param mask Flags to examine; zero always returns false.
     * @return True when at least one masked flag is set.
     */
    bool hasAnyFlag(u32 mask) const { return (mFlags & mask) != 0; }

    /**
     * @brief Tests whether all flags in a mask are set.
     * @param mask Required flags; zero always returns true.
     * @return True when every masked flag is set.
     */
    bool hasAllFlags(u32 mask) const { return (mFlags & mask) == mask; }

    u32 mFlags;
    s32 mClearType;
    f32 mClearValue;
    s32 mBestScore;
    s32 mBestTime;
    CourseGreenStarInfo mGreenStars;
    s32 mMissCount;
};

static_assert(sizeof(CourseInfo) == 0x1c);
