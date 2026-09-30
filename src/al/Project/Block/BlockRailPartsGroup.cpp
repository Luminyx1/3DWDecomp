#include "Project/Block/BlockRailPartsGroup.hpp"

#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Block/BlockRailLink.hpp"
#include "Project/Block/BlockRailParts.hpp"

namespace al {
/**
 * Constructs an empty block rail parts group.
 */
BlockRailPartsGroup::BlockRailPartsGroup() = default;

/**
 * Creates the linked parts and connects them.
 * @param rInfo actor init info
 */
void BlockRailPartsGroup::init(const ActorInitInfo& rInfo) {
    mPartsNum = calcLinkChildNum(rInfo, "Parts");

    if (mPartsNum == 0) {
        return;
    }

    mParts = new BlockRailParts*[mPartsNum];

    for (s32 i = 0; i < mPartsNum; i++) {
        const char* name = getLinksActorDisplayName(rInfo, "Parts", i);
        mParts[i] = new BlockRailParts(name);
        initLinksActor(mParts[i], rInfo, "Parts", i);
    }

    for (s32 i = 0; i < mPartsNum; i++) {
        for (s32 j = i + 1; j < mPartsNum; j++) {
            BlockRailParts::tryConnect(mParts[i], mParts[j]);
        }
    }
}

/**
 * Calculates the offsets of all parts from the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailPartsGroup::calcOffset(const sead::Vector3f& rBaseTrans) {
    for (s32 i = 0; i < mPartsNum; i++) {
        mParts[i]->calcOffset(rBaseTrans);
    }
}

/**
 * Moves all parts with the base translation.
 * @param rBaseTrans base translation
 */
void BlockRailPartsGroup::updateLinkedTrans(const sead::Vector3f& rBaseTrans) {
    for (s32 i = 0; i < mPartsNum; i++) {
        mParts[i]->updateLinkedTrans(rBaseTrans);
    }
}

/**
 * Makes all parts appear.
 */
void BlockRailPartsGroup::active() {
    for (s32 i = 0; i < mPartsNum; i++) {
        mParts[i]->appear();
    }
}

/**
 * Makes one part appear.
 * @param index part index
 */
void BlockRailPartsGroup::specialActive(u32 index) {
    mParts[index]->appear();
}

/**
 * Kills all parts.
 */
void BlockRailPartsGroup::deactive() {
    for (s32 i = 0; i < mPartsNum; i++) {
        mParts[i]->makeActorDead();
    }
}

/**
 * Gets a part.
 * @param index part index
 * @return part
 */
BlockRailParts* BlockRailPartsGroup::getParts(s32 index) const {
    return mParts[index];
}

/**
 * Counts the unconnected link ends.
 * @return number of unconnected link ends
 */
s32 BlockRailPartsGroup::calcEmptyLinkCount() const {
    s32 count = 0;

    for (s32 i = 0; i < mPartsNum; i++) {
        BlockRailParts* parts = mParts[i];
        s32 linkNum = parts->getLinkNum();

        for (s32 j = 0; j < linkNum; j++) {
            BlockRailLink* link = parts->getLink(j);

            if (link->isTerminate()) {
                continue;
            }

            if (link->getPrevLinkNum() == 0) {
                count++;
            }

            if (link->getNextLinkNum() == 0) {
                count++;
            }
        }
    }

    return count;
}
}  // namespace al
