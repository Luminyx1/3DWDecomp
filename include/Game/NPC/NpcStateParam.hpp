#pragma once

#include <basis/seadTypes.h>

/**
 * @brief Movement parameters shared by the NPC states (wander, chase, run away...).
 * @note The fields have not been reconstructed yet.
 */
class NpcStateParam {
public:
    NpcStateParam();
    NpcStateParam(f32 _0, f32 _4, f32 _8, f32 unused0, f32 unused1, f32 unused2, f32 unused3,
                  f32 _c, s32 _10, f32 _14);

private:
    u8 _0[0x18];
};

static_assert(sizeof(NpcStateParam) == 0x18);
