#pragma once
#include "Enemy/ChorobonNode.hpp"

/** @brief A Fuzzy that follows its holder's rail. */
class Chorobon : public ChorobonNode {
public:
    Chorobon(const char* pName, const f32* pSpeed);
private:
    u8 mUnknown[24];
};
static_assert(sizeof(Chorobon) == 0x168);
