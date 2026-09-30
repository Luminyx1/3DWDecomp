#pragma once

#include "Library/Factory/Factory.hpp"

namespace al {
class BlockRailShape;

using BlockRailShapeCreatorFunction = BlockRailShape* (*)(const char* pName);

/**
 * Creates a block rail shape.
 * @param pName shape name
 * @return created shape
 */
template <typename T>
BlockRailShape* createBlockRailShapeFunction(const char* pName) {
    return new T(pName);
}

class BlockRailShapeFactory : public Factory<BlockRailShapeCreatorFunction> {
public:
    BlockRailShapeFactory();
};
}  // namespace al
