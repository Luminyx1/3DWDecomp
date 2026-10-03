#pragma once
#include <basis/seadTypes.h>
class GameDataHolder;
class SingleModeStockItemArray {
  public:
    SingleModeStockItemArray(GameDataHolder* pHolder);
    void initialize();
    void copy(const SingleModeStockItemArray* pOther);
    bool stockItem(s32 itemType);
    bool useStockItem(s32 itemType);
    u32 getStockItemCount(s32 itemType) const;
    u32 getStockItemCountByIndex(s32 index) const;
    void setStockItemCountByIndex(s32 index, u32 count);

  private:
    GameDataHolder* mHolder;
    u32 mCounts[6];
};
static_assert(sizeof(SingleModeStockItemArray) == 0x20);
