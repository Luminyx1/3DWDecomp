#include <prim/seadStringUtil.h>

namespace sead::StringUtil
{
char16 replace(char16 c, const Buffer<const Char16Pair>& rSortedTable)
{
    if (rSortedTable.size() == 0)
    {
        return c;
    }

    const s32 idx =
        rSortedTable.binarySearch(Char16Pair{c, 0}, [](const Char16Pair* p1, const Char16Pair* p2) {
            return p1->before - p2->before;
        });
    if (idx < 0)
    {
        return c;
    }

    return rSortedTable[idx].after;
}
}  // namespace sead::StringUtil
