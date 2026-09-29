#include "Library/Item/ActorItemInfo.hpp"

#include "Project/Base/StringUtil.hpp"

namespace al {
    /**
     * @brief Constructs an item info.
     * @param pItemKind The kind of item.
     * @param pTiming The timing name the item appears at.
     * @param pFactor The factor name the item appears for.
     */
    ActorItemInfo::ActorItemInfo(const char* pItemKind, const char* pTiming, const char* pFactor)
        : mItemKind(pItemKind), mTiming(pTiming), mFactor(pFactor) {}

    /**
     * @brief Checks whether this item's timing equals the given one (both null counts as equal).
     * @param pTiming The timing name to compare.
     * @return Whether the timings are equal.
     */
    bool ActorItemInfo::isEqualTiming(const char* pTiming) const {
        if (pTiming == nullptr || mTiming == nullptr) {
            return pTiming == nullptr && mTiming == nullptr;
        }
        return isEqualString(pTiming, mTiming);
    }

    /**
     * @brief Checks whether this item's factor equals the given one (both null counts as equal).
     * @param pFactor The factor name to compare.
     * @return Whether the factors are equal.
     */
    bool ActorItemInfo::isEqualFactor(const char* pFactor) const {
        if (pFactor == nullptr || mFactor == nullptr) {
            return pFactor == nullptr && mFactor == nullptr;
        }
        return isEqualString(pFactor, mFactor);
    }
}  // namespace al
