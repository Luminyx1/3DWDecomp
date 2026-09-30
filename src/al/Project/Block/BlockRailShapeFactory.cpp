#include "Project/Block/BlockRailShapeFactory.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Block/BlockRailShape.hpp"
#include "Project/Block/BlockRailShapeStraight.hpp"
#include "Project/Block/BlockRailShapeTerminater.hpp"

namespace al {
static const NameToCreator<BlockRailShapeCreatorFunction> sBlockRailShapeFactoryEntries[] = {
    {"BlockRailShapeStraight", createBlockRailShapeFunction<BlockRailShapeStraight>},
    {"BlockRailShapeCurve", createBlockRailShapeFunction<BlockRailShapeCurve>},
    {"BlockRailShapeTerminater", createBlockRailShapeFunction<BlockRailShapeTerminater>},
};

/**
 * Constructs the block rail shape factory.
 */
BlockRailShapeFactory::BlockRailShapeFactory() : Factory("ブロックレール形状ファクトリ") {
    initFactory(sBlockRailShapeFactoryEntries);
}

/**
 * Constructs a block rail shape.
 * @param pName shape name
 */
BlockRailShape::BlockRailShape(const char* pName) : mName(pName) {}

/**
 * Initializes the shape from the placement's rotation and translation.
 * @param rInfo actor init info
 * @param rIter shape parameters
 */
void BlockRailShape::init(const ActorInitInfo& rInfo, const ByamlIter& rIter) {
    sead::Quatf quat = sead::Quatf::unit;
    tryGetQuat(&quat, rInfo);
    sead::Vector3f trans = sead::Vector3f::zero;
    tryGetTrans(&trans, rInfo);
    init(quat, trans, rIter);
}

/**
 * Initializes the shape. Does nothing by default.
 * @param rQuat rotation
 * @param rTrans translation
 * @param rIter shape parameters
 */
void BlockRailShape::init(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
                          const ByamlIter& rIter) {}

/**
 * Checks if a movement crosses the shape.
 * @param pRate crossing rate
 * @param rPrevPos previous position
 * @param rPos current position
 * @return false
 */
bool BlockRailShape::isRide(f32* pRate, const sead::Vector3f& rPrevPos,
                            const sead::Vector3f& rPos) const {
    return false;
}

/**
 * Gets the total length.
 * @return 0
 */
f32 BlockRailShape::getTotalLength() const {
    return 0.0f;
}

/**
 * Calculates the position at a rate.
 * @param pPos output position
 * @param rate rate on the shape
 */
void BlockRailShape::calcPos(sead::Vector3f* pPos, f32 rate) const {
    *pPos = sead::Vector3f::zero;
}

/**
 * Calculates the direction at a rate.
 * @param pDir output direction
 * @param rate rate on the shape
 */
void BlockRailShape::calcDir(sead::Vector3f* pDir, f32 rate) const {
    *pDir = sead::Vector3f::ez;
}

/**
 * Calculates the nearest point on the shape.
 * @param pPos output position
 * @param pRate output rate
 * @param rPos position
 */
void BlockRailShape::calcNearestParam(sead::Vector3f* pPos, f32* pRate,
                                      const sead::Vector3f& rPos) const {
    *pPos = sead::Vector3f::zero;
    *pRate = 0.0f;
}

/**
 * Calculates the offset from the base translation. Does nothing by default.
 * @param rBaseTrans base translation
 */
void BlockRailShape::calcOffset(const sead::Vector3f& rBaseTrans) {}

/**
 * Updates the position from the base translation. Does nothing by default.
 * @param rBaseTrans base translation
 */
void BlockRailShape::updateLinkedTrans(const sead::Vector3f& rBaseTrans) {}

/**
 * Checks if this is a terminater.
 * @return false
 */
bool BlockRailShape::isTerminate() const {
    return false;
}
}  // namespace al
