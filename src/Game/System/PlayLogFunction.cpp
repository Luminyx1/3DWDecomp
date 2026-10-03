#include "System/PlayLogFunction.hpp"
#include "System/PlayLogData.hpp"

/**
 * @brief Access the common play-log storage.
 * @param accessor Accessor to an initialized game-data holder.
 * @return The requested counter count or buffer.
 */
int PlayLogFunction::getPlayLogDataNum(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getPlayLog()->getValueCount();
}

/**
 * @brief Access the common play-log storage.
 * @param accessor Accessor to an initialized game-data holder.
 * @return The requested counter count or buffer.
 */
u32* PlayLogFunction::getPlayLogData(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getPlayLog()->getValues();
}

/**
 * @brief Count another play session.
 * @param writer Accessor to an initialized game-data holder.
 */
void PlayLogFunction::setPlayStart(GameDataHolderWriter writer) {
    ++writer.getHolder()->getPlayLog()->getCommonData()[0];
}

/**
 * @brief Accumulate time in the common play log.
 * @param writer Accessor to an initialized game-data holder.
 * @param time Elapsed play time to add in the caller's time units.
 */
void PlayLogFunction::setPlayTime(GameDataHolderWriter writer, int time) {
    writer.getHolder()->getPlayLog()->getCommonData()[1] += time;
}
