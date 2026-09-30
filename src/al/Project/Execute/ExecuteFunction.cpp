#include "Project/Execute/ExecuteFunction.hpp"

#include "Library/Execute/ExecuteOrder.hpp"
#include "Project/Base/StringUtil.hpp"

namespace alExecutorFunction {
/**
 * Counts the execute orders belonging to a list.
 * @param pOrders The execute orders.
 * @param orderNum Number of execute orders.
 * @param pListName List name.
 * @return The number of matching orders.
 */
s32 calcExecutorListNumMax(const al::ExecuteOrder* pOrders, s32 orderNum, const char* pListName) {
    s32 count = 0;
    for (s32 i = 0; i < orderNum; i++) {
        if (isListName(pOrders[i], pListName)) {
            count++;
        }
    }

    return count;
}

/**
 * Checks whether an execute order belongs to a list.
 * @param rOrder The execute order.
 * @param pListName List name.
 * @return True if it belongs to the list.
 */
bool isListName(const al::ExecuteOrder& rOrder, const char* pListName) {
    return al::isEqualString(rOrder.mExecuteGroup, pListName);
}
}  // namespace alExecutorFunction
