#include "System/GameDataPlayReportCommon.hpp"
#include <attributes.h>
#include <cstring>
#include <stream/seadStream.h>

namespace {

/**
 * @brief Serialized common play-report block; the tail is reserved and written as zero.
 */
struct GameDataPlayReportCommonSaveBlock {
    GameDataPlayReportCommonValues values;
    u8 reserved[0x100];
};
static_assert(sizeof(GameDataPlayReportCommonSaveBlock) == 0x15c);

/**
 * @brief Copy play-report counters field by field.
 * @param pDst Destination counters.
 * @param rSrc Source counters.
 */
ALWAYS_INLINE void copyValues(GameDataPlayReportCommonValues* pDst,
                              const GameDataPlayReportCommonValues& rSrc) {
    pDst->mValue0 = rSrc.mValue0;
    pDst->mValue1 = rSrc.mValue1;
    pDst->mValue2 = rSrc.mValue2;
    pDst->mValue3 = rSrc.mValue3;
    std::memcpy(pDst->mCounts0, rSrc.mCounts0, sizeof(rSrc.mCounts0));
    std::memcpy(pDst->mCounts1, rSrc.mCounts1, sizeof(rSrc.mCounts1));
    std::memcpy(pDst->mCounts2, rSrc.mCounts2, sizeof(rSrc.mCounts2));
    std::memcpy(pDst->mCounts3, rSrc.mCounts3, sizeof(rSrc.mCounts3));
}

} // namespace

/**
 * @brief Clear the common play-report counters.
 * @param pHolder Game-data holder; unused by this constructor.
 */
GameDataPlayReportCommon::GameDataPlayReportCommon(GameDataHolder* pHolder) {
    std::memset(&mValues, 0, sizeof(mValues));
}

/**
 * @brief Reset all common play-report counters.
 */
void GameDataPlayReportCommon::initializeData() { std::memset(&mValues, 0, sizeof(mValues)); }

/**
 * @brief Accept common play-report data without additional validation.
 * @return Always true.
 */
bool GameDataPlayReportCommon::checkValid() { return true; }

/**
 * @brief Read the size-prefixed common play-report block.
 * @param pStream Non-null input stream positioned at the play-report block.
 * @return Always true; stream errors are handled by the stream.
 */
bool GameDataPlayReportCommon::readFromStream(sead::ReadStream* pStream) {
    s32 size;
    pStream->readS32(size);
    GameDataPlayReportCommonSaveBlock block = {};
    pStream->readMemBlock(&block, sizeof(block));
    copyValues(&mValues, block.values);
    return true;
}

/**
 * @brief Write the size-prefixed common play-report block.
 * @param pStream Non-null output stream positioned at the play-report block.
 */
void GameDataPlayReportCommon::writeToStream(sead::WriteStream* pStream) const {
    pStream->writeS32(sizeof(GameDataPlayReportCommonSaveBlock));
    GameDataPlayReportCommonSaveBlock block = {};
    copyValues(&block.values, mValues);
    pStream->writeMemBlock(&block, sizeof(block));
}
