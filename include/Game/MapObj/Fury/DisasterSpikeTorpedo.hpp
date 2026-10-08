#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.hpp"

class DisasterSpikeDirector;
class SuperBowser;

/// Spike Fury Bowser shoots through the ocean at Plessie.
class DisasterSpikeTorpedo : public al::LiveActor {
public:
    explicit DisasterSpikeTorpedo(const char* pName);

    void setDisasterSpikeDirector(DisasterSpikeDirector* pDirector);
    void appear(SuperBowser* pBowser, s32 index, bool isFirst);
    void appearReckless(SuperBowser* pBowser, sead::Vector3f pos);

    /**
     * @brief Get the id given to the torpedo when it was shot.
     * @return The torpedo id.
     */
    s32 getID() const { return mID; }

private:
    u8 _144[0x160 - 0x144];
    s32 mID;  // 0x160
    u8 _164[0x1d8 - 0x164];
};

static_assert(sizeof(DisasterSpikeTorpedo) == 0x1d8);
