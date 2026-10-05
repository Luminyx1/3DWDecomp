#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class BlockQuestionChameleon : public al::LiveActor {
public:
    BlockQuestionChameleon();
private:
    unsigned char _144[0x34];
};
static_assert(sizeof(BlockQuestionChameleon) == 0x178);
