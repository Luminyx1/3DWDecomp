#include "System/PlayingStageDataFunction.hpp"
#include "System/Data/StageDataHolder.hpp"

/**
 * @brief Record the stage player count used by assist blocks.
 * @param writer Accessor to the active game-data holder.
 * @param count Current stage player count, normally 1 through 4.
 */
void PlayingStageDataFunction::setStagePlayerNumForAssistBlock(GameDataHolderWriter writer, int count) {
    writer.getHolder()->getStageDataHolderPtr()->setStagePlayerNumForAssistBlock(count);
}

/**
 * @brief Read the player count used by assist blocks.
 * @param accessor Accessor to the active game-data holder.
 * @return The recorded player count.
 */
int PlayingStageDataFunction::getStagePlayerNumForAssistBlock(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getStageDataHolder()->getStagePlayerNumForAssistBlock();
}
