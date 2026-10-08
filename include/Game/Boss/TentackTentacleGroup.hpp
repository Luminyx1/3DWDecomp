#pragma once
#include <basis/seadTypes.h>
#include <math/seadVector.h>

class TentackHead;
class TentackTentacle;

/** @brief Set of tentacles that attack together under one head. */
class TentackTentacleGroup {
public:
    explicit TentackTentacleGroup(s32 tentacleNumMax);
    void update();
    void reset();
    void registerTentacle(TentackTentacle* pTentacle);
    void appearAll();
    bool requestEndSwingAll();
    void eatAttachItemAll();
    TentackHead* getHead() const;

private:
    unsigned char mUnknown0[0x20];

public:
    sead::Vector3f mTargetPos;  // 0x20 position the owning head turns towards

private:
    unsigned char mUnknown2C[0x4];
};
static_assert(sizeof(TentackTentacleGroup) == 0x30);
