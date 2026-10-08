#pragma once

#include <basis/seadTypes.h>

/**
 * @brief Parameters of the stage the sequence starts next.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class ProductStageStartParam {
public:
    void setWorldId(s32 worldId);
    void setStageId(s32 stageId);
};
