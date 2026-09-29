#pragma once

#include <basis/seadTypes.h>

namespace al {
class EffectKeeper {
public:
    void update();
    void deleteAndClearEffectAll();

    void onCalcAndDraw();
    void offCalcAndDraw();
};
}  // namespace al
