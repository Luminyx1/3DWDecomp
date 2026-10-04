#pragma once

#include <prim/seadLongBitFlag.h>
#include <stream/seadStream.h>

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

    /**
     * @brief Copies every flag from another world record.
     * @param rOther Source record; copying a record onto itself does nothing.
     */
    void copyFlags(const WorldGameData& rOther) {
        if (this != &rOther) {
            mFlags = rOther.mFlags;
        }
    }

    /**
     * @brief Loads the raw flag words from a save stream.
     * @param pStream Non-null stream positioned at this record.
     */
    void readFlags(sead::ReadStream* pStream) { pStream->readMemBlock(&mFlags, sizeof(mFlags)); }

    /**
     * @brief Stores the raw flag words to a save stream, or skips over them.
     * @param pStream Non-null stream positioned at this record.
     * @param isSkip True to advance past the record without writing it.
     */
    void writeFlags(sead::WriteStream* pStream, bool isSkip) const {
        if (isSkip) {
            pStream->skip(sizeof(mFlags));
        } else {
            pStream->writeMemBlock(&mFlags, sizeof(mFlags));
        }
    }

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
