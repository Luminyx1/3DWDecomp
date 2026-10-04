#include "System/Data/StockItemList.hpp"
#include "System/GameDataHolderAccessor.hpp"

namespace rc {
int getActiveControlUserNum(GameDataHolderAccessor accessor);
}

/**
 * @brief Allocate four empty stock-item slots.
 * @param pHolder Game-data holder supplying the active player count.
 */
StockItemList::StockItemList(GameDataHolder* pHolder) : mpHolder(pHolder) {
    StockItemSlot* pItems = new StockItemSlot[4];
    mpItems = pItems;

    for (int i = 0; i < 4; ++i) {
        pItems[i].mItemId = 0;
    }
}

/**
 * @brief Clear every stock-item slot.
 */
void StockItemList::initialize() {
    for (int i = 0; i < 4; ++i) {
        mpItems[i].mItemId = 0;
    }
}

/**
 * @brief Copy all four stock items.
 * @param pOther Non-null stock-item list to copy.
 */
void StockItemList::copy(const StockItemList* pOther) {
    for (int i = 0; i < 4; ++i) {
        mpItems[i].mItemId = pOther->mpItems[i].mItemId;
    }
}

/**
 * @brief Convert temporary stock items when leaving a stage.
 */
void StockItemList::clearStage() { normalizeStockItems(); }

/**
 * @brief Convert temporary stock items when leaving a stage.
 */
void StockItemList::retireStage() { normalizeStockItems(); }

/**
 * @brief Insert a stock item while respecting the active player count.
 *
 * A full list drops its last basic item (1) to make room; the new item is inserted in front of the
 * first slot not holding a temporary item (6), and slots beyond the active player count are cleared.
 * @param itemId Item identifier; zero is rejected. Requires one to four active players.
 * @return False for an empty item or a basic item when the available slots are full.
 */
bool StockItemList::stockItem(int itemId) {
    if (itemId == 0) {
        return false;
    }

    StockItemSlot* pItems = mpItems;
    const int count = rc::getActiveControlUserNum(GameDataHolderAccessor(mpHolder));

    if (itemId == 1) {
        if (pItems[count - 1].mItemId != 0) {
            return false;
        }
    } else if (pItems[count - 1].mItemId != 0) {
        for (int i = count - 1; i >= 0; --i) {
            if (pItems[i].mItemId == 1) {
                for (int j = i; j < 3; ++j) {
                    pItems[j].mItemId = pItems[j + 1].mItemId;
                }

                pItems[3].mItemId = 0;
                break;
            }
        }
    }

    int index = 4;
    for (u32 i = 0; i < 4; ++i) {
        if (pItems[i].mItemId != 6) {
            index = i;
            break;
        }
    }

    if (index != 4) {
        for (int j = 3; j > index; --j) {
            pItems[j].mItemId = pItems[j - 1].mItemId;
        }

        pItems[index].mItemId = itemId;
    }

    for (int i = count; i < 4; ++i) {
        pItems[i].mItemId = 0;
    }

    return true;
}

/**
 * @brief Consume the first stock item and shift remaining active slots.
 */
void StockItemList::useStockItem() {
    StockItemSlot* pItems = mpItems;
    const int count = rc::getActiveControlUserNum(GameDataHolderAccessor(mpHolder));

    for (int i = 0; i < count - 1; ++i) {
        pItems[i].mItemId = pItems[i + 1].mItemId;
    }

    for (int i = count - 1; i < 4; ++i) {
        pItems[i].mItemId = 0;
    }
}

/**
 * @brief Read a stock-item slot.
 * @param index Slot index from 0 through 3.
 * @return The item identifier, or zero for an empty slot.
 */
int StockItemList::getStockItem(int index) const { return mpItems[index].mItemId; }

/**
 * @brief Replace a stock-item slot.
 * @param index Slot index from 0 through 3.
 * @param itemId Item identifier; zero empties the slot.
 */
void StockItemList::setStockItem(int index, int itemId) { mpItems[index].mItemId = itemId; }
