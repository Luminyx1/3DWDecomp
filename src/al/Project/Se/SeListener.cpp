#include "Project/Se/SeListener.hpp"

#include "Project/Base/StringUtil.hpp"

namespace al {

SeListener::SeListener(s32 poserNum) {
    mPosers.allocBuffer(poserNum, nullptr);
}

void SeListener::addPoser(ISeListenerPoser* pPoser) {
    mPosers.pushBack(pPoser);
}

void SeListener::setCurrentPoserIndex(s32 index) {
    mCurrentPoserIndex = index;
}

void SeListener::setCurrentPoser(const sead::SafeString& rName) {
    for (s32 i = 0; i < mPosers.size(); i++) {
        if (isEqualString(mPosers.unsafeAt(i)->getName(), rName)) {
            mCurrentPoserIndex = i;
            return;
        }
    }
}

void SeListener::calcListenerMatrix(const ISeListenerParam& rParam) {
    mPosers.unsafeAt(mCurrentPoserIndex)->calcListenerPose(&mListenerMatrix, &mListenerPos, rParam);
}

ISeListenerPoser* SeListener::getCurrentPoser() const {
    return mPosers.at(mCurrentPoserIndex);
}

}  // namespace al
