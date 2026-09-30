#include "Project/Block/BlockRailLink.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Block/BlockRailShape.hpp"
#include "Project/Block/BlockRailShapeFactory.hpp"

namespace al {
/**
 * Constructs a block rail link.
 * @param maxLinks maximum number of previous and next links
 */
BlockRailLink::BlockRailLink(s32 maxLinks) : mMaxLinks(maxLinks) {
    mPrevLinks = new BlockRailLink*[maxLinks];
    mNextLinks = new BlockRailLink*[maxLinks];
    for (s32 i = 0; i < mMaxLinks; i++) {
        mPrevLinks[i] = nullptr;
        mNextLinks[i] = nullptr;
    }
}

/**
 * Creates the shape from the placement's rotation and translation.
 * @param rInfo actor init info
 * @param rIter shape parameters
 */
void BlockRailLink::init(const ActorInitInfo& rInfo, const ByamlIter& rIter) {
    sead::Quatf quat = sead::Quatf::unit;
    tryGetQuat(&quat, rInfo);
    sead::Vector3f trans = sead::Vector3f::zero;
    tryGetTrans(&trans, rInfo);
    init(quat, trans, rIter);
}

/**
 * Creates the shape named in the parameters and initializes it.
 * @param rQuat rotation
 * @param rTrans translation
 * @param rIter shape parameters
 */
void BlockRailLink::init(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
                         const ByamlIter& rIter) {
    const char* shapeName = nullptr;
    rIter.tryGetStringByKey(&shapeName, "Shape");
    BlockRailShapeFactory factory;
    BlockRailShapeCreatorFunction creator = nullptr;
    factory.getEntryIndex(&creator, shapeName);
    if (creator) {
        mShape = creator(shapeName);
        mShape->init(rQuat, rTrans, rIter);
    }
}

/**
 * Calculates the shape's offset from the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailLink::calcOffset(const sead::Vector3f& rBaseTrans) {
    if (mShape) {
        mShape->calcOffset(rBaseTrans);
    }
}

/**
 * Updates the shape's position from the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailLink::updateLinkedTrans(const sead::Vector3f& rBaseTrans) {
    if (mShape) {
        mShape->updateLinkedTrans(rBaseTrans);
    }
}

/**
 * Sets the shape.
 * @param pShape shape
 */
void BlockRailLink::setShape(BlockRailShape* pShape) {
    mShape = pShape;
}

/**
 * Adds a link connected to the start.
 * @param pLink link
 */
void BlockRailLink::addPrev(BlockRailLink* pLink) {
    if (mPrevLinkNum < mMaxLinks) {
        mPrevLinks[mPrevLinkNum] = pLink;
        mPrevLinkNum++;
    }
}

/**
 * Adds a link connected to the end.
 * @param pLink link
 */
void BlockRailLink::addNext(BlockRailLink* pLink) {
    if (mNextLinkNum < mMaxLinks) {
        mNextLinks[mNextLinkNum] = pLink;
        mNextLinkNum++;
    }
}

/**
 * Checks if a movement crosses the rail.
 * @param pRate crossing rate
 * @param rPrevPos previous position
 * @param rPos current position
 * @return whether the movement crosses the rail
 */
bool BlockRailLink::isRide(f32* pRate, const sead::Vector3f& rPrevPos,
                           const sead::Vector3f& rPos) const {
    if (mIsValidRide) {
        return mShape->isRide(pRate, rPrevPos, rPos);
    }

    return false;
}

/**
 * Gets the total length.
 * @return total length
 */
f32 BlockRailLink::getTotalLength() const {
    return mShape->getTotalLength();
}

/**
 * Calculates the position and direction at a rate.
 * @param pPos output position
 * @param pDir output direction
 * @param rate rate on the rail
 */
void BlockRailLink::calcPosAndDir(sead::Vector3f* pPos, sead::Vector3f* pDir, f32 rate) const {
    mShape->calcPos(pPos, rate);
    mShape->calcDir(pDir, rate);
}

/**
 * Calculates the position at a rate.
 * @param pPos output position
 * @param rate rate on the rail
 */
void BlockRailLink::calcPos(sead::Vector3f* pPos, f32 rate) const {
    mShape->calcPos(pPos, rate);
}

/**
 * Calculates the direction at a rate.
 * @param pDir output direction
 * @param rate rate on the rail
 */
void BlockRailLink::calcDir(sead::Vector3f* pDir, f32 rate) const {
    mShape->calcDir(pDir, rate);
}

/**
 * Calculates the nearest point on the rail.
 * @param pPos output position
 * @param pRate output rate
 * @param rPos position
 */
void BlockRailLink::calcNearestParam(sead::Vector3f* pPos, f32* pRate,
                                     const sead::Vector3f& rPos) const {
    mShape->calcNearestParam(pPos, pRate, rPos);
}

/**
 * Checks if this is a terminater.
 * @return whether this is a terminater
 */
bool BlockRailLink::isTerminate() const {
    return mShape->isTerminate();
}

/**
 * Gets a link connected to the start.
 * @param index link index
 * @return link
 */
BlockRailLink* BlockRailLink::getPrevLink(s32 index) const {
    return mPrevLinks[index];
}

/**
 * Gets a link connected to the end.
 * @param index link index
 * @return link
 */
BlockRailLink* BlockRailLink::getNextLink(s32 index) const {
    return mNextLinks[index];
}

/**
 * Checks if a link is connected to the start.
 * @param pLink link
 * @return whether the link is connected to the start
 */
bool BlockRailLink::isPrevLink(const BlockRailLink* pLink) const {
    if (!pLink) {
        return false;
    }

    for (s32 i = 0; i < mPrevLinkNum; i++) {
        if (mPrevLinks[i] == pLink) {
            return true;
        }
    }

    return false;
}

/**
 * Checks if a link is connected to the end.
 * @param pLink link
 * @return whether the link is connected to the end
 */
bool BlockRailLink::isNextLink(const BlockRailLink* pLink) const {
    if (!pLink) {
        return false;
    }

    for (s32 i = 0; i < mNextLinkNum; i++) {
        if (mNextLinks[i] == pLink) {
            return true;
        }
    }

    return false;
}

/**
 * Connects two links whose ends are close to each other.
 * @param pLinkA first link
 * @param pLinkB second link
 * @param distance maximum distance between connected ends
 */
void BlockRailLink::tryConnect(BlockRailLink* pLinkA, BlockRailLink* pLinkB, f32 distance) {
    sead::Vector3f startA;
    pLinkA->mShape->calcPos(&startA, 0.0f);
    sead::Vector3f endA;
    pLinkA->mShape->calcPos(&endA, 1.0f);
    sead::Vector3f startB;
    pLinkB->mShape->calcPos(&startB, 0.0f);
    sead::Vector3f endB;
    pLinkB->mShape->calcPos(&endB, 1.0f);

    if ((startA - startB).length() < distance) {
        pLinkA->addPrev(pLinkB);
        pLinkB->addPrev(pLinkA);
    }

    if (!pLinkB->isTerminate() && (startA - endB).length() < distance) {
        pLinkA->addPrev(pLinkB);
        pLinkB->addNext(pLinkA);
    }

    if (pLinkA->isTerminate()) {
        return;
    }

    if ((endA - startB).length() < distance) {
        pLinkA->addNext(pLinkB);
        pLinkB->addPrev(pLinkA);
    }

    if (!pLinkB->isTerminate() && (endA - endB).length() < distance) {
        pLinkA->addNext(pLinkB);
        pLinkB->addNext(pLinkA);
    }
}
}  // namespace al
