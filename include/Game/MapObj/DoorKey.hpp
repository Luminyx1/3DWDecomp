#pragma once

#include "Library/LiveActor/LiveActor.hpp"

// Partial interface used by enemy death handling; the key's member layout is not yet reconstructed.
class DoorKey : public al::LiveActor {
public:
    void triggerKillForce(bool isForce);
};
