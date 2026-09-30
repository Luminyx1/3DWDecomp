#include "Library/Execute/ExecutorListBase.hpp"

namespace al {
/**
 * Constructs an executor list.
 * @param pListName List name.
 * @param pGroupName Group name, or nullptr to use the list name.
 */
ExecutorListBase::ExecutorListBase(const char* pListName, const char* pGroupName) {
    mListName = pListName;
    mGroupName = pGroupName;

    if (pGroupName == nullptr) {
        mGroupName = mListName;
    }
}
}  // namespace al
