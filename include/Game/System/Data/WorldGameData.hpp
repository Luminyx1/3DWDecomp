#pragma once

#include <prim/seadLongBitFlag.h>

class WorldGameData {
  public:
    WorldGameData();
    void initialize();
    bool isOnItemFlag(s32 itemIndex) const;
    void setItemFlag(s32 itemIndex);
    void resetItemFlag();
    void resetAllItemFlag();
    bool isShowFirstDemo() const;
    void setShowFirstDemoFlag();

  private:
    /**
     * @brief Clears a prefix of the world's item flags.
     * @param count Number of leading flags to clear, from 0 to 64.
     */
    void clearItemFlags(s32 count) {
        for (s32 i = 0; i < count; ++i) {
            mFlags.resetBit(i);
        }
    }

    sead::LongBitFlag<64> mFlags;
};

static_assert(sizeof(WorldGameData) == 8);
