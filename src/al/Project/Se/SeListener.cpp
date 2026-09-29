#include "Project/Se/SeListener.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Se/SeListenerPoser.hpp"

namespace al {
/**
 * @brief Constructs a sound listener able to hold a fixed number of posers.
 * @param poserNumMax The maximum number of posers that can be added.
 */
SeListener::SeListener(s32 poserNumMax) : mCurrentPoserIndex(0) {
    mPosers.allocBuffer(poserNumMax, nullptr);
}

/**
 * @brief Adds a poser the listener can switch to.
 * @param pPoser The poser to add; ignored if the list is full.
 */
void SeListener::addPoser(ISeListenerPoser* pPoser) {
    mPosers.pushBack(pPoser);
}

/**
 * @brief Selects the poser used to compute the listener pose, by index.
 * @param index The index of the poser in the order it was added.
 */
void SeListener::setCurrentPoserIndex(s32 index) {
    mCurrentPoserIndex = index;
}

/**
 * @brief Selects the poser used to compute the listener pose, by name.
 * @param rName The name of the poser; nothing changes if no poser has this name.
 */
void SeListener::setCurrentPoser(const sead::SafeString& rName) {
    for (s32 i = 0; i < mPosers.size(); i++) {
        if (isEqualString(mPosers.unsafeAt(i)->getName(), rName)) {
            mCurrentPoserIndex = i;
            return;
        }
    }
}

/**
 * @brief Updates the listener matrix and position with the current poser.
 * @param rParam The listener parameters providing the camera state.
 */
void SeListener::calcListenerMatrix(const ISeListenerParam& rParam) {
    mPosers.unsafeAt(mCurrentPoserIndex)->calcListenerPose(&mListenerMatrix, &mListenerPos, rParam);
}

/**
 * @brief Gets the poser currently used to compute the listener pose.
 * @return The current poser, or nullptr if the current index is out of range.
 */
ISeListenerPoser* SeListener::getCurrentPoser() const {
    return mPosers[mCurrentPoserIndex];
}
}  // namespace al
