#include "Project/Block/BlockRailDirector.hpp"

#include "Project/Block/BlockRail.hpp"
#include "Project/Block/BlockRailLink.hpp"
#include "Project/Block/BlockRailRider.hpp"

namespace al {
/**
 * Constructs the block rail director.
 */
BlockRailDirector::BlockRailDirector() {
    mRails = new BlockRail*[0x100];

    for (s32 i = 0; i < mMaxRails; i++) {
        mRails[i] = nullptr;
    }
}

/**
 * Connects all registered rails and creates their end models.
 * @param rInfo actor init info
 */
void BlockRailDirector::initAfterPlacementSceneObj(const ActorInitInfo& rInfo) {
    for (s32 i = 0; i < mRailNum - 1; i++) {
        for (s32 j = i + 1; j < mRailNum; j++) {
            BlockRail::tryConnect(mRails[i], mRails[j]);
        }
    }

    for (s32 i = 0; i < mRailNum; i++) {
        mRails[i]->tryCreateRailEnd(rInfo);
    }
}

/**
 * Registers a rail.
 * @param pRail rail
 */
void BlockRailDirector::registerBlockRail(BlockRail* pRail) {
    if (mRailNum < mMaxRails) {
        pRail->getName();
        mRails[mRailNum] = pRail;
        mRailNum++;
    }
}

/**
 * Puts a rider on the first rail crossed by a movement.
 * @param pRider rider
 * @param rPrevPos previous position
 * @param rPos current position
 * @return whether a rail was ridden
 */
bool BlockRailDirector::tryRideBlockRail(BlockRailRider* pRider, const sead::Vector3f& rPrevPos,
                                         const sead::Vector3f& rPos) {
    for (s32 i = 0; i < mRailNum; i++) {
        f32 rate = 0.0f;

        if (mRails[i]->getRailLink()->isRide(&rate, rPrevPos, rPos)) {
            BlockRailLink* link = mRails[i]->getRailLink();
            pRider->setRailPart(link, rate * link->getTotalLength());
            return true;
        }
    }

    return false;
}
}  // namespace al
