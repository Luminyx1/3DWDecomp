#include "System/Data/SingleModeStockItemArray.hpp"

namespace {
/**
 * @brief Maps a storable item type to its inventory slot.
 * @param itemType Item type; only 1 through 5 and 7 are storable.
 * @return Slot index, or -1 for an unsupported item.
 */
inline s64 getStockIndex(s32 itemType) {
    switch (itemType) {
    case 1:
        return 1;
    case 2:
        return 0;
    case 3:
        return 3;
    case 4:
        return 4;
    case 5:
        return 2;
    case 7:
        return 5;
    default:
        return -1;
    }
}

} // namespace

/**
 * @brief Creates empty stock counts for a game-data holder.
 * @param pHolder Owning game-data holder; retained without dereferencing.
 */
SingleModeStockItemArray::SingleModeStockItemArray(GameDataHolder* pHolder) : mHolder(pHolder), mCounts{} {}

/**
 * @brief Clears all six stock counts.
 */
void SingleModeStockItemArray::initialize() {
    for (auto& rCount : mCounts) {
        rCount = 0;
    }
}

/**
 * @brief Copies stock counts while retaining the owning holder.
 * @param pOther Non-null source inventory; self-copy is permitted.
 */
void SingleModeStockItemArray::copy(const SingleModeStockItemArray* pOther) {
    for (s32 i = 0; i < 6; ++i) {
        mCounts[i] = pOther->mCounts[i];
    }
}

/**
 * @brief Adds a storable item unless its slot already holds five.
 * @param itemType Item type; unsupported types return false without changing stock.
 * @return True when the count was changed.
 */
bool SingleModeStockItemArray::stockItem(s32 itemType) {
    const s64 index = getStockIndex(itemType);
    if (index < 0) {
        return false;
    }
    if (mCounts[index] >= 5) {
        return false;
    }
    ++mCounts[index];
    return true;
}

/**
 * @brief Consumes a storable item if one is available.
 * @param itemType Item type; unsupported types return false without changing stock.
 * @return True when the count was changed.
 */
bool SingleModeStockItemArray::useStockItem(s32 itemType) {
    const s64 index = getStockIndex(itemType);
    if (index < 0) {
        return false;
    }
    if (mCounts[index] == 0) {
        return false;
    }
    --mCounts[index];
    return true;
}

/**
 * @brief Gets the stock count for an item type.
 * @param itemType Item type to look up; unsupported types return zero.
 * @return Current stock count.
 */
u32 SingleModeStockItemArray::getStockItemCount(s32 itemType) const {
    const s64 index = getStockIndex(itemType);
    if (index < 0) {
        return 0;
    }
    return mCounts[index];
}

/**
 * @brief Gets a stock count by slot index.
 * @param index Inventory slot from 0 to 5; not bounds-checked.
 * @return Stored count.
 */
u32 SingleModeStockItemArray::getStockItemCountByIndex(s32 index) const { return mCounts[index]; }

/**
 * @brief Directly replaces a stock count.
 * @param index Inventory slot from 0 to 5; not bounds-checked.
 * @param count New count; this setter does not clamp to the normal limit of five.
 */
void SingleModeStockItemArray::setStockItemCountByIndex(s32 index, u32 count) { mCounts[index] = count; }
