#pragma once

#include "Library/Actor/ComboCounter.hpp"

namespace al {
class LiveActor;

/**
 * @brief Combo counter that plays the sequential beat sound of its actor on every combo.
 */
class ComboCounterWithSe : public ComboCounter {
public:
    ComboCounterWithSe(LiveActor* pActor);

    void increment() override;

private:
    LiveActor* mActor;
};

static_assert(sizeof(ComboCounterWithSe) == 0x18);

}  // namespace al
