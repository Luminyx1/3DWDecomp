#pragma once

#include <basis/seadTypes.h>

namespace al {
    class LiveActor;
    class Nerve;
};  // namespace al

class ItemAssistRotateParam;

namespace CoinUtil {
    const ItemAssistRotateParam* getCoinAssistRotateParam();
    bool tryStartSpinCoinIfMicInputOn(al::LiveActor* pActor, const al::Nerve* pNerve);
};  // namespace CoinUtil
