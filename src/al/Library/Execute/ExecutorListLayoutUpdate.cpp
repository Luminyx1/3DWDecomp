#include "Library/Execute/ExecutorListLayout.hpp"

#include "Library/Layout/LayoutActor.hpp"

namespace al {
/**
 * Constructs a layout update executor list.
 * @param pListName List name.
 * @param capacity Maximum number of layouts.
 * @param pGroupName Group name.
 */
ExecutorListLayoutUpdate::ExecutorListLayoutUpdate(const char* pListName, s32 capacity,
                                                   const char* pGroupName)
    : ExecutorListBase(pListName, pGroupName), mLayoutNumMax(capacity) {
    mLayouts = new LayoutActor*[capacity];
    for (s32 i = 0; i < mLayoutNumMax; i++) {
        mLayouts[i] = nullptr;
    }
}

/**
 * Adds a layout.
 * @param pLayout The layout.
 */
void ExecutorListLayoutUpdate::registerLayout(LayoutActor* pLayout) {
    mLayouts[mLayoutNum] = pLayout;
    mLayoutNum++;
}

/**
 * Updates all alive layouts.
 */
void ExecutorListLayoutUpdate::executeList() const {
    for (s32 i = 0; i < mLayoutNum; i++) {
        LayoutActor* layout = mLayouts[i];
        if (layout->isAlive()) {
            layout->movement();
            layout->calcAnim(true);
        }
    }
}
}  // namespace al
