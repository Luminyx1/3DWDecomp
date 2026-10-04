#pragma once
#include <basis/seadTypes.h>
class GameDataHolder;

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
            if (mpItems[i] == 6) {
                mpItems[i] = 4;
            }
        }
    }

    GameDataHolder* mpHolder;
    s32* mpItems;
};
static_assert(sizeof(StockItemList) == 0x10);
