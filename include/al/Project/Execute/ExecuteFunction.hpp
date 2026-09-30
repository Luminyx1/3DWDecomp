#pragma once

#include <basis/seadTypes.h>

namespace al {
struct ExecuteOrder;
}

namespace alExecutorFunction {
s32 calcExecutorListNumMax(const al::ExecuteOrder* pOrders, s32 orderNum, const char* pListName);
bool isListName(const al::ExecuteOrder& rOrder, const char* pListName);
}  // namespace alExecutorFunction
