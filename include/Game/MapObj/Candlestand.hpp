#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class CandlestandWatcher;
class Candlestand : public al::LiveActor {
public:
    Candlestand(const char*, CandlestandWatcher* = nullptr);
    bool isLightOff();
private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(Candlestand) == 0x180);
