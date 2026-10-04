#pragma once
#include <basis/seadTypes.h>
class GameDataHolder;

/**
 * @brief One stock-item slot. Stored as a union in the target, so slot writes may alias any member.
 */
union StockItemSlot {
    s32 mItemId;
};

class StockItemList {
  public:
    explicit StockItemList(GameDataHolder* pHolder);
    void initialize();
    void copy(const StockItemList* pOther);
    void clearStage();
    void retireStage();
    bool stockItem(int itemId);
    void useStockItem();
    int getStockItem(int index) const;
    void setStockItem(int index, int itemId);

  private:
    /**
     * @brief Replace temporary stock item 6 with its persistent item 4.
     */
    void normalizeStockItems() {
        for (int i = 0; i < 4; ++i) {
            if (mpItems[i].mItemId == 6) {
                mpItems[i].mItemId = 4;
            }
        }
    }

    GameDataHolder* mpHolder;
    StockItemSlot* mpItems;
};
static_assert(sizeof(StockItemList) == 0x10);
