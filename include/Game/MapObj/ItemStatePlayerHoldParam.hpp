#pragma once

#include <math/seadVector.h>

/// Hold offsets of an item carried by the player, one per player size and pose.
class ItemStatePlayerHoldParam {
public:
    ItemStatePlayerHoldParam(const sead::Vector3f&, const sead::Vector3f&, const sead::Vector3f&,
                             const sead::Vector3f&, const sead::Vector3f&, const sead::Vector3f&,
                             const sead::Vector3f&, const sead::Vector3f&, const sead::Vector3f&,
                             const sead::Vector3f&, const sead::Vector3f&);

private:
    sead::Vector3f mOffsets[11];
};
