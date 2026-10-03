#include "Library/Actor/ComboCounterWithSe.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Se/Function/SeFunction.hpp"

namespace al {

/**
 * @brief Creates a combo counter playing its sound on the given actor.
 * @param pActor Actor playing the combo sound.
 */
ComboCounterWithSe::ComboCounterWithSe(LiveActor* pActor) : mActor(pActor) {}

/**
 * @brief Counts up the combo and plays the sequential beat sound from the second hit on.
 */
void ComboCounterWithSe::increment() {
    ComboCounter::increment();

    if (mCounter >= 2) {
        s32 index = sead::Mathi::min(mCounter - 1, 7);
        startSeSetSeqLoacalVariable(mActor, "SeSyBeatEnemySequentially", 0, index);
    }
}

}  // namespace al
