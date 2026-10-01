#include "Project/Block/BlockRailRider.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Project/Block/BlockRailLink.hpp"

namespace {
using namespace al;

class DefaultBlockRailRouteSelecter : public BlockRailRouteSelecter {
public:
    bool compareBlockRailRoute(const BlockRailRider* pRider, const BlockRailLink* pLinkA,
                               const BlockRailLink* pLinkB) const override;
};

DefaultBlockRailRouteSelecter sDefaultRouteSelecter;
}  // namespace

namespace al {
/**
 * Constructs a block rail rider that uses the default route selecter.
 */
BlockRailRider::BlockRailRider() : mRouteSelecter(&sDefaultRouteSelecter) {}

/**
 * Moves along the rails, switching links at the ends.
 * @param speed movement distance
 * @param pPos output position
 * @param pDir output direction
 */
void BlockRailRider::move(f32 speed, sead::Vector3f* pPos, sead::Vector3f* pDir) {
    mIsReachEnd = false;

    if (mRailLink == nullptr) {
        return;
    }

    if (!mRailLink->isValidRide()) {
        calcPosAndDir(pPos, pDir);
        *pPos += *pDir * speed;
        mRailLink = nullptr;
        return;
    }

    while (true) {
        mCoord += mIsForward ? speed : -speed;
        BlockRailLink* nextLink;
        bool isFromEnd;

        if (mCoord < 0.0f) {
            speed = speed > 0.0f ? -mCoord : mCoord;
            nextLink = trySelectPrevLink();
            isFromEnd = false;
            mCoord = 0.0f;
        } else if (mCoord > mRailLink->getTotalLength()) {
            if (speed > 0.0f) {
                speed = mCoord - mRailLink->getTotalLength();
            } else {
                speed = mRailLink->getTotalLength() - mCoord;
            }

            nextLink = trySelectNextLink();
            isFromEnd = true;
            mCoord = mRailLink->getTotalLength();
        } else {
            calcPosAndDir(pPos, pDir);
            return;
        }

        if (nextLink == nullptr) {
            calcPosAndDir(pPos, pDir);

            if (mIsLeaveAtEnd) {
                *pPos += speed * *pDir;
                mRailLink = nullptr;
            } else {
                mIsReachEnd = true;
            }

            return;
        }

        if (nextLink->isTerminate()) {
            mIsReachEnd = true;
            calcPosAndDir(pPos, pDir);
            return;
        }

        if (nextLink->isPrevLink(mRailLink)) {
            mRailLink = nextLink;

            if (!isFromEnd) {
                mIsForward = !mIsForward;
            }

            mCoord = 0.0f;
        } else if (nextLink->isNextLink(mRailLink)) {
            mRailLink = nextLink;

            if (isFromEnd) {
                mIsForward = !mIsForward;
            }

            mCoord = nextLink->getTotalLength();
        } else {
            calcPosAndDir(pPos, pDir);
            mRailLink = nullptr;
            return;
        }
    }
}

/**
 * Calculates the position and movement direction.
 * @param pPos output position
 * @param pDir output direction
 * @return whether the rider is on a rail
 */
bool BlockRailRider::calcPosAndDir(sead::Vector3f* pPos, sead::Vector3f* pDir) const {
    if (mRailLink == nullptr) {
        return false;
    }

    f32 length = mRailLink->getTotalLength();
    f32 rate = isNearZero(length, 0.001f) ? 0.0f : mCoord / length;
    mRailLink->calcPos(pPos, rate);
    mRailLink->calcDir(pDir, rate);

    if (!mIsForward) {
        pDir->negate();
    }

    return true;
}

/**
 * Selects the link to continue on from the start of the current link.
 * @return selected link, or null
 */
BlockRailLink* BlockRailRider::trySelectPrevLink() const {
    BlockRailLink* selectedLink = nullptr;
    s32 linkNum = mRailLink->getPrevLinkNum();

    for (s32 i = 0; i < linkNum; i++) {
        selectedLink = selectRoute(selectedLink, mRailLink->getPrevLink(i));
    }

    return selectedLink;
}

/**
 * Selects the link to continue on from the end of the current link.
 * @return selected link, or null
 */
BlockRailLink* BlockRailRider::trySelectNextLink() const {
    BlockRailLink* selectedLink = nullptr;
    s32 linkNum = mRailLink->getNextLinkNum();

    for (s32 i = 0; i < linkNum; i++) {
        selectedLink = selectRoute(selectedLink, mRailLink->getNextLink(i));
    }

    return selectedLink;
}

/**
 * Chooses between the current choice and a candidate link.
 * @param pCurrentLink current choice
 * @param pCandidateLink candidate link
 * @return chosen link
 */
BlockRailLink* BlockRailRider::selectRoute(BlockRailLink* pCurrentLink,
                                           BlockRailLink* pCandidateLink) const {
    if (!pCandidateLink->isValidRide()) {
        return pCurrentLink;
    }

    if (pCurrentLink == nullptr) {
        return pCandidateLink;
    }

    if (mRouteSelecter == nullptr) {
        return pCurrentLink;
    }

    if (mRouteSelecter->compareBlockRailRoute(this, pCandidateLink, pCurrentLink)) {
        return pCandidateLink;
    }

    return pCurrentLink;
}

/**
 * Calculates the movement direction.
 * @param pDir output direction
 * @return whether the rider is on a rail
 */
bool BlockRailRider::calcDir(sead::Vector3f* pDir) const {
    if (mRailLink == nullptr) {
        return false;
    }

    f32 length = mRailLink->getTotalLength();
    f32 rate = isNearZero(length, 0.001f) ? 0.0f : mCoord / length;
    mRailLink->calcDir(pDir, rate);

    if (!mIsForward) {
        pDir->negate();
    }

    return true;
}

/**
 * Puts the rider on a link.
 * @param pLink link
 * @param coord distance from the start of the link
 */
void BlockRailRider::setRailPart(BlockRailLink* pLink, f32 coord) {
    mRailLink = pLink;
    mCoord = coord;
    mIsForward = true;
}

/**
 * Sets the route selecter.
 * @param pSelecter route selecter
 */
void BlockRailRider::setRouteSelecter(BlockRailRouteSelecter* pSelecter) {
    mRouteSelecter = pSelecter;
}

/**
 * Reverses the movement direction.
 */
void BlockRailRider::reverse() {
    mIsForward = !mIsForward;
}

/**
 * Checks if the rider is on a rail.
 * @return whether the rider is on a rail
 */
bool BlockRailRider::isRide() const {
    return mRailLink != nullptr;
}
}  // namespace al

namespace {
/**
 * Checks if the first link continues straighter than the second.
 * @param pRider rider
 * @param pLinkA first link
 * @param pLinkB second link
 * @return whether the first link is preferred
 */
bool DefaultBlockRailRouteSelecter::compareBlockRailRoute(const BlockRailRider* pRider,
                                                          const BlockRailLink* pLinkA,
                                                          const BlockRailLink* pLinkB) const {
    const BlockRailLink* currentLink = pRider->getRailLink();
    sead::Vector3f dirA = sead::Vector3f::ez;

    if (pLinkA->isPrevLink(currentLink)) {
        pLinkA->calcDir(&dirA, 1.0f);
    } else if (pLinkA->isNextLink(currentLink)) {
        pLinkA->calcDir(&dirA, 0.0f);
        dirA.negate();
    }

    sead::Vector3f dirB = sead::Vector3f::ez;

    if (pLinkB->isPrevLink(currentLink)) {
        pLinkB->calcDir(&dirB, 1.0f);
    } else if (pLinkB->isNextLink(currentLink)) {
        pLinkB->calcDir(&dirB, 0.0f);
        dirB.negate();
    }

    sead::Vector3f riderDir = sead::Vector3f::ez;
    pRider->calcDir(&riderDir);
    return riderDir.dot(dirA) > riderDir.dot(dirB);
}
}  // namespace
